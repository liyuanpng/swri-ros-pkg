"""autogenerated by genpy from nrg_object_recognition/segmentationRequest.msg. Do not edit."""
import sys
python3 = True if sys.hexversion > 0x03000000 else False
import genpy
import struct

import std_msgs.msg
import sensor_msgs.msg

class segmentationRequest(genpy.Message):
  _md5sum = "fe21ba53cba1fa67e508830d4391bec5"
  _type = "nrg_object_recognition/segmentationRequest"
  _has_header = False #flag to mark the presence of a Header object
  _full_text = """sensor_msgs/PointCloud2 scene
float32 min_x
float32 max_x
float32 min_y
float32 max_y
float32 min_z
float32 max_z

================================================================================
MSG: sensor_msgs/PointCloud2
# This message holds a collection of N-dimensional points, which may
# contain additional information such as normals, intensity, etc. The
# point data is stored as a binary blob, its layout described by the
# contents of the "fields" array.

# The point cloud data may be organized 2d (image-like) or 1d
# (unordered). Point clouds organized as 2d images may be produced by
# camera depth sensors such as stereo or time-of-flight.

# Time of sensor data acquisition, and the coordinate frame ID (for 3d
# points).
Header header

# 2D structure of the point cloud. If the cloud is unordered, height is
# 1 and width is the length of the point cloud.
uint32 height
uint32 width

# Describes the channels and their layout in the binary data blob.
PointField[] fields

bool    is_bigendian # Is this data bigendian?
uint32  point_step   # Length of a point in bytes
uint32  row_step     # Length of a row in bytes
uint8[] data         # Actual point data, size is (row_step*height)

bool is_dense        # True if there are no invalid points

================================================================================
MSG: std_msgs/Header
# Standard metadata for higher-level stamped data types.
# This is generally used to communicate timestamped data 
# in a particular coordinate frame.
# 
# sequence ID: consecutively increasing ID 
uint32 seq
#Two-integer timestamp that is expressed as:
# * stamp.secs: seconds (stamp_secs) since epoch
# * stamp.nsecs: nanoseconds since stamp_secs
# time-handling sugar is provided by the client library
time stamp
#Frame this data is associated with
# 0: no frame
# 1: global frame
string frame_id

================================================================================
MSG: sensor_msgs/PointField
# This message holds the description of one point entry in the
# PointCloud2 message format.
uint8 INT8    = 1
uint8 UINT8   = 2
uint8 INT16   = 3
uint8 UINT16  = 4
uint8 INT32   = 5
uint8 UINT32  = 6
uint8 FLOAT32 = 7
uint8 FLOAT64 = 8

string name      # Name of field
uint32 offset    # Offset from start of point struct
uint8  datatype  # Datatype enumeration, see above
uint32 count     # How many elements in the field

"""
  __slots__ = ['scene','min_x','max_x','min_y','max_y','min_z','max_z']
  _slot_types = ['sensor_msgs/PointCloud2','float32','float32','float32','float32','float32','float32']

  def __init__(self, *args, **kwds):
    """
    Constructor. Any message fields that are implicitly/explicitly
    set to None will be assigned a default value. The recommend
    use is keyword arguments as this is more robust to future message
    changes.  You cannot mix in-order arguments and keyword arguments.

    The available fields are:
       scene,min_x,max_x,min_y,max_y,min_z,max_z

    :param args: complete set of field values, in .msg order
    :param kwds: use keyword arguments corresponding to message field names
    to set specific fields.
    """
    if args or kwds:
      super(segmentationRequest, self).__init__(*args, **kwds)
      #message fields cannot be None, assign default values for those that are
      if self.scene is None:
        self.scene = sensor_msgs.msg.PointCloud2()
      if self.min_x is None:
        self.min_x = 0.
      if self.max_x is None:
        self.max_x = 0.
      if self.min_y is None:
        self.min_y = 0.
      if self.max_y is None:
        self.max_y = 0.
      if self.min_z is None:
        self.min_z = 0.
      if self.max_z is None:
        self.max_z = 0.
    else:
      self.scene = sensor_msgs.msg.PointCloud2()
      self.min_x = 0.
      self.max_x = 0.
      self.min_y = 0.
      self.max_y = 0.
      self.min_z = 0.
      self.max_z = 0.

  def _get_types(self):
    """
    internal API method
    """
    return self._slot_types

  def serialize(self, buff):
    """
    serialize message into buffer
    :param buff: buffer, ``StringIO``
    """
    try:
      _x = self
      buff.write(_struct_3I.pack(_x.scene.header.seq, _x.scene.header.stamp.secs, _x.scene.header.stamp.nsecs))
      _x = self.scene.header.frame_id
      length = len(_x)
      if python3 or type(_x) == unicode:
        _x = _x.encode('utf-8')
        length = len(_x)
      buff.write(struct.pack('<I%ss'%length, length, _x))
      _x = self
      buff.write(_struct_2I.pack(_x.scene.height, _x.scene.width))
      length = len(self.scene.fields)
      buff.write(_struct_I.pack(length))
      for val1 in self.scene.fields:
        _x = val1.name
        length = len(_x)
        if python3 or type(_x) == unicode:
          _x = _x.encode('utf-8')
          length = len(_x)
        buff.write(struct.pack('<I%ss'%length, length, _x))
        _x = val1
        buff.write(_struct_IBI.pack(_x.offset, _x.datatype, _x.count))
      _x = self
      buff.write(_struct_B2I.pack(_x.scene.is_bigendian, _x.scene.point_step, _x.scene.row_step))
      _x = self.scene.data
      length = len(_x)
      # - if encoded as a list instead, serialize as bytes instead of string
      if type(_x) in [list, tuple]:
        buff.write(struct.pack('<I%sB'%length, length, *_x))
      else:
        buff.write(struct.pack('<I%ss'%length, length, _x))
      _x = self
      buff.write(_struct_B6f.pack(_x.scene.is_dense, _x.min_x, _x.max_x, _x.min_y, _x.max_y, _x.min_z, _x.max_z))
    except struct.error as se: self._check_types(se)
    except TypeError as te: self._check_types(te)

  def deserialize(self, str):
    """
    unpack serialized message in str into this message instance
    :param str: byte array of serialized message, ``str``
    """
    try:
      if self.scene is None:
        self.scene = sensor_msgs.msg.PointCloud2()
      end = 0
      _x = self
      start = end
      end += 12
      (_x.scene.header.seq, _x.scene.header.stamp.secs, _x.scene.header.stamp.nsecs,) = _struct_3I.unpack(str[start:end])
      start = end
      end += 4
      (length,) = _struct_I.unpack(str[start:end])
      start = end
      end += length
      if python3:
        self.scene.header.frame_id = str[start:end].decode('utf-8')
      else:
        self.scene.header.frame_id = str[start:end]
      _x = self
      start = end
      end += 8
      (_x.scene.height, _x.scene.width,) = _struct_2I.unpack(str[start:end])
      start = end
      end += 4
      (length,) = _struct_I.unpack(str[start:end])
      self.scene.fields = []
      for i in range(0, length):
        val1 = sensor_msgs.msg.PointField()
        start = end
        end += 4
        (length,) = _struct_I.unpack(str[start:end])
        start = end
        end += length
        if python3:
          val1.name = str[start:end].decode('utf-8')
        else:
          val1.name = str[start:end]
        _x = val1
        start = end
        end += 9
        (_x.offset, _x.datatype, _x.count,) = _struct_IBI.unpack(str[start:end])
        self.scene.fields.append(val1)
      _x = self
      start = end
      end += 9
      (_x.scene.is_bigendian, _x.scene.point_step, _x.scene.row_step,) = _struct_B2I.unpack(str[start:end])
      self.scene.is_bigendian = bool(self.scene.is_bigendian)
      start = end
      end += 4
      (length,) = _struct_I.unpack(str[start:end])
      start = end
      end += length
      if python3:
        self.scene.data = str[start:end].decode('utf-8')
      else:
        self.scene.data = str[start:end]
      _x = self
      start = end
      end += 25
      (_x.scene.is_dense, _x.min_x, _x.max_x, _x.min_y, _x.max_y, _x.min_z, _x.max_z,) = _struct_B6f.unpack(str[start:end])
      self.scene.is_dense = bool(self.scene.is_dense)
      return self
    except struct.error as e:
      raise genpy.DeserializationError(e) #most likely buffer underfill


  def serialize_numpy(self, buff, numpy):
    """
    serialize message with numpy array types into buffer
    :param buff: buffer, ``StringIO``
    :param numpy: numpy python module
    """
    try:
      _x = self
      buff.write(_struct_3I.pack(_x.scene.header.seq, _x.scene.header.stamp.secs, _x.scene.header.stamp.nsecs))
      _x = self.scene.header.frame_id
      length = len(_x)
      if python3 or type(_x) == unicode:
        _x = _x.encode('utf-8')
        length = len(_x)
      buff.write(struct.pack('<I%ss'%length, length, _x))
      _x = self
      buff.write(_struct_2I.pack(_x.scene.height, _x.scene.width))
      length = len(self.scene.fields)
      buff.write(_struct_I.pack(length))
      for val1 in self.scene.fields:
        _x = val1.name
        length = len(_x)
        if python3 or type(_x) == unicode:
          _x = _x.encode('utf-8')
          length = len(_x)
        buff.write(struct.pack('<I%ss'%length, length, _x))
        _x = val1
        buff.write(_struct_IBI.pack(_x.offset, _x.datatype, _x.count))
      _x = self
      buff.write(_struct_B2I.pack(_x.scene.is_bigendian, _x.scene.point_step, _x.scene.row_step))
      _x = self.scene.data
      length = len(_x)
      # - if encoded as a list instead, serialize as bytes instead of string
      if type(_x) in [list, tuple]:
        buff.write(struct.pack('<I%sB'%length, length, *_x))
      else:
        buff.write(struct.pack('<I%ss'%length, length, _x))
      _x = self
      buff.write(_struct_B6f.pack(_x.scene.is_dense, _x.min_x, _x.max_x, _x.min_y, _x.max_y, _x.min_z, _x.max_z))
    except struct.error as se: self._check_types(se)
    except TypeError as te: self._check_types(te)

  def deserialize_numpy(self, str, numpy):
    """
    unpack serialized message in str into this message instance using numpy for array types
    :param str: byte array of serialized message, ``str``
    :param numpy: numpy python module
    """
    try:
      if self.scene is None:
        self.scene = sensor_msgs.msg.PointCloud2()
      end = 0
      _x = self
      start = end
      end += 12
      (_x.scene.header.seq, _x.scene.header.stamp.secs, _x.scene.header.stamp.nsecs,) = _struct_3I.unpack(str[start:end])
      start = end
      end += 4
      (length,) = _struct_I.unpack(str[start:end])
      start = end
      end += length
      if python3:
        self.scene.header.frame_id = str[start:end].decode('utf-8')
      else:
        self.scene.header.frame_id = str[start:end]
      _x = self
      start = end
      end += 8
      (_x.scene.height, _x.scene.width,) = _struct_2I.unpack(str[start:end])
      start = end
      end += 4
      (length,) = _struct_I.unpack(str[start:end])
      self.scene.fields = []
      for i in range(0, length):
        val1 = sensor_msgs.msg.PointField()
        start = end
        end += 4
        (length,) = _struct_I.unpack(str[start:end])
        start = end
        end += length
        if python3:
          val1.name = str[start:end].decode('utf-8')
        else:
          val1.name = str[start:end]
        _x = val1
        start = end
        end += 9
        (_x.offset, _x.datatype, _x.count,) = _struct_IBI.unpack(str[start:end])
        self.scene.fields.append(val1)
      _x = self
      start = end
      end += 9
      (_x.scene.is_bigendian, _x.scene.point_step, _x.scene.row_step,) = _struct_B2I.unpack(str[start:end])
      self.scene.is_bigendian = bool(self.scene.is_bigendian)
      start = end
      end += 4
      (length,) = _struct_I.unpack(str[start:end])
      start = end
      end += length
      if python3:
        self.scene.data = str[start:end].decode('utf-8')
      else:
        self.scene.data = str[start:end]
      _x = self
      start = end
      end += 25
      (_x.scene.is_dense, _x.min_x, _x.max_x, _x.min_y, _x.max_y, _x.min_z, _x.max_z,) = _struct_B6f.unpack(str[start:end])
      self.scene.is_dense = bool(self.scene.is_dense)
      return self
    except struct.error as e:
      raise genpy.DeserializationError(e) #most likely buffer underfill

_struct_I = genpy.struct_I
_struct_IBI = struct.Struct("<IBI")
_struct_3I = struct.Struct("<3I")
_struct_2I = struct.Struct("<2I")
_struct_B2I = struct.Struct("<B2I")
_struct_B6f = struct.Struct("<B6f")
"""autogenerated by genpy from nrg_object_recognition/segmentationResponse.msg. Do not edit."""
import sys
python3 = True if sys.hexversion > 0x03000000 else False
import genpy
import struct

import std_msgs.msg
import sensor_msgs.msg

class segmentationResponse(genpy.Message):
  _md5sum = "f94e5e56fc7ff419809a40b3ff371b4c"
  _type = "nrg_object_recognition/segmentationResponse"
  _has_header = False #flag to mark the presence of a Header object
  _full_text = """sensor_msgs/PointCloud2[] clusters

================================================================================
MSG: sensor_msgs/PointCloud2
# This message holds a collection of N-dimensional points, which may
# contain additional information such as normals, intensity, etc. The
# point data is stored as a binary blob, its layout described by the
# contents of the "fields" array.

# The point cloud data may be organized 2d (image-like) or 1d
# (unordered). Point clouds organized as 2d images may be produced by
# camera depth sensors such as stereo or time-of-flight.

# Time of sensor data acquisition, and the coordinate frame ID (for 3d
# points).
Header header

# 2D structure of the point cloud. If the cloud is unordered, height is
# 1 and width is the length of the point cloud.
uint32 height
uint32 width

# Describes the channels and their layout in the binary data blob.
PointField[] fields

bool    is_bigendian # Is this data bigendian?
uint32  point_step   # Length of a point in bytes
uint32  row_step     # Length of a row in bytes
uint8[] data         # Actual point data, size is (row_step*height)

bool is_dense        # True if there are no invalid points

================================================================================
MSG: std_msgs/Header
# Standard metadata for higher-level stamped data types.
# This is generally used to communicate timestamped data 
# in a particular coordinate frame.
# 
# sequence ID: consecutively increasing ID 
uint32 seq
#Two-integer timestamp that is expressed as:
# * stamp.secs: seconds (stamp_secs) since epoch
# * stamp.nsecs: nanoseconds since stamp_secs
# time-handling sugar is provided by the client library
time stamp
#Frame this data is associated with
# 0: no frame
# 1: global frame
string frame_id

================================================================================
MSG: sensor_msgs/PointField
# This message holds the description of one point entry in the
# PointCloud2 message format.
uint8 INT8    = 1
uint8 UINT8   = 2
uint8 INT16   = 3
uint8 UINT16  = 4
uint8 INT32   = 5
uint8 UINT32  = 6
uint8 FLOAT32 = 7
uint8 FLOAT64 = 8

string name      # Name of field
uint32 offset    # Offset from start of point struct
uint8  datatype  # Datatype enumeration, see above
uint32 count     # How many elements in the field

"""
  __slots__ = ['clusters']
  _slot_types = ['sensor_msgs/PointCloud2[]']

  def __init__(self, *args, **kwds):
    """
    Constructor. Any message fields that are implicitly/explicitly
    set to None will be assigned a default value. The recommend
    use is keyword arguments as this is more robust to future message
    changes.  You cannot mix in-order arguments and keyword arguments.

    The available fields are:
       clusters

    :param args: complete set of field values, in .msg order
    :param kwds: use keyword arguments corresponding to message field names
    to set specific fields.
    """
    if args or kwds:
      super(segmentationResponse, self).__init__(*args, **kwds)
      #message fields cannot be None, assign default values for those that are
      if self.clusters is None:
        self.clusters = []
    else:
      self.clusters = []

  def _get_types(self):
    """
    internal API method
    """
    return self._slot_types

  def serialize(self, buff):
    """
    serialize message into buffer
    :param buff: buffer, ``StringIO``
    """
    try:
      length = len(self.clusters)
      buff.write(_struct_I.pack(length))
      for val1 in self.clusters:
        _v1 = val1.header
        buff.write(_struct_I.pack(_v1.seq))
        _v2 = _v1.stamp
        _x = _v2
        buff.write(_struct_2I.pack(_x.secs, _x.nsecs))
        _x = _v1.frame_id
        length = len(_x)
        if python3 or type(_x) == unicode:
          _x = _x.encode('utf-8')
          length = len(_x)
        buff.write(struct.pack('<I%ss'%length, length, _x))
        _x = val1
        buff.write(_struct_2I.pack(_x.height, _x.width))
        length = len(val1.fields)
        buff.write(_struct_I.pack(length))
        for val2 in val1.fields:
          _x = val2.name
          length = len(_x)
          if python3 or type(_x) == unicode:
            _x = _x.encode('utf-8')
            length = len(_x)
          buff.write(struct.pack('<I%ss'%length, length, _x))
          _x = val2
          buff.write(_struct_IBI.pack(_x.offset, _x.datatype, _x.count))
        _x = val1
        buff.write(_struct_B2I.pack(_x.is_bigendian, _x.point_step, _x.row_step))
        _x = val1.data
        length = len(_x)
        # - if encoded as a list instead, serialize as bytes instead of string
        if type(_x) in [list, tuple]:
          buff.write(struct.pack('<I%sB'%length, length, *_x))
        else:
          buff.write(struct.pack('<I%ss'%length, length, _x))
        buff.write(_struct_B.pack(val1.is_dense))
    except struct.error as se: self._check_types(se)
    except TypeError as te: self._check_types(te)

  def deserialize(self, str):
    """
    unpack serialized message in str into this message instance
    :param str: byte array of serialized message, ``str``
    """
    try:
      if self.clusters is None:
        self.clusters = None
      end = 0
      start = end
      end += 4
      (length,) = _struct_I.unpack(str[start:end])
      self.clusters = []
      for i in range(0, length):
        val1 = sensor_msgs.msg.PointCloud2()
        _v3 = val1.header
        start = end
        end += 4
        (_v3.seq,) = _struct_I.unpack(str[start:end])
        _v4 = _v3.stamp
        _x = _v4
        start = end
        end += 8
        (_x.secs, _x.nsecs,) = _struct_2I.unpack(str[start:end])
        start = end
        end += 4
        (length,) = _struct_I.unpack(str[start:end])
        start = end
        end += length
        if python3:
          _v3.frame_id = str[start:end].decode('utf-8')
        else:
          _v3.frame_id = str[start:end]
        _x = val1
        start = end
        end += 8
        (_x.height, _x.width,) = _struct_2I.unpack(str[start:end])
        start = end
        end += 4
        (length,) = _struct_I.unpack(str[start:end])
        val1.fields = []
        for i in range(0, length):
          val2 = sensor_msgs.msg.PointField()
          start = end
          end += 4
          (length,) = _struct_I.unpack(str[start:end])
          start = end
          end += length
          if python3:
            val2.name = str[start:end].decode('utf-8')
          else:
            val2.name = str[start:end]
          _x = val2
          start = end
          end += 9
          (_x.offset, _x.datatype, _x.count,) = _struct_IBI.unpack(str[start:end])
          val1.fields.append(val2)
        _x = val1
        start = end
        end += 9
        (_x.is_bigendian, _x.point_step, _x.row_step,) = _struct_B2I.unpack(str[start:end])
        val1.is_bigendian = bool(val1.is_bigendian)
        start = end
        end += 4
        (length,) = _struct_I.unpack(str[start:end])
        start = end
        end += length
        if python3:
          val1.data = str[start:end].decode('utf-8')
        else:
          val1.data = str[start:end]
        start = end
        end += 1
        (val1.is_dense,) = _struct_B.unpack(str[start:end])
        val1.is_dense = bool(val1.is_dense)
        self.clusters.append(val1)
      return self
    except struct.error as e:
      raise genpy.DeserializationError(e) #most likely buffer underfill


  def serialize_numpy(self, buff, numpy):
    """
    serialize message with numpy array types into buffer
    :param buff: buffer, ``StringIO``
    :param numpy: numpy python module
    """
    try:
      length = len(self.clusters)
      buff.write(_struct_I.pack(length))
      for val1 in self.clusters:
        _v5 = val1.header
        buff.write(_struct_I.pack(_v5.seq))
        _v6 = _v5.stamp
        _x = _v6
        buff.write(_struct_2I.pack(_x.secs, _x.nsecs))
        _x = _v5.frame_id
        length = len(_x)
        if python3 or type(_x) == unicode:
          _x = _x.encode('utf-8')
          length = len(_x)
        buff.write(struct.pack('<I%ss'%length, length, _x))
        _x = val1
        buff.write(_struct_2I.pack(_x.height, _x.width))
        length = len(val1.fields)
        buff.write(_struct_I.pack(length))
        for val2 in val1.fields:
          _x = val2.name
          length = len(_x)
          if python3 or type(_x) == unicode:
            _x = _x.encode('utf-8')
            length = len(_x)
          buff.write(struct.pack('<I%ss'%length, length, _x))
          _x = val2
          buff.write(_struct_IBI.pack(_x.offset, _x.datatype, _x.count))
        _x = val1
        buff.write(_struct_B2I.pack(_x.is_bigendian, _x.point_step, _x.row_step))
        _x = val1.data
        length = len(_x)
        # - if encoded as a list instead, serialize as bytes instead of string
        if type(_x) in [list, tuple]:
          buff.write(struct.pack('<I%sB'%length, length, *_x))
        else:
          buff.write(struct.pack('<I%ss'%length, length, _x))
        buff.write(_struct_B.pack(val1.is_dense))
    except struct.error as se: self._check_types(se)
    except TypeError as te: self._check_types(te)

  def deserialize_numpy(self, str, numpy):
    """
    unpack serialized message in str into this message instance using numpy for array types
    :param str: byte array of serialized message, ``str``
    :param numpy: numpy python module
    """
    try:
      if self.clusters is None:
        self.clusters = None
      end = 0
      start = end
      end += 4
      (length,) = _struct_I.unpack(str[start:end])
      self.clusters = []
      for i in range(0, length):
        val1 = sensor_msgs.msg.PointCloud2()
        _v7 = val1.header
        start = end
        end += 4
        (_v7.seq,) = _struct_I.unpack(str[start:end])
        _v8 = _v7.stamp
        _x = _v8
        start = end
        end += 8
        (_x.secs, _x.nsecs,) = _struct_2I.unpack(str[start:end])
        start = end
        end += 4
        (length,) = _struct_I.unpack(str[start:end])
        start = end
        end += length
        if python3:
          _v7.frame_id = str[start:end].decode('utf-8')
        else:
          _v7.frame_id = str[start:end]
        _x = val1
        start = end
        end += 8
        (_x.height, _x.width,) = _struct_2I.unpack(str[start:end])
        start = end
        end += 4
        (length,) = _struct_I.unpack(str[start:end])
        val1.fields = []
        for i in range(0, length):
          val2 = sensor_msgs.msg.PointField()
          start = end
          end += 4
          (length,) = _struct_I.unpack(str[start:end])
          start = end
          end += length
          if python3:
            val2.name = str[start:end].decode('utf-8')
          else:
            val2.name = str[start:end]
          _x = val2
          start = end
          end += 9
          (_x.offset, _x.datatype, _x.count,) = _struct_IBI.unpack(str[start:end])
          val1.fields.append(val2)
        _x = val1
        start = end
        end += 9
        (_x.is_bigendian, _x.point_step, _x.row_step,) = _struct_B2I.unpack(str[start:end])
        val1.is_bigendian = bool(val1.is_bigendian)
        start = end
        end += 4
        (length,) = _struct_I.unpack(str[start:end])
        start = end
        end += length
        if python3:
          val1.data = str[start:end].decode('utf-8')
        else:
          val1.data = str[start:end]
        start = end
        end += 1
        (val1.is_dense,) = _struct_B.unpack(str[start:end])
        val1.is_dense = bool(val1.is_dense)
        self.clusters.append(val1)
      return self
    except struct.error as e:
      raise genpy.DeserializationError(e) #most likely buffer underfill

_struct_I = genpy.struct_I
_struct_IBI = struct.Struct("<IBI")
_struct_B = struct.Struct("<B")
_struct_2I = struct.Struct("<2I")
_struct_B2I = struct.Struct("<B2I")
class segmentation(object):
  _type          = 'nrg_object_recognition/segmentation'
  _md5sum = '96781735db4389fa2867dac25719da4b'
  _request_class  = segmentationRequest
  _response_class = segmentationResponse
