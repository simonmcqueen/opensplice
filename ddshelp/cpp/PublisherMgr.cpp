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

#include "PublisherMgr.h"
#include "ParticipantMgr.h"
#include "DDSError.h"

using namespace DDSHelp;
using namespace DDS;

void
PublisherMgr::create(const PublisherQos& publisher_qos)
{
  destroy();

  m_partition_name = "PARTITION_NAME_NOT_SPECIFIED"; // REVISIT - get from qos?

  m_publisher = m_participant->create_publisher(publisher_qos,
						0,
						ANY_STATUS);
  if (!m_publisher)
    {
      throw DDSError("FAILED",
		     "DomainParticipant::create_publisher",
		     m_partition_name);
    }
}

void
PublisherMgr::create_w_partition(const char* partition_name)
{
  destroy();

  m_partition_name = partition_name;

  PublisherQos publisher_qos;
  m_participant->get_default_publisher_qos(publisher_qos);
  publisher_qos.partition.name.length(1);
  publisher_qos.partition.name[0] = DDS::string_dup(m_partition_name);

  m_publisher = m_participant->create_publisher(publisher_qos,
						0,
						ANY_STATUS);
  if (!m_publisher)
    {
      throw DDSError("FAILED",
		     "DomainParticipant::create_publisher",
		     m_partition_name);
    }
}

void
PublisherMgr::destroy()
{
  if (m_publisher)
    {
      ReturnCode_t retcode = m_participant->delete_publisher(m_publisher);
      check(retcode,
	    "DomainParticipant::delete_publisher");
      
      m_publisher = 0;
      m_partition_name = (char*)0;
    }
}

void
PublisherMgr::check(ReturnCode_t retcode,
		    const char* operation)
{
  if (retcode != RETCODE_OK)
    {
      throw DDSError(retcode,
		     operation,
		     m_partition_name);
    }
}
