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

#ifndef RECEIVER_H
#define RECEIVER_H

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
/* standard includes */
#include <map>

using namespace DDSTouchStone;
using namespace DDSHelp;
using namespace DDS;
using namespace std;

class Receiver
  : public ProcessorBase
{
public:
  explicit
  Receiver(Partition& partition);

  ~Receiver();

  void
  create(const receiverDef& def);

  void
  dispose();

  void
  update_def(const receiverDef& def);

  void
  update_qos(const receiverQos& qos);
  
private:

  /* These next three classes are expected to become a little
   * more complicated over time... */

  struct AliveWriterKey
  {
    ULong random_id;
  };

  struct AliveWriterData
  {
    AliveWriterData() : start_time(0){};
    double start_time;
  };

  class AliveWriterCompare;
  friend class AliveWriterCompare;

  class AliveWriterCompare
  {
  public:
    bool
    operator()(const AliveWriterKey& lhs,
               const AliveWriterKey& rhs) const
    {
      return (lhs.random_id < rhs.random_id);
    }
  };

  typedef map<AliveWriterKey, AliveWriterData, AliveWriterCompare> AliveWriterMap;

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

  SubscriberMgr&
  subscriber()
  { return partition().subscriber(); }

  ReaderMgr<receiverQosDataReader>&
  qos_reader()
  { return processor().receiver_qos_reader(); }

  WriterMgr<receiverQosDataWriter>&
  qos_writer()
  { return processor().receiver_qos_writer(); }

  WriterMgr<discoveryReportDataWriter>&
  discovery_report_writer()
  { return processor().discovery_report_writer(); }

  WriterMgr<receiverReportDataWriter>&
  receiver_report_writer()
  { return processor().receiver_report_writer(); }

  void
  set_topic();

  void
  set_qos();

  void
  reader_thread();

  bool
  read_throughput_message(Condition_ptr condition);

  void
  report_thread();

  Partition& m_partition;
  QueryConditionMgr m_qos_query;
  Dispatcher<Receiver> m_dispatcher;
  TopicMgr<throughput_messageTypeSupport> m_topic;
  ReaderMgr<throughput_messageDataReader> m_reader;
  StatusConditionMgr m_reader_condition;
  Attachment<Receiver> m_reader_attachment;
  Thread<Receiver> m_reader_thread;
  Thread<Receiver> m_report_thread;
  bool m_active;
  int m_config_number;
  timestamp m_creation_time;
  usec_duration m_creation_duration;
  receiverDef m_def;
  receiverQos m_qos;
  DDS::DataReaderQos m_reader_qos;
  int m_byte_count;
  int m_msgs_count;
  throughput_messageSeq m_reader_messages;
  SampleInfoSeq m_reader_infos;
  AliveWriterMap m_alive_writers;
  bool m_receiver_was_discovered;
};

#endif // RECEIVER_H
