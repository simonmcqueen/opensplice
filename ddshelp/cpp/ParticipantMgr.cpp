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

#include "ParticipantMgr.h"
#include "DDSError.h"

using namespace DDSHelp;
using namespace DDS;

void
ParticipantMgr::create(DomainId_t domain_id,
		       const DDS::DomainParticipantQos& participant_qos)
{
  destroy();

  m_domain_id = domain_id;

  if (!sm_factory)
    {
      sm_factory = DomainParticipantFactory::get_instance();
      if (!sm_factory)
	{
	  throw DDSError("returned 0", "DomainParticipantFactory::get_instance", "");
	}
    }

  m_participant = sm_factory->create_participant(m_domain_id,
						 participant_qos,
						 0,
						 ANY_STATUS);
  if (!m_participant)
    {
      throw DDSError("DOMAIN_NOT_RUNNING", "DomainParticipantFactory::create_participant", "");
    }
}

void
ParticipantMgr::destroy()
{
  if (m_participant)
    {
      assert(sm_factory);

      ReturnCode_t retcode = sm_factory->delete_participant(m_participant);
      if (retcode != RETCODE_OK)
	{
	  throw DDSError(retcode, "DomainParticipantFactory::delete_participant", "");
	}

      m_participant = 0;
      m_domain_id = 0;
    }
}

void
ParticipantMgr::check(ReturnCode_t retcode,
		      const char* operation)
{
  if (retcode != RETCODE_OK)
    {
      throw DDSError(retcode,
		     operation,
		     "");	// REVISIT - DomainId?
    }
}

DomainParticipantFactory_var ParticipantMgr::sm_factory;
