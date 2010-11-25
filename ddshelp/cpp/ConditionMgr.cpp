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

#include "ConditionMgr.h"
#include "EntityMgr.h"
#include "ReaderMgr.h"
#include "DDSError.h"

using namespace DDSHelp;
using namespace DDS;

// REVISIT - get name for DDSErrors from EntityMgr/ReaderMgr?

void
StatusConditionMgr::get()
{
  release();

  m_status_condition = m_entity->get_statuscondition();
  if (!m_status_condition)
    {
      throw DDSError("FAILED", "Entity::get_statuscondtion");
    }
  m_condition = m_status_condition;
}

void
StatusConditionMgr::release()
{
  m_condition = 0;
  m_status_condition = 0;
}

void
ReadConditionMgr::create(SampleStateMask sample_states,
			 ViewStateMask view_states,
			 InstanceStateMask instance_states)
{
  destroy();

  m_read_condition = m_reader->create_readcondition(sample_states,
						    view_states,
						    instance_states);
  if (!m_read_condition)
    {
      throw DDSError("FAILED", "DataReader::create_readcondition");
    }
  m_condition = m_read_condition;
}

void
ReadConditionMgr::destroy()
{
  if (m_read_condition)
    {
      ReturnCode_t retcode = m_reader->delete_readcondition(m_read_condition);
      if (retcode != RETCODE_OK)
	{
	  throw DDSError(retcode, "Reader::delete_readcondition");
	}

      m_condition = 0;
      m_read_condition = 0;
    }
}

void
QueryConditionMgr::create(SampleStateMask sample_states,
			  ViewStateMask view_states,
			  InstanceStateMask instance_states,
			  const char* query_expression,
			  const StringSeq& query_parameters)
{
  destroy();

  m_query_condition = m_reader->create_querycondition(sample_states,
						      view_states,
						      instance_states,
						      query_expression,
						      query_parameters);
  if (!m_query_condition)
    {
      throw DDSError("FAILED", "DataReader::create_querycondition");
    }
  m_condition = m_query_condition;
}

void
QueryConditionMgr::destroy()
{
  if (m_query_condition)
    {
      ReturnCode_t retcode = m_reader->delete_readcondition(m_query_condition);
      if (retcode != RETCODE_OK)
	{
	  throw DDSError(retcode, "Reader::delete_querycondition");
	}

      m_condition = 0;
      m_query_condition = 0;
    }
}
