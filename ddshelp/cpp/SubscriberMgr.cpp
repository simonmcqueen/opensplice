/*
 *  DDSTouchStone: a scenario-driven Open Source benchmarking framework
 *  for evaluating the performance of OMG DDS compliant implementations.
 *
 *  Copyright (C) 2008-2009 PrismTech Ltd.
 *  ddstouchstone@prismtech.com
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License Version 3 dated 29 June 2007, as published by the
 *  Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with DDSTouchStone; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "SubscriberMgr.h"
#include "ParticipantMgr.h"
#include "DDSError.h"

using namespace DDSHelp;
using namespace DDS;

void
SubscriberMgr::create(const SubscriberQos& subscriber_qos)
{
  destroy();

  m_partition_name = "PARTITION_NAME_NOT_SPECIFIED";

  m_subscriber = m_participant->create_subscriber(subscriber_qos,
						  0,
						  ANY_STATUS);
  if (!m_subscriber)
    {
      throw DDSError("FAILED",
		     "DomainParticipant::create_subscriber",
		     m_partition_name);
    }
}

void
SubscriberMgr::create_w_partition(const char* partition_name)
{
  destroy();

  m_partition_name = partition_name;

  SubscriberQos subscriber_qos;
  m_participant->get_default_subscriber_qos(subscriber_qos);
  subscriber_qos.partition.name.length(1);
  subscriber_qos.partition.name[0] = DDS::string_dup(m_partition_name);

  m_subscriber = m_participant->create_subscriber(subscriber_qos,
						  0,
						  ANY_STATUS);
  if (!m_subscriber)
    {
      throw DDSError("FAILED",
		     "DomainParticipant::create_subscriber",
		     m_partition_name);
    }
}

void
SubscriberMgr::destroy()
{
  if (m_subscriber)
    {
      ReturnCode_t retcode = m_participant->delete_subscriber(m_subscriber);
      check(retcode,
	    "DomainParticipant::delete_subscriber");
      
      m_subscriber = 0;
      m_partition_name = (char*)0;
    }
}

void
SubscriberMgr::check(ReturnCode_t retcode,
		     const char* operation)
{
  if (retcode != RETCODE_OK)
    {
      throw DDSError(retcode,
		     operation,
		     m_partition_name);
    }
}
