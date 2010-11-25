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

#ifndef DDSHELP_SUBSCRIBER_MGR_H
#define DDSHELP_SUBSCRIBER_MGR_H

#include <ccpp_dds_dcps.h>
#include <assert.h>

namespace DDSHelp
{
  class ParticipantMgr;

  class SubscriberMgr
  {
  public:
    explicit
    SubscriberMgr(ParticipantMgr& participant)
      : m_participant(participant)
    {}

    ~SubscriberMgr()
    { destroy(); }

    DDS::Subscriber_ptr
    operator->()
    { assert(m_subscriber); return m_subscriber; }

    operator DDS::Subscriber_ptr()
    { assert(m_subscriber); return m_subscriber; }

    operator bool()
    { return m_subscriber; }

    ParticipantMgr&
    participant()
    { return m_participant; }

    void
    create(const DDS::SubscriberQos& subscriber_qos
	   = *DDS::DomainParticipantFactory::subscriber_qos_default());

    void
    create_w_partition(const char* partition_name);

    void
    destroy();

    void
    check(DDS::ReturnCode_t retcode,
	  const char* operation);

  private:
    SubscriberMgr(const SubscriberMgr&); // disabled
    SubscriberMgr& operator=(const SubscriberMgr&); // disabled

    ParticipantMgr& m_participant;
    DDS::String_var m_partition_name;
    DDS::Subscriber_var m_subscriber;
  };
}

#endif // DDSHELP_SUBSCRIBER_MGR_H
