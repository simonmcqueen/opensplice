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

#ifndef TRANSCEIVER_H
#define TRANSCEIVER_H

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
#include <vector>

using namespace DDSTouchStone;
using namespace DDSHelp;
using namespace DDS;
using namespace std;

class Transceiver
  : public ProcessorBase
{
public:
  explicit
  Transceiver(Partition& partition);

  ~Transceiver();

  void
  create(const transceiverDef& def);

  void
  dispose();

  void
  update_def(const transceiverDef& def);

  void
  update_qos(const transceiverQos& qos);
  
private:
  typedef metricsReport Reports[4];

  class Metrics;
  friend class Transceiver::Metrics;

  class Metrics
  {
  public:
    Metrics()
    { m_stamps.reserve(10000); }

    void
    init()
    { m_stamps.clear(); }

    void
    add(double stamp)
    { m_stamps.push_back(stamp); }

    void
    analyze(Reports& reports);

  private:
    void
    report(double percentile,
	   metricsReport& report);

    vector<double> m_stamps;
  };

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

  ReaderMgr<transceiverQosDataReader>&
  qos_reader()
  { return processor().transceiver_qos_reader(); }

  WriterMgr<transceiverQosDataWriter>&
  qos_writer()
  { return processor().transceiver_qos_writer(); }

  WriterMgr<transceiverReportDataWriter>&
  transceiver_report_writer()
  { return processor().transceiver_report_writer(); }

  void
  set_topics();

  void
  set_qos();

  void
  writer_thread();

  void
  reader_thread();

  bool
  read_latency_message(Condition_ptr condition);

  void
  report_thread();

  Partition& m_partition;
  QueryConditionMgr m_qos_query;
  Dispatcher<Transceiver> m_dispatcher;
  TopicMgr<latency_messageTypeSupport> m_topic;
  WriterMgr<latency_messageDataWriter> m_writer;
  TopicMgr<latency_messageTypeSupport> m_echo_topic;
  ReaderMgr<latency_messageDataReader> m_reader;
  StatusConditionMgr m_reader_condition;
  Attachment<Transceiver> m_reader_attachment;
  Thread<Transceiver> m_writer_thread;
  Thread<Transceiver> m_reader_thread;
  Thread<Transceiver> m_report_thread;
  bool m_writer_active;
  bool m_reader_active;
  bool m_report_active;
  int m_config_number;
  transceiverDef m_def;
  transceiverQos m_qos;
  DDS::DataWriterQos m_writer_qos;
  DDS::DataReaderQos m_reader_qos;
  timestamp m_previous_time;
  Metrics m_send_latency;
  Metrics m_echo_latency;
  Metrics m_trip_latency;
  Metrics m_send_source_latency;
  Metrics m_send_arrival_latency;
  Metrics m_send_trip_latency;
  Metrics m_echo_source_latency;
  Metrics m_echo_arrival_latency;
  Metrics m_echo_trip_latency;
  Metrics m_inter_arrival_time;
  latency_messageSeq m_reader_messages;
  SampleInfoSeq m_reader_infos;
};

#endif // TRANSCEIVER_H
