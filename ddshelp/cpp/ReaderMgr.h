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

#ifndef DDSHELP_READER_MGR_H
#define DDSHELP_READER_MGR_H

#include "EntityMgr.h"
#include <ccpp_dds_dcps.h>
#include <assert.h>

namespace DDSHelp
{
  class SubscriberMgr;
  class TopicDescriptionMgr;

  class ReaderMgrBase
    : public EntityMgr
  {
  public:
    DDS::DataReader_ptr
    operator->()
    { assert(m_reader_base); return m_reader_base; }

    operator DDS::DataReader_ptr()
    { assert(m_reader_base); return m_reader_base; }

    operator bool()
    { return m_reader_base; }

    void
    check(DDS::ReturnCode_t retcode,
	  const char* operation);

  protected:
    ReaderMgrBase(SubscriberMgr& subscriber,
		  TopicDescriptionMgr& topic_description)
      : m_subscriber(subscriber),
	m_topic_description(topic_description)
    {}

    ~ReaderMgrBase()
    { assert(!m_reader_base); }

    void
    create_base(const DDS::DataReaderQos& reader_qos);

    void
    destroy_base();

  private:
    ReaderMgrBase(const ReaderMgrBase&); // disabled
    ReaderMgrBase& operator=(const ReaderMgrBase&); // disabled

  protected:
    SubscriberMgr& m_subscriber;
    TopicDescriptionMgr& m_topic_description;
    DDS::DataReader_var m_reader_base;
  };

  template <class DR>
  class ReaderMgr
    : public ReaderMgrBase
  {
  public:
    ReaderMgr(SubscriberMgr& subscriber,
	      TopicDescriptionMgr& topic_description)
      : ReaderMgrBase(subscriber, topic_description),
	m_reader(0)
    {}

    ~ReaderMgr()
    { destroy(); }

    DR*
    operator->()
    { assert(m_reader); return m_reader; }

    operator DR*()
    { assert(m_reader); return m_reader; }

    operator bool()
    { return m_reader; }

    void
    create(const DDS::DataReaderQos& reader_qos
	   = *DDS::DomainParticipantFactory::datareader_qos_default());

    void
    destroy();

  private:
    DR* m_reader;
  };

  template <class DR>
  void
  ReaderMgr<DR>::create(const DDS::DataReaderQos& reader_qos)
  {
    create_base(reader_qos);

    m_reader = DR::_narrow(m_reader_base);
    assert(m_reader);
  }

  template <class DR>
  void
  ReaderMgr<DR>::destroy()
  {
    DDS::release(m_reader);
    m_reader = 0;

    destroy_base();
  }
}

#endif // DDSHELP_READER_MGR_H
