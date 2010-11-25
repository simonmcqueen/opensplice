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

#include "ReaderMgr.h"
#include "SubscriberMgr.h"
#include "TopicMgr.h"
#include "DDSError.h"

using namespace DDSHelp;
using namespace DDS;

void
ReaderMgrBase::check(ReturnCode_t retcode,
		     const char* operation)
{
  if (retcode != RETCODE_OK)
    {
      throw DDSError(retcode, operation, m_topic_description.topic_name());
    }
}

void
ReaderMgrBase::create_base(const DataReaderQos& reader_qos)
{
  destroy_base();

  m_reader_base = m_subscriber->create_datareader(m_topic_description,
						  reader_qos,
						  0,
						  ANY_STATUS);
  if (!m_reader_base)
    {
      throw DDSError("FAILED", "Subscriber::create_datareader", m_topic_description.topic_name());
    }

  m_entity = m_reader_base;
}

void
ReaderMgrBase::destroy_base()
{
  if (m_reader_base)
    {
      DDS::ReturnCode_t retcode = m_subscriber->delete_datareader(m_reader_base);
      if (retcode != DDS::RETCODE_OK)
	{
	  throw DDSError(retcode, "Subscriber::delete_datareader", m_topic_description.topic_name());
	}
      m_reader_base = 0;
      m_entity = 0;
    }
}
