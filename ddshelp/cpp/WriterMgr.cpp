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

#include "WriterMgr.h"
#include "PublisherMgr.h"
#include "TopicMgr.h"
#include "DDSError.h"

using namespace DDSHelp;
using namespace DDS;

void
WriterMgrBase::check(ReturnCode_t retcode,
		     const char* operation)
{
  if (retcode != RETCODE_OK)
    {
      throw DDSError(retcode, operation, m_topic.topic_name());
    }
}

void
WriterMgrBase::create_base(const DataWriterQos& writer_qos)
{
  destroy_base();

  m_writer_base = m_publisher->create_datawriter(m_topic,
						 writer_qos,
						 0,
						 ANY_STATUS);
  if (!m_writer_base)
    {
      throw DDSError("FAILED", "Publisher::create_datawriter", m_topic.topic_name());
    }

  m_entity = m_writer_base;
}

void
WriterMgrBase::destroy_base()
{
  if (m_writer_base)
    {
      DDS::ReturnCode_t retcode = m_publisher->delete_datawriter(m_writer_base);
      if (retcode != DDS::RETCODE_OK)
	{
	  throw DDSError(retcode, "Publisher::delete_datawriter", m_topic.topic_name());
	}
      m_writer_base = 0;
      m_entity = 0;
    }
}
