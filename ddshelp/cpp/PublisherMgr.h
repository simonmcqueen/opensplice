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

#ifndef DDSHELP_PUBLISHER_MGR_H
#define DDSHELP_PUBLISHER_MGR_H

#include <ccpp_dds_dcps.h>
#include <assert.h>

namespace DDSHelp
{
  class ParticipantMgr;

  class PublisherMgr
  {
  public:
    explicit
    PublisherMgr(ParticipantMgr& participant)
      : m_participant(participant)
    {}

    ~PublisherMgr()
    { destroy(); }

    DDS::Publisher_ptr
    operator->()
    { assert(m_publisher); return m_publisher; }

    operator DDS::Publisher_ptr()
    { assert(m_publisher); return m_publisher; }

    operator bool()
    { return m_publisher.in() != 0; }

    ParticipantMgr&
    participant()
    { return m_participant; }

    void
    create(const DDS::PublisherQos& publisher_qos
	   = *DDS::DomainParticipantFactory::publisher_qos_default());

    void
    create_w_partition(const char* partition_name);

    void
    destroy();

    void
    check(DDS::ReturnCode_t retcode,
	  const char* operation);

  private:
    PublisherMgr(const PublisherMgr&); // disabled
    PublisherMgr& operator=(const PublisherMgr&); // disabled

    ParticipantMgr& m_participant;
    DDS::String_var m_partition_name;
    DDS::Publisher_var m_publisher;
  };
}

#endif // DDSHELP_PUBLISHER_MGR_H
