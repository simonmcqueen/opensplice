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

#include "TopicMgr.h"
#include "ParticipantMgr.h"
#include "DDSError.h"

using namespace DDSHelp;
using namespace DDS;

void
TopicDescriptionMgr::check(ReturnCode_t retcode,
			   const char* operation)
{
  if (retcode != RETCODE_OK)
    {
      throw DDSError(retcode,
		     operation,
		     m_topic_name);
    }
}

void
TopicMgrBase::create(const char* topic_name,
		     const TopicQos& topic_qos)
{
  destroy();

  assert(m_type_support);
  ReturnCode_t retcode = m_type_support->register_type(m_participant,
						       m_type_name);
  check(retcode,
	"TypeSupport::register_type");

  m_topic_name = topic_name;

  m_topic = m_participant->create_topic(m_topic_name,
					m_type_name,
					topic_qos,
					0,
					ANY_STATUS);
  if (!m_topic)
    {
      throw DDSError("FAILED", "DomainParticipant::create_topic", m_topic_name);
    }
  m_topic_description = m_topic;
}

void
TopicMgrBase::destroy()
{
  if (m_topic)
    {
      ReturnCode_t retcode = m_participant->delete_topic(m_topic);
      check(retcode,
	    "DomainParticipant::delete_topic");

      m_topic = 0;
      m_topic_description = 0;
      m_topic_name = (char*)0;
    }
}

void
FilteredTopicMgr::create(const char* topic_name,
			 const char* filter_expression,
			 const StringSeq& expression_parameters)
{
  destroy();

  m_topic_name = topic_name;

  m_filtered_topic = m_participant->create_contentfilteredtopic(m_topic_name,
								m_related_topic,
								filter_expression,
								expression_parameters);
  if (!m_filtered_topic)
    {
      throw DDSError("FAILED", "DomainParticipant::create_contentfilteredtopic", m_topic_name);
    }
  m_topic_description = m_filtered_topic;
}

void
FilteredTopicMgr::destroy()
{
  if (m_filtered_topic)
    {
      ReturnCode_t retcode = m_participant->delete_contentfilteredtopic(m_filtered_topic);
      check(retcode,
	    "DomainParticipant::delete_contentfilteredtopic");

      m_filtered_topic = 0;
      m_topic_description = 0;
      m_topic_name = (char*)0;
    }
}
