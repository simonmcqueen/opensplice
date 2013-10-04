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

#ifndef DDSHELP_PARTICIPANT_MGR_H
#define DDSHELP_PARTICIPANT_MGR_H

#include <ccpp_dds_dcps.h>
#include <assert.h>

namespace DDSHelp
{
  class ParticipantMgr
  {
  public:
    ParticipantMgr()
    {}

    ~ParticipantMgr()
    { destroy(); }

    DDS::DomainParticipant_ptr
    operator->()
    { assert(m_participant); return m_participant; }

    operator DDS::DomainParticipant_ptr()
    { assert(m_participant); return m_participant; }

    operator bool()
    { return m_participant; }

    void
    create(DDS::DomainId_t domain_id,
	   const DDS::DomainParticipantQos& participant_qos
	   = *DDS::DomainParticipantFactory::participant_qos_default());

    void
    destroy();

    void
    check(DDS::ReturnCode_t retcode,
	  const char* operation);

  private:
    ParticipantMgr(const ParticipantMgr&); // disabled
    ParticipantMgr& operator=(const ParticipantMgr&); // disabled

    static DDS::DomainParticipantFactory_var sm_factory;
    DDS::DomainId_t m_domain_id;
    DDS::DomainParticipant_var m_participant;
  };
}

#endif // DDSHELP_PARTICIPANT_MGR_H
