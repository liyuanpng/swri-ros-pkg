/*
 * Software License Agreement (BSD License)
 *
 * Copyright (c) 2011, Southwest Research Institute
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *       * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *       * Neither the name of the Southwest Research Institute, nor the names
 *       of its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "simple_message.h"
#include "log_wrapper.h"

using namespace industrial::byte_array;

namespace industrial
{

namespace simple_message
{

SimpleMessage::SimpleMessage(void)
{
}

SimpleMessage::~SimpleMessage(void)
{
}


bool SimpleMessage::init(int msgType, int commType, int replyCode, ByteArray & data )
{
  this->setMessageType(msgType);
  this->setCommType(commType);
  this->setReplyCode(replyCode);
  this->data_.copyFrom(data);

  return this->validateMessage();
}

bool SimpleMessage::init(ByteArray & msg)
{
  int dataSize;
  bool rtn;

  if (msg.getBufferSize() > this->getHeaderSize())
  {
    dataSize = msg.getBufferSize() - this->getHeaderSize();
    msg.unload(this->data_.getRawDataPtr(), dataSize);
    msg.unload(this->reply_code_);
    msg.unload(this->comm_type_);
    msg.unload(this->message_type_);
    rtn = this->validateMessage();
  }
  else
  {
    rtn = false;
  }
  return rtn;
}

void SimpleMessage::toByteArray(ByteArray & msg)
{
  msg.init();

  msg.load(this->getMessageType());
  msg.load(this->getCommType());
  msg.load(this->getReplyCode());
  msg.load(this->getData().getRawDataPtr(), this->data_.getBufferSize());

}


void SimpleMessage::setData( ByteArray & data)
{
  this->data_.copyFrom(data);
}


bool SimpleMessage::validateMessage()
{

  if ( StandardMsgTypes::UNUSED == this->getMessageType())
  {
    LOG_WARN("Invalid message type: %u", this->getMessageType());
    return false;
  }

  if ( CommTypes::UNUSED == this->getCommType())
  {
    LOG_WARN("Invalid comms. type: %u", this->getCommType());
    return false;
  }

  if (
      (CommTypes::SERVICE_REPLY ==  this->getCommType() &&
          ReplyTypes::UNUSED == this->getReplyCode()) ||
          ((CommTypes::SERVICE_REPLY !=  this->getCommType() &&
              ReplyTypes::UNUSED != this->getReplyCode()))
  )
  {
    LOG_WARN("Invalid reply. Comm type: %u, Reply type: %u",
             this->getCommType(), this->getReplyCode());
    return false;
  }

  return true;
}



	
} // namespace simple_message
} // namespace industrial