/*
 * AutomatedPickerRobotNavigator.cpp
 *
 *  Created on: Nov 6, 2012
 *      Author: coky
 */

//#define USE_SPHERE_SEGMENTATION

#include <mantis_object_manipulation/arm_navigators/AutomatedPickerRobotNavigator.h>
#include <mantis_object_manipulation/arm_navigators/AutomatedPickerRobotNavigator.h>
#include <mantis_perception/mantis_recognition.h>
#include <boost/foreach.hpp>
#include <algorithm>

std::string AutomatedPickerRobotNavigator::MARKER_SEGMENTED_OBJECT = "segmented_obj";
std::string AutomatedPickerRobotNavigator::SEGMENTATION_NAMESPACE = "segmentation";
std::string AutomatedPickerRobotNavigator::GOAL_NAMESPACE = "goal";
std::string AutomatedPickerRobotNavigator::JOINT_CONFIGURATIONS_NAMESPACE = "joints";
std::string AutomatedPickerRobotNavigator::MARKER_ARRAY_TOPIC = "object_array";

// global variables
static const double COLLISION_SPHERE_RADIUS = 0.01f;


AutomatedPickerRobotNavigator::AutomatedPickerRobotNavigator()
:RobotNavigator(),
 num_of_grasp_attempts_(4),
 offset_from_first_grasp_(0.01f), //1 cm
 attached_obj_bb_side_(0.1f)
{
	// TODO Auto-generated constructor stub
	GOAL_NAMESPACE = NODE_NAME + "/" + GOAL_NAMESPACE;
	SEGMENTATION_NAMESPACE = NODE_NAME + "/" + SEGMENTATION_NAMESPACE;
	JOINT_CONFIGURATIONS_NAMESPACE = NODE_NAME + "/" + JOINT_CONFIGURATIONS_NAMESPACE;

}

AutomatedPickerRobotNavigator::~AutomatedPickerRobotNavigator()
{
	// TODO Auto-generated destructor stub
	ROS_INFO_STREAM(NODE_NAME<<": Exiting navigator");
}

void AutomatedPickerRobotNavigator::setup()
{
	ros::NodeHandle nh;
	std::string nodeName = ros::this_node::getName();

	ROS_INFO_STREAM(NODE_NAME<<": Loading ros parameters");

	// getting ros parametets
	fetchParameters(NAVIGATOR_NAMESPACE);
	zone_selector_.fetchParameters(NODE_NAME);

	ROS_INFO_STREAM(NODE_NAME<<": Setting up execution Monitors");
	// setting up execution monitors
	{
		joint_state_recorder_.reset(new JointStateTrajectoryRecorder(joint_states_topic_));
		arm_controller_handler_.reset(new FollowJointTrajectoryControllerHandler(arm_group_name_,trajectory_action_service_));
		gripper_controller_handler_.reset(new GraspPoseControllerHandler(gripper_group_name_,grasp_action_service_));

		trajectory_execution_monitor_.addTrajectoryRecorder(joint_state_recorder_);
		trajectory_execution_monitor_.addTrajectoryControllerHandler(arm_controller_handler_);
		trajectory_execution_monitor_.addTrajectoryControllerHandler(gripper_controller_handler_);
	}

	ROS_INFO_STREAM(NODE_NAME<<": Setting up Service Clients");
    // setting up service clients, this is configuration specific
	{
		// segmentation
		seg_srv_ = nh.serviceClient<tabletop_object_detector::TabletopSegmentation>(segmentation_service_, true);

		//recognition
		//recognition_client_ = nh.serviceClient<mantis_perception::mantis_recognition>("/mantis_object_recognition");
		recognition_client_ = nh.serviceClient<mantis_perception::mantis_recognition>(recognition_service_,true);

		// path and grasp planning
		grasp_planning_client = nh.serviceClient<object_manipulation_msgs::GraspPlanning>(grasp_planning_service_, true);
		planning_service_client_ = nh.serviceClient<arm_navigation_msgs::GetMotionPlan>(path_planner_service_);

		// arm trajectory filter service
		trajectory_filter_service_client_ = nh.serviceClient<arm_navigation_msgs::FilterJointTrajectoryWithConstraints>(trajectory_filter_service_);
		ROS_INFO_STREAM(NODE_NAME<<": Waiting for trajectory filter service");
		trajectory_filter_service_client_.waitForExistence();
		ROS_INFO_STREAM(NODE_NAME<<": Trajectory filter service connected");

		// planing scene
		ROS_INFO_STREAM(NODE_NAME <<": Waiting for " + planning_scene_service_ + " service");
		ros::service::waitForService(planning_scene_service_);
		set_planning_scene_diff_client_ = nh.serviceClient<arm_navigation_msgs::SetPlanningSceneDiff>(planning_scene_service_);
	}

	// will use grasp execution client to request pre-grasp action since the default gripper controller handler
	// ignores this step.
	ROS_INFO_STREAM(NODE_NAME << ": Setting up Service Action Clients");
	{
		grasp_exec_action_client_ =
				boost::make_shared< GraspActionServerClient >(grasp_action_service_,true);
		while(!grasp_exec_action_client_->waitForServer(ros::Duration(0.5)))
		{
			ROS_INFO_STREAM(NODE_NAME << "Waiting for action service "<< grasp_action_service_);
		}
		ROS_INFO_STREAM(NODE_NAME<<" : Connected to action service "<<grasp_action_service_);
	}

	ROS_INFO_STREAM(NODE_NAME<<": Setting up ros publishers");
	{
		// setting up ros publishers
		marker_publisher_ = nh.advertise<visualization_msgs::Marker> (VISUALIZATION_TOPIC, 128);
		marker_array_pub_ = nh.advertise<visualization_msgs::MarkerArray>(MARKER_ARRAY_TOPIC,1);
		attached_object_publisher_ = nh.advertise<arm_navigation_msgs::AttachedCollisionObject> ("attached_collision_object_alternate", 1);

		// setting up timer obj
		marker_pub_timer_ = nh.createTimer(ros::Duration(0.4f),&AutomatedPickerRobotNavigator::callbackPublishMarkers,this);

		ROS_INFO_STREAM(NODE_NAME<<": Setting up dynamic libraries");

		// trajectory generators
		grasp_tester_ = GraspTesterPtr(new GraspSequenceValidator(&cm_, ik_plugin_name_));
		place_tester_ = PlaceSequencePtr(new PlaceSequenceValidator(&cm_, ik_plugin_name_));

		// trajectory callbacks
		trajectories_finished_function_ = boost::bind(&AutomatedPickerRobotNavigator::trajectoryFinishedCallback, this, true,_1);
		grasp_action_finished_function_ = boost::bind(&AutomatedPickerRobotNavigator::trajectoryFinishedCallback, this, false,_1);

	}

	ROS_INFO_STREAM(NODE_NAME<<": Setting up published markers");
	{
		visualization_msgs::Marker marker;
		marker.header.frame_id = cm_.getWorldFrameId();
		marker.ns = NODE_NAME;
		marker.type = visualization_msgs::Marker::SPHERE;
		marker.action = visualization_msgs::Marker::DELETE;
		tf::poseTFToMsg(tf::Transform::getIdentity(),marker.pose);

		// adding marker to map
		addMarker(MARKER_SEGMENTED_OBJECT,marker);
		addMarker(MARKER_ATTACHED_OBJECT,marker);

		// pick and place zone markers
		updateMarkerArrayMsg();
	}

	ROS_INFO_STREAM(NODE_NAME<<": Finished setup");
}

void AutomatedPickerRobotNavigator::fetchParameters(std::string nameSpace)
{
	RobotNavigator::fetchParameters(nameSpace);
	ros::param::param(nameSpace + "/" + PARAM_NAME_ATTACHED_OBJECT_BB_SIDE,attached_obj_bb_side_,
			attached_obj_bb_side_);
	ros::param::param(nameSpace + "/" + PARAM_NAME_NUM_GRASP_ATTEMTPTS,num_of_grasp_attempts_,
			num_of_grasp_attempts_);
	ros::param::param(nameSpace + "/" + PARAM_NAME_NEW_GRASP_OFFSET,offset_from_first_grasp_,
			offset_from_first_grasp_);
}

bool AutomatedPickerRobotNavigator::performSphereSegmentation()
{
	//  ===================================== preparing result objects =====================================
	  arm_navigation_msgs::CollisionObject obj;
	  int bestClusterIndex = -1;
	  sensor_msgs::PointCloud sphereCluster;

	  //  ===================================== calling segmentation =====================================
	  sphere_segmentation_.fetchParameters(SEGMENTATION_NAMESPACE);
	  bool success = sphere_segmentation_.segment(segmented_clusters_,obj,bestClusterIndex);

	  // ===================================== checking results ========================================
	  if(!success)
	  {
		  ROS_ERROR_STREAM(NODE_NAME<<": Sphere segmentation did not succeed");
		  return false;
	  }

	  //  ===================================== storing results =====================================
	  // assigning id first
	  obj.id = makeCollisionObjectNameFromModelId(0);

	  // retrieving segmented sphere cluster
	  sphere_segmentation_.getSphereCluster(sphereCluster);

	  // storing best cluster
	  segmented_clusters_.clear();
	  segmented_clusters_.push_back(sphereCluster);

	  // storing pose
	  geometry_msgs::PoseStamped pose;
	  pose.header.frame_id = obj.header.frame_id;
	  pose.pose = obj.poses[0];
	  recognized_obj_pose_map_[std::string(obj.id)] = pose;

	  // storing recognized object as model for planning

	  household_objects_database_msgs::DatabaseModelPoseList models;
	  household_objects_database_msgs::DatabaseModelPose model;
	  model.model_id = 0;
	  model.pose = pose;
	  model.confidence = 1.0f;
	  model.detector_name = "sphere_segmentation";
	  models.model_list.push_back(model);
	  recognized_models_.clear();
	  recognized_models_.push_back(models);
	  recognized_collision_object_ = obj;

	  //  ===================================== updating local planning scene =====================================
	  addDetectedObjectToLocalPlanningScene(obj);

	  //  ===================================== updating markers =====================================
	  // update markers
	  visualization_msgs::Marker &segMarker = getMarker(MARKER_SEGMENTED_OBJECT);
	  visualization_msgs::Marker &attachedMarker = getMarker(MARKER_ATTACHED_OBJECT);

	  // segmented object
	  collisionObjToMarker(obj,segMarker);
	  segMarker.action = visualization_msgs::Marker::ADD;
	  addMarker(MARKER_SEGMENTED_OBJECT,segMarker);

	  // attached object, hide for now until gripper is closed
	  collisionObjToMarker(obj,attachedMarker);
	  attachedMarker.action = visualization_msgs::Marker::DELETE;
	  attachedMarker.header.frame_id = gripper_link_name_;
	  addMarker(MARKER_ATTACHED_OBJECT,attachedMarker);

	  // ===================================== printing completion info message =====================================
	  arm_navigation_msgs::Shape &shape = obj.shapes[0];
	  ROS_INFO_STREAM("\n"<<NODE_NAME<<": Sphere Segmentation completed:\n");
	  ROS_INFO_STREAM("\tFrame id: "<<obj.header.frame_id<<"\n");
	  ROS_INFO_STREAM("\tRadius: "<<shape.dimensions[0]<<"\n");
	  ROS_INFO_STREAM("\tx: "<<pose.pose.position.x<<", y: "<<pose.pose.position.y
			  <<", z: "<<pose.pose.position.z<<"\n");

	return true;
}

bool AutomatedPickerRobotNavigator::performRecognition()
{
	// declaring temporary variables to generate object ids
#ifdef USE_SPHERE_SEGMENTATION

	static const int MaxIdCount = 8;
	static int CurrentIdCount = 1;

	ROS_WARN_STREAM(NODE_NAME<<": Using id "<<CurrentIdCount<<" for the next object");

	// assigning id ( if recognition is used the id returned in the result should be used instead)
	recognized_obj_id_ = CurrentIdCount;
	CurrentIdCount++;
	if(CurrentIdCount > MaxIdCount)
	{
		CurrentIdCount = 1;
	}

	// passed recognized object details to zone selector
	double bbBoxSide = zone_selector_.getPlaceZone().MinObjectSpacing;
	tf::Vector3 objSize = tf::Vector3(bbBoxSide,bbBoxSide,bbBoxSide);
	PickPlaceZoneSelector::ObjectDetails objDetails(tf::Transform::getIdentity(),objSize,
			CurrentIdCount,"next_object");
	zone_selector_.getPlaceZone().setNextObjectDetails(objDetails);

	// computing poses so that no move is attempted if no locations are available in the place zone.
	candidate_place_poses_.clear();
	if(!zone_selector_.generateNextLocationCandidates(candidate_place_poses_))
	{
		ROS_WARN_STREAM(NODE_NAME<<": Couldn't find available location for object, swapping zones and resetting counter.");
		// no more locations available, swapping zones
		zone_selector_.swapPickPlaceZones();
		CurrentIdCount = 1;
		return false;
	}

#else

	// preparing recognition results
	arm_navigation_msgs::CollisionObject obj;
	double sphereRadius = COLLISION_SPHERE_RADIUS;
	obj.id = makeCollisionObjectNameFromModelId(0);
	mantis_perception::mantis_recognition rec_srv;
	rec_srv.request.clusters = segmented_clusters_;
	rec_srv.request.table = segmentation_results_.table;

	// recognition call
	if (!recognition_client_.call(rec_srv))
	{
	  ROS_ERROR_STREAM(NODE_NAME<<": Call to mantis recognition service failed");
	  return false;
	}
	else
	{
		ROS_WARN_STREAM(NODE_NAME<<": Found object with id: "<<rec_srv.response.model_id);
	}

/*
 * Storing recognition results
 */
	// parsing pose results
	tf::Transform t = tf::Transform::getIdentity();
	nrg_object_recognition::pose &tempPose =  rec_srv.response.pose;
	t.setOrigin(tf::Vector3(tempPose.x,tempPose.y,tempPose.z));
	t.setRotation(tf::Quaternion(tf::Vector3(0.0f,0.0f,1.0f),tempPose.rotation));

	ROS_INFO_STREAM(NODE_NAME<<": Recognized object position in world coordinates is: [ "<<
			t.getOrigin().x()<<", "<<t.getOrigin().y()<<" "<<t.getOrigin().z()<<" ]");

	// storing pose
	geometry_msgs::PoseStamped pose;
	tf::poseTFToMsg(t,pose.pose);
	pose.header.frame_id = cm_.getWorldFrameId();
	recognized_obj_pose_map_[std::string(obj.id)] = pose;

	// adding collision obj to planning scene
	ROS_INFO_STREAM(NODE_NAME<<": Adding Object bounding sphere to planning scene with frame id "
			<<cm_.getWorldFrameId()<<" and radius: "<<sphereRadius);
	obj.header.frame_id = cm_.getWorldFrameId();
	obj.padding = 0;
	obj.shapes = std::vector<arm_navigation_msgs::Shape>();
	arm_navigation_msgs::Shape shape;
	shape.type = arm_navigation_msgs::Shape::SPHERE;
	//shape.dimensions.push_back(zone_selector_.getPlaceZone().MinObjectSpacing/2.0f); // radius;
	shape.dimensions.push_back(sphereRadius);
	obj.shapes.push_back(shape);
	obj.poses.push_back(pose.pose);
	obj.poses[0].position.z  = pose.pose.position.z - sphereRadius;
	addDetectedObjectToLocalPlanningScene(obj);

	// storing model detail for path planning
	household_objects_database_msgs::DatabaseModelPoseList models;
	household_objects_database_msgs::DatabaseModelPose model;
	model.model_id = 0;
	model.pose = pose;
	model.confidence = 1.0f;
	model.detector_name = "cfh_recognition";
	models.model_list.push_back(model);
	recognized_models_.clear();
	recognized_models_.push_back(models);

	// storing grasp candidate poses
	candidate_pick_poses_.clear();
	if(!rec_srv.response.pick_poses.empty())
	{
		candidate_pick_poses_.assign(rec_srv.response.pick_poses.begin(),rec_srv.response.pick_poses.end());
	}
	// Finished storing recognition results

	// ==================================================================================
	// passed recognized object details to zone selector
	updateMarkerArrayMsg();
	tf::Vector3 objSize = tf::Vector3(attached_obj_bb_side_,attached_obj_bb_side_,attached_obj_bb_side_);
	PickPlaceZoneSelector::ObjectDetails objDetails(tf::Transform::getIdentity(),objSize,
			rec_srv.response.model_id,rec_srv.response.label);
	zone_selector_.setNextObjectDetails(objDetails);

	// computing poses so that no move is attempted if no reachable locations are available in the place zone.
	candidate_place_poses_.clear();

	// generating locations in active place zones
	ROS_WARN_STREAM(NODE_NAME<<": using box with size "<<attached_obj_bb_side_);
	if(!zone_selector_.generateNextLocationCandidates(candidate_place_poses_))
	{
		ROS_WARN_STREAM(NODE_NAME<<": Couldn't find available location for object, swapping zones.");
		// no more locations available, swapping zones
		zone_selector_.goToNextPickZone();
		updateMarkerArrayMsg();
		return false;
	}

	// checking that found place location is reachable
	const static int maxAttempts = 5;
	static int attemptCounter = 0;
	if(!createCandidateGoalPoses(candidate_place_poses_))
	{
		if(attemptCounter == maxAttempts)
		{
			ROS_ERROR_STREAM(NODE_NAME<<": Couldn't find reachable place location after "
					<< maxAttempts<<" attempts, aborting ");
			attemptCounter = 0;
			zone_selector_.goToNextPickZone();
			updateMarkerArrayMsg();
		}
		else
		{
			ROS_WARN_STREAM(NODE_NAME<<": Place location is not reachable, trying again ");
			attemptCounter++;
			zone_selector_.removeLastObjectAdded();

		}
		return false;
	}

	ROS_INFO_STREAM(NODE_NAME<<": Found reachable place location for object ");

#endif

	return true;
}

bool AutomatedPickerRobotNavigator::performGraspPlanning()
{
	//  ===================================== saving current time stamp =====================================
	ros::WallTime start_time = ros::WallTime::now();

	//  ===================================== clearing results from last call =====================================
	grasp_pickup_goal_.target.potential_models.clear();
	grasp_candidates_.clear();

	//  ===================================== calling service =====================================
	// checking available recognized models
	if((recognized_models_.size() == 0) || (recognized_models_[0].model_list.size() == 0))
	{
	  return false;
	}

	// populating grasp plan request/response
	household_objects_database_msgs::DatabaseModelPose modelPose = recognized_models_[0].model_list[0];
	object_manipulation_msgs::GraspPlanning::Request request;
	object_manipulation_msgs::GraspPlanning::Response response;
	std::string modelId = makeCollisionObjectNameFromModelId(modelPose.model_id);
	modelPose.pose = recognized_obj_pose_map_[modelId];
	modelPose.pose.header.frame_id = recognized_model_description_.name; // should be updated during recognition stage
	modelPose.pose.header.stamp = ros::Time::now();
	request.arm_name = arm_group_name_;
	request.target.potential_models.push_back(modelPose);
	request.target.reference_frame_id = segmented_clusters_[0].header.frame_id;
	request.target.cluster = segmented_clusters_[0];

	// creating candidate grasp to evaluate
	if(!candidate_pick_poses_.empty())
	{
		std::vector<object_manipulation_msgs::Grasp> evaluateGrasps;
		for(std::vector<geometry_msgs::PoseStamped>::iterator i = candidate_pick_poses_.begin();
				i != candidate_pick_poses_.end(); i++)
		{
			object_manipulation_msgs::Grasp graspEval;
			graspEval.grasp_pose = i->pose;
			evaluateGrasps.push_back(graspEval);
		}
		request.grasps_to_evaluate.assign(evaluateGrasps.begin(),evaluateGrasps.end());
	}
	else
	{
		ROS_ERROR_STREAM(NODE_NAME<<": No candidate pick poses were returned from recognition, aborting");
		return false;
	}

	bool success = grasp_planning_client.call(request, response);

	// ===================================== checking results ========================================
	if(!success)
	{
		ROS_WARN_STREAM(NODE_NAME<<": grasp planning call unsuccessful, exiting");
		return false;
	}

	if(response.error_code.value != response.error_code.SUCCESS)
	{
		ROS_WARN_STREAM(NODE_NAME<<": grasp planning call returned error code, exiting " << response.error_code.value);
		return false;
	}

	if(response.grasps.size() == 0)
	{
		ROS_WARN_STREAM(NODE_NAME<<": No grasps returned in response");
		return false;
	}

	//TODO - actually deal with the different cases here, especially for the cluster planner
	if(request.target.reference_frame_id != recognized_model_description_.name ||
		  request.target.reference_frame_id != modelPose.pose.header.frame_id)
	{
	  ROS_WARN_STREAM(NODE_NAME<<": Cluster does not match recognition");
	}

	//  ===================================== storing results =====================================
	/* Storing grasp candidates:
	 * 	Grasp poses are poses of the wrist link in terms of the world,
	 * 	we need to get them relative to the object
	 */
	grasp_candidates_.assign(response.grasps.begin(),response.grasps.end());

	// instantiating needed transforms and poses
	tf::StampedTransform wrist_in_tcp_tf = tf::StampedTransform();
	tf::Transform object_in_world_tf; // will remove
	tf::Transform object_in_world_inverse_tf;// will remove

	// populating transforms
	tf::poseMsgToTF(modelPose.pose.pose, object_in_world_tf);
	object_in_world_inverse_tf = object_in_world_tf.inverse();
	_TfListener.lookupTransform(gripper_link_name_,wrist_link_name_,ros::Time(0),wrist_in_tcp_tf);

	// applying transformation to grasp pose so that the arm wrist relative to the object is obtained
	for(unsigned int i = 0; i < grasp_candidates_.size(); i++)
	{
		tf::Transform tcp_in_world_tf;
		tf::poseMsgToTF(grasp_candidates_[i].grasp_pose, tcp_in_world_tf);
		tf::poseTFToMsg(object_in_world_inverse_tf*(tcp_in_world_tf*wrist_in_tcp_tf),
			  grasp_candidates_[i].grasp_pose);
	}

	// storing grasp pickup goal to be used later during pick move sequence execution
	grasp_pickup_goal_.arm_name = arm_group_name_;
	grasp_pickup_goal_.collision_object_name = modelId;
	grasp_pickup_goal_.lift.direction.header.frame_id = cm_.getWorldFrameId();
	grasp_pickup_goal_.lift.direction.vector.z = 1.0;
	grasp_pickup_goal_.lift.desired_distance = .1;
	grasp_pickup_goal_.target.reference_frame_id = modelId;
	grasp_pickup_goal_.target.cluster = segmented_clusters_[0];
	grasp_pickup_goal_.allow_gripper_support_collision = true;
	grasp_pickup_goal_.collision_support_surface_name = "table";
	grasp_pickup_goal_.target.potential_models.push_back(modelPose);

	//  ===================================== updating local planning scene =====================================
	// updating gripper (not sure if this is necessary)
	tf::Transform first_grasp_in_world_tf;
	tf::poseMsgToTF(response.grasps[0].grasp_pose, first_grasp_in_world_tf);
	planning_models::KinematicState state(*current_robot_state_);
	state.updateKinematicStateWithLinkAt(gripper_link_name_, first_grasp_in_world_tf);


	// ===================================== printing completion info message =====================================
	ROS_INFO_STREAM(NODE_NAME<<": Grasp is " << response.grasps[0].grasp_pose.position.x << " "
			  << response.grasps[0].grasp_pose.position.y << " "
			  << response.grasps[0].grasp_pose.position.z);

	ROS_INFO_STREAM(NODE_NAME<<": Cloud header " << segmented_clusters_[0].header.frame_id);
	ROS_INFO_STREAM(NODE_NAME<<": Recognition pose frame " << modelPose.pose.header.frame_id);

	return true;
}

bool AutomatedPickerRobotNavigator::performSegmentation()
{
	bool success = false;
	success =  RobotNavigator::performSegmentation();
	if(!success)
	{
		return false;
	}

	// clearing obstacle clusters from zone
	zone_selector_.clearObstableClusters();

	// check if at least one cluster is located in pick zone
	std::vector<int> inZone;
	success = zone_selector_.isInPickZone(segmented_clusters_,inZone);
	if(!success)
	{
		ROS_WARN_STREAM(NODE_NAME<<": Neither cluster was found in pick zone, swapping zones");
		zone_selector_.goToNextPickZone();

		return false;
	}
	else
	{
		ROS_INFO_STREAM(NODE_NAME<<": A total of "<<inZone.size()<<" were found in pick zone");

		// finding clusters outside of pick zone to be used as obstacles
		std::vector<sensor_msgs::PointCloud> tempArray;
		for(std::size_t i = 0;i < segmented_clusters_.size();i++)
		{
			if(std::find(inZone.begin(),inZone.end(),i) == inZone.end())
			{
				// not in pick zone
				tempArray.push_back(segmented_clusters_[i]);
			}
		}

		// adding clusters
		if(tempArray.size() > 0)
		{
			zone_selector_.addObstacleClusters(tempArray);
			tempArray.clear();
		}

		// retaining only cluster in pick zone
		for(std::size_t i = 0;i < inZone.size();i++)
		{
			tempArray.push_back(segmented_clusters_[inZone[i]]);
		}
		segmented_clusters_.assign(tempArray.begin(),tempArray.end());
	}

	updateMarkerArrayMsg();

#ifdef USE_SPHERE_SEGMENTATION
	success =  performSphereSegmentation();
	if(!success)
	{
		return false;
	}

#endif
	return true;

}

bool AutomatedPickerRobotNavigator::moveArmToSide()
{

    joint_configuration_.fetchParameters(JOINT_CONFIGURATIONS_NAMESPACE);
    return updateChangesToPlanningScene() && moveArm(arm_group_name_,joint_configuration_.SideAngles);
}

bool AutomatedPickerRobotNavigator::moveArmThroughPickSequence()
{
	// pushing local changes to planning scene
	updateChangesToPlanningScene();

	// grasp planning
	bool success = performGraspPlanning();
	if(!success)
	{
		zone_selector_.removeLastObjectAdded();
		ROS_INFO_STREAM(NODE_NAME<<": Pick Move attempt aborted");
		return false;
	}

	// will attempt to grasp object multiple times if current pick attempt fails
	object_manipulation_msgs::Grasp firstGrasp =  grasp_candidates_[0];
	double angleIncrement = 2*M_PI/((double)num_of_grasp_attempts_);
	int graspIndex = 0; // index to last successful grasp;

	for(int i = 0; i <= num_of_grasp_attempts_; i++)
	{
		// creating pick move sequence
		ROS_INFO_STREAM(NODE_NAME<<": Generating new move sequence for pick");
		std::vector<object_manipulator::GraspExecutionInfo> graspSequence;
		createPickMoveSequence(grasp_pickup_goal_,grasp_candidates_,graspSequence);

		// try each successful grasp
		success = false;
		graspIndex = 0; // index to grasp array
		BOOST_FOREACH(object_manipulator::GraspExecutionInfo graspMoves,graspSequence)
		{
			bool proceed = (graspMoves.result_.result_code == object_manipulation_msgs::GraspResult::SUCCESS);
			if(proceed)
			{
				ROS_INFO_STREAM(NODE_NAME<<": Attempting Pick grasp sequence");
				success = attemptGraspSequence(arm_group_name_,graspMoves,false);
				if(!success)
				{
					ROS_INFO_STREAM(NODE_NAME<<": Grasp pick move failed");
				}
				else
				{
				  ROS_INFO_STREAM(NODE_NAME<<": Grasp pick move succeeded");
				}

				break;
			}
			else
			{
				ROS_WARN_STREAM(NODE_NAME<<": Grasp pick move unreachable, skipping to next.");
			}
			graspIndex++;
		}

		if(!success)
		{
			if(i == num_of_grasp_attempts_ )
			{
				// currently on last iteration, all options have been attempted, exiting
				ROS_ERROR_STREAM(NODE_NAME<<"No more pick attempts remain, aborting pick");
				zone_selector_.removeLastObjectAdded();
				return false;
			}
			else
			{
				ROS_WARN_STREAM(NODE_NAME<<"Generating new object pose with offset: "<<offset_from_first_grasp_
						<<" and angle: "<<angleIncrement * i << " from original");
			}

			// generating object6 pose near original
			tf::Transform newObjTf;
			geometry_msgs::Pose newObjPose;
			std::vector<geometry_msgs::Pose> candidatePoses;

			// converting first pose into tf type
			tf::poseMsgToTF(recognized_obj_pose_map_[grasp_pickup_goal_.collision_object_name].pose,newObjTf);

			// offsetting first grasp a small amount in the x-y plane
			tf::Vector3 offsetVect = newObjTf.getOrigin();
			offsetVect.setX(offsetVect.getX() + offset_from_first_grasp_ * std::cos(angleIncrement * i));
			offsetVect.setY(offsetVect.getY() + offset_from_first_grasp_ * std::sin(angleIncrement * i));
			newObjTf.setOrigin(offsetVect);

			// converting back into pose msg
			tf::poseTFToMsg(newObjTf,newObjPose);

			// updating grasp pick goal object
			household_objects_database_msgs::DatabaseModelPose &model =	grasp_pickup_goal_.target.potential_models[0];
			model.pose.pose = newObjPose;

			// updating approach distance
			for(std::size_t j = 0; j < grasp_candidates_.size(); j++)
			{
				object_manipulation_msgs::Grasp &g = grasp_candidates_[j];
				g.desired_approach_distance = 0.02f;
			}

		}
		else
		{
			break;
		}

	}



	// storing current grasp data for marker publishing
	object_manipulation_msgs::Grasp tempGrasp;
	tf::StampedTransform gripperTcpToWrist = tf::StampedTransform();// wrist pose relative to gripper
	tf::Transform wristInObjPose;
	_TfListener.lookupTransform(gripper_link_name_,wrist_link_name_,ros::Time(0),gripperTcpToWrist);

	tf::poseMsgToTF(grasp_candidates_[graspIndex].grasp_pose,wristInObjPose);
	tf::poseTFToMsg(wristInObjPose*(gripperTcpToWrist.inverse()),tempGrasp.grasp_pose);
	current_grasp_map_[arm_group_name_] = tempGrasp;
	current_grasped_object_name_[arm_group_name_] = grasp_pickup_goal_.collision_object_name;

	// updating attached object marker pose
	if(hasMarker(MARKER_ATTACHED_OBJECT))
	{
		visualization_msgs::Marker &m = getMarker(MARKER_ATTACHED_OBJECT);
		tf::poseTFToMsg(wristInObjPose.inverse(),m.pose);
		m.header.frame_id = gripper_link_name_;
		addMarker(MARKER_ATTACHED_OBJECT,m);
	}

	return success;
}

bool AutomatedPickerRobotNavigator::moveArmThroughPlaceSequence()
{
	if(place_sequence_.empty())
	{
		// no valid place moves have been stored
		return false;
	}

	// resetting scene
	updateChangesToPlanningScene();

	// starting timer
	ros::WallTime start_time = ros::WallTime::now();

	// try each place sequence candidate
	bool success = attemptPlaceSequence(arm_group_name_,place_sequence_[0]);
	if(success)
	{
		ROS_INFO_STREAM(NODE_NAME<<": Grasp place move succeeded");
	}
	else
	{
		ROS_ERROR_STREAM(NODE_NAME<<"Grasp place move failed, aborting");
		zone_selector_.removeLastObjectAdded();
		return false;
	}

	updateMarkerArrayMsg();
	return success;
}

bool AutomatedPickerRobotNavigator::createCandidateGoalPoses(std::vector<geometry_msgs::PoseStamped> &placePoses)
{
	// creating grasp goal
	grasp_place_goal_.arm_name = arm_group_name_;
	grasp_place_goal_.grasp = current_grasp_map_[arm_group_name_];
	grasp_place_goal_.desired_retreat_distance = .1;
	grasp_place_goal_.min_retreat_distance = .1;
	grasp_place_goal_.approach.desired_distance = .1;
	grasp_place_goal_.approach.min_distance = .1;
	grasp_place_goal_.approach.direction.header.frame_id = cm_.getWorldFrameId();
	grasp_place_goal_.approach.direction.vector.x = 0.0;
	grasp_place_goal_.approach.direction.vector.y = 0.0;
	grasp_place_goal_.approach.direction.vector.z = -1.0;
	grasp_place_goal_.collision_object_name = "attached_"+current_grasped_object_name_[arm_group_name_];
	grasp_place_goal_.allow_gripper_support_collision = true;
	grasp_place_goal_.collision_support_surface_name = "table";
	grasp_place_goal_.place_padding = .02;

	// finding valid place move sequence
	std::vector<object_manipulator::PlaceExecutionInfo> placeMoveCandidates;
	createPlaceMoveSequence(grasp_place_goal_,placePoses,placeMoveCandidates);

	// checking results
	place_sequence_.clear();
	bool success = false;
	std::vector<object_manipulator::PlaceExecutionInfo>::iterator iter;
	for(iter = placeMoveCandidates.begin(); iter != placeMoveCandidates.end(); iter++)
	{
		object_manipulator::PlaceExecutionInfo &placeMove = *iter;
		if(placeMove.result_.result_code == object_manipulation_msgs::PlaceLocationResult::SUCCESS)
		{
			ROS_INFO_STREAM(NODE_NAME<<": Found valid place sequence");

			// storing result and exiting;
			place_sequence_.push_back(placeMove);
			success = true;
			break;
		}
	}

	return success;
}

void AutomatedPickerRobotNavigator::callbackPublishMarkers(const ros::TimerEvent &evnt)
{
	RobotNavigator::callbackPublishMarkers(evnt);
	//marker_array_pub_.publish(marker_array_msg_);

	boost::mutex::scoped_lock lock(marker_array_mutex_);
	{
		if(!marker_array_msg_.markers.empty())
		{
			marker_array_pub_.publish(marker_array_msg_);
		}
	}
}

void AutomatedPickerRobotNavigator::updateMarkerArrayMsg()
{
	typedef std::vector<visualization_msgs::Marker>::iterator Iter;
	boost::mutex::scoped_lock lock(marker_array_mutex_);
	{
		for(Iter i = marker_array_msg_.markers.begin(); i != marker_array_msg_.markers.end(); i++)
		{
			visualization_msgs::Marker &marker = *i;
			marker.action = visualization_msgs::Marker::DELETE;
		}
		marker_array_pub_.publish(marker_array_msg_);
		marker_array_msg_.markers.clear();
		zone_selector_.getAllActiveZonesCombinedMarkers(marker_array_msg_);
		zone_selector_.getAllObjectsMarkers(marker_array_msg_);
	}
}

void AutomatedPickerRobotNavigator::generateGraspPoses(const geometry_msgs::Pose &pose,int numCandidates,
		std::vector<geometry_msgs::Pose> &poses)
{
	tf::Transform graspTf = tf::Transform::getIdentity();
	tf::Transform candidateTf;
	tfScalar angle = tfScalar(2*M_PI/(double(numCandidates)));

	// converting initial pose to tf
	tf::poseMsgToTF(pose,graspTf);

	// obtaining approach vector
	tf::Vector3 approachVector = graspTf.getBasis().getColumn(2);

	for(int i = 0; i < numCandidates; i++)
	{
		candidateTf = graspTf*tf::Transform(tf::Quaternion(approachVector,i*angle),
				tf::Vector3(0.0f,0.0f,0.0f));
		geometry_msgs::Pose candidatePose = geometry_msgs::Pose();
		tf::poseTFToMsg(candidateTf,candidatePose);
		poses.push_back(candidatePose);
	}
}

