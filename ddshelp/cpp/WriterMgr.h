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

#ifndef DDSHELP_WRITER_MGR_H
#define DDSHELP_WRITER_MGR_H

#include "EntityMgr.h"
#include <ccpp_dds_dcps.h>
#include <assert.h>

namespace DDSHelp
{
  class PublisherMgr;
  class TopicMgrBase;

  class WriterMgrBase
    : public EntityMgr
  {
  public:
    DDS::DataWriter_ptr
    operator->()
    { assert(m_writer_base); return m_writer_base; }

    operator DDS::DataWriter_ptr()
    { assert(m_writer_base); return m_writer_base; }

    operator bool()
    { return m_writer_base.in() != 0; }

    void
    check(DDS::ReturnCode_t retcode,
	  const char* operation);

  protected:
    WriterMgrBase(PublisherMgr& publisher,
		  TopicMgrBase& topic)
      : m_publisher(publisher),
	m_topic(topic)
    {}

    ~WriterMgrBase()
    { assert(!m_writer_base); }

    void
    create_base(const DDS::DataWriterQos& writer_qos);

    void
    destroy_base();

  private:
    WriterMgrBase(const WriterMgrBase&); // disabled
    WriterMgrBase& operator=(const WriterMgrBase&); // disabled

  protected:
    PublisherMgr& m_publisher;
    TopicMgrBase& m_topic;
    DDS::DataWriter_var m_writer_base;
  };

  template <class DW>
  class WriterMgr
    : public WriterMgrBase
  {
  public:
    WriterMgr(PublisherMgr& publisher,
	      TopicMgrBase& topic)
      : WriterMgrBase(publisher, topic),
	m_writer(0)
    {}

    ~WriterMgr()
    { destroy(); }

    DW*
    operator->()
    { assert(m_writer); return m_writer; }

    operator DW*()
    { assert(m_writer); return m_writer; }

    operator bool()
    { return m_writer != 0; }

    void
    create(const DDS::DataWriterQos& writer_qos
	   = *DDS::DomainParticipantFactory::datawriter_qos_default());

    void
    destroy();

  private:
    DW* m_writer;
  };

  template <class DW>
  void
  WriterMgr<DW>::create(const DDS::DataWriterQos& writer_qos)
  {
    create_base(writer_qos);

    m_writer = DW::_narrow(m_writer_base);
    assert(m_writer);
  }

  template <class DW>
  void
  WriterMgr<DW>::destroy()
  {
    DDS::release(m_writer);
    m_writer = 0;

    destroy_base();
  }
}

#endif // DDSHELP_WRITER_MGR_H
