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

// -*- mode: c++ -*-

#ifndef DDSHELP_TOPIC_MGR_H
#define DDSHELP_TOPIC_MGR_H

#include <ccpp_dds_dcps.h>
#include <assert.h>

namespace DDSHelp
{
  class ParticipantMgr;

  class TopicDescriptionMgr
  {
  public:
    DDS::TopicDescription_ptr
    operator->()
    { assert(m_topic_description); return m_topic_description; }

    operator DDS::TopicDescription_ptr()
    { assert(m_topic_description); return m_topic_description; }

    operator bool()
    { return m_topic_description; }

    ParticipantMgr&
    participant()
    { return m_participant; }

    const char*
    topic_name()
    { return m_topic_name; }

    void
    check(DDS::ReturnCode_t retcode,
	  const char* operation);

  protected:
    TopicDescriptionMgr(ParticipantMgr& participant)
      : m_participant(participant),
	m_topic_description(0)
    {}

    ~TopicDescriptionMgr()
    { assert(!m_topic_description); }

  private:
    TopicDescriptionMgr(const TopicDescriptionMgr&); // disabled
    TopicDescriptionMgr& operator=(const TopicDescriptionMgr&); // disabled

  protected:
    ParticipantMgr& m_participant;
    DDS::String_var m_topic_name;
    DDS::TopicDescription_ptr m_topic_description; // managed by derived class
  };

  class TopicMgrBase
    : public TopicDescriptionMgr
  {
  public:
    DDS::Topic_ptr
    operator->()
    { assert(m_topic); return m_topic; }

    operator DDS::Topic_ptr()
    { assert(m_topic); return m_topic; }

    operator bool()
    { return m_topic; }

    void
    create(const char* topic_name,
	   const DDS::TopicQos& topic_qos
	   = *DDS::DomainParticipantFactory::topic_qos_default());

    void
    destroy();

  protected:
    explicit
    TopicMgrBase(ParticipantMgr& participant,
		 DDS::TypeSupport_ptr type_support)
      : TopicDescriptionMgr(participant),
	m_type_support(type_support)
    { assert(m_type_support); m_type_name = m_type_support->get_type_name(); }

    ~TopicMgrBase()
    { destroy(); }

  private:
    DDS::TypeSupport_var m_type_support;
    DDS::String_var m_type_name;
    DDS::Topic_var m_topic;
  };

  template <class TS>
  class TopicMgr
    : public TopicMgrBase
  {
  public:
    explicit
    TopicMgr(ParticipantMgr& participant)
      : TopicMgrBase(participant,
		     new TS())
    {}
  };

  class FilteredTopicMgr
    : public TopicDescriptionMgr
  {
  public:
    explicit
    FilteredTopicMgr(TopicMgrBase& related_topic)
      : TopicDescriptionMgr(related_topic.participant()),
	m_related_topic(related_topic)
    {}

    ~FilteredTopicMgr()
    { destroy(); }

    DDS::ContentFilteredTopic_ptr
    operator->()
    { assert(m_filtered_topic); return m_filtered_topic; }

    operator DDS::ContentFilteredTopic_ptr()
    { assert(m_filtered_topic); return m_filtered_topic; }

    operator bool()
    { return m_filtered_topic; }

    void
    create(const char* topic_name,
	   const char* filter_expression,
	   const DDS::StringSeq& expression_parameters);

    void
    destroy();

  private:  
    TopicMgrBase& m_related_topic;
    DDS::ContentFilteredTopic_var m_filtered_topic;
  };
}

#endif // DDSHELP_TOPIC_MGR_H
