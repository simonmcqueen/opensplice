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

#ifndef TRANSPONDER_H
#define TRANSPONDER_H

/* touchstone includes */
#include "Partition.h"
#include "Processor.h"
/* ddshelp includes */
#include "ConditionMgr.h"
#include "TopicMgr.h"
#include "ReaderMgr.h"
#include "WriterMgr.h"
#include "Dispatcher.h"
#include "Thread.h"
/* DDS vendor abstraction includes */
#include "touchstone_vendor_abstraction_cpp.h"

using namespace DDSTouchStone;
using namespace DDSHelp;
using namespace DDS;
using namespace std;

class Transponder
  : public ProcessorBase
{
public:
  explicit
  Transponder(Partition& partition);

  ~Transponder();

  void
  create(const transponderDef& def);

  void
  dispose();

  void
  update_def(const transponderDef& def);

  void
  update_qos(const transponderQos& qos);
  
private:
  Partition&
  partition()
  { return m_partition; }

  ULong
  partition_id()
  { return partition().partition_id(); }

  Processor&
  processor()
  { return partition().processor(); }
  
  ParticipantMgr&
  participant()
  { return partition().participant(); }

  PublisherMgr&
  publisher()
  { return partition().publisher(); }

  SubscriberMgr&
  subscriber()
  { return partition().subscriber(); }

  ReaderMgr<transponderQosDataReader>&
  qos_reader()
  { return processor().transponder_qos_reader(); }

  WriterMgr<transponderQosDataWriter>&
  qos_writer()
  { return processor().transponder_qos_writer(); }

  void
  set_topics();

  void
  set_qos();

  void
  thread();

  bool
  read_latency_message(Condition_ptr condition);

  Partition& m_partition;
  QueryConditionMgr m_qos_query;
  Dispatcher<Transponder> m_dispatcher;
  TopicMgr<latency_messageTypeSupport> m_topic;
  ReaderMgr<latency_messageDataReader> m_reader;
  StatusConditionMgr m_reader_condition;
  Attachment<Transponder> m_reader_attachment;
  TopicMgr<latency_messageTypeSupport> m_echo_topic;
  WriterMgr<latency_messageDataWriter> m_writer;
  Thread<Transponder> m_thread;
  bool m_active;
  int m_config_number;
  transponderDef m_def;
  transponderQos m_qos;
  DDS::DataReaderQos m_reader_qos;
  DDS::DataWriterQos m_writer_qos;
  latency_messageSeq m_reader_messages;
  SampleInfoSeq m_reader_infos;
};

#endif // TRANSPONDER_H
