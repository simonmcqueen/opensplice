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

#ifndef PROCESSOR_H
#define PROCESSOR_H

/* ddshelp includes */
#include "Dispatcher.h"
#include "ParticipantMgr.h"
#include "PublisherMgr.h"
#include "SubscriberMgr.h"
#include "TopicMgr.h"
#include "WriterMgr.h"
#include "ReaderMgr.h"
#include "DDSError.h"
/* DDS vendor abstraction includes */
#include "touchstone_vendor_abstraction_cpp.h"
/* standard includes */
#include <map>

using namespace DDSTouchStone;
using namespace DDSHelp;
using namespace DDS;
using namespace std;

class Partition;

class Processor
  : public ProcessorBase
{
public:
  Processor(const char* application_name,
	    ULong application_id,
	    ULong group_id,
	    const DomainId_t& domain_id);

  ~Processor();

  void
  run();

  void
  shutdown()
  { m_dispatcher.shutdown(); }

  void
  report_error(const DDSError& error,
	       ULong partition_id = 0,
	       ULong entity_id = 0);

  const char*
  application_name()
  { return m_application_name; }

  ULong
  application_id()
  { return m_application_id; }

  ULong
  group_id()
  { return m_group_id; }

  const DomainId_t&
  domain_id()
  { return m_domain_id; }

  ULong
  random_id();
  /* { return m_random_id; } */

  timestamp
  get_timestamp();

  static timestamp
  to_timestamp(const Time_t& time)
  { return time.sec * 1000000 + time.nanosec / 1000; }

  static void
  msec_sleep(msec_t milliseconds);

  WriterMgr<discoveryReportDataWriter>&
  discovery_report_writer()
  { return m_discovery_report_writer; }

  WriterMgr<receiverReportDataWriter>&
  receiver_report_writer()
  { return m_receiver_report_writer; }

  WriterMgr<transceiverReportDataWriter>&
  transceiver_report_writer()
  { return m_transceiver_report_writer; }

  WriterMgr<transmitterQosDataWriter>&
  transmitter_qos_writer()
  { return m_transmitter_qos_writer; }

  ReaderMgr<transmitterQosDataReader>&
  transmitter_qos_reader()
  { return m_transmitter_qos_reader; }

  WriterMgr<receiverQosDataWriter>&
  receiver_qos_writer()
  { return m_receiver_qos_writer; }

  ReaderMgr<receiverQosDataReader>&
  receiver_qos_reader()
  { return m_receiver_qos_reader; }

  WriterMgr<transceiverQosDataWriter>&
  transceiver_qos_writer()
  { return m_transceiver_qos_writer; }

  ReaderMgr<transceiverQosDataReader>&
  transceiver_qos_reader()
  { return m_transceiver_qos_reader; }

  WriterMgr<transponderQosDataWriter>&
  transponder_qos_writer()
  { return m_transponder_qos_writer; }

  ReaderMgr<transponderQosDataReader>&
  transponder_qos_reader()
  { return m_transponder_qos_reader; }

private:
  typedef map<ULong, Partition*> PartitionMap;

  bool
  read_transmitter_def(Condition_ptr condition);

  bool
  read_transmitter_qos(Condition_ptr condition);

  bool
  read_receiver_def(Condition_ptr condition);

  bool
  read_receiver_qos(Condition_ptr condition);

  bool
  read_transceiver_def(Condition_ptr condition);

  bool
  read_transceiver_qos(Condition_ptr condition);

  bool
  read_transponder_def(Condition_ptr condition);

  bool
  read_transponder_qos(Condition_ptr condition);

  String_var m_application_name;
  ULong m_application_id;
  ULong m_group_id;
  DomainId_t m_domain_id;
  /* ULong m_random_id; */

  Dispatcher<Processor> m_dispatcher;
  ParticipantMgr m_participant;

  // DDSTouchStoneReports partition
  PublisherMgr m_report_publisher;

  // errorReportTopic
  TopicMgr<errorReportTypeSupport> m_error_report_topic;
  WriterMgr<errorReportDataWriter> m_error_report_writer;

  // discoveryReportTopic
  TopicMgr<discoveryReportTypeSupport> m_discovery_report_topic;
  WriterMgr<discoveryReportDataWriter> m_discovery_report_writer;

  // receiverReportTopic
  TopicMgr<receiverReportTypeSupport> m_receiver_report_topic;
  WriterMgr<receiverReportDataWriter> m_receiver_report_writer;

  // transceiverReportTopic
  TopicMgr<transceiverReportTypeSupport> m_transceiver_report_topic;
  WriterMgr<transceiverReportDataWriter> m_transceiver_report_writer;

  // DDSTouchStoneCommands partition
  PublisherMgr m_command_publisher;
  SubscriberMgr m_command_subscriber;

  // transmitterDefTopic
  TopicMgr<transmitterDefTypeSupport> m_transmitter_def_topic;
  FilteredTopicMgr m_transmitter_def_filtered_topic;
  ReaderMgr<transmitterDefDataReader> m_transmitter_def_reader;
  ReadConditionMgr m_transmitter_def_condition;
  Attachment<Processor> m_transmitter_def_attachment;

  // transmitterQosTopic
  TopicMgr<transmitterQosTypeSupport> m_transmitter_qos_topic;
  WriterMgr<transmitterQosDataWriter> m_transmitter_qos_writer;
  FilteredTopicMgr m_transmitter_qos_filtered_topic;
  ReaderMgr<transmitterQosDataReader> m_transmitter_qos_reader;
  ReadConditionMgr m_transmitter_qos_condition;
  Attachment<Processor> m_transmitter_qos_attachment;

  // receiverDefTopic
  TopicMgr<receiverDefTypeSupport> m_receiver_def_topic;
  FilteredTopicMgr m_receiver_def_filtered_topic;
  ReaderMgr<receiverDefDataReader> m_receiver_def_reader;
  ReadConditionMgr m_receiver_def_condition;
  Attachment<Processor> m_receiver_def_attachment;

  // receiverQosTopic
  TopicMgr<receiverQosTypeSupport> m_receiver_qos_topic;
  WriterMgr<receiverQosDataWriter> m_receiver_qos_writer;
  FilteredTopicMgr m_receiver_qos_filtered_topic;
  ReaderMgr<receiverQosDataReader> m_receiver_qos_reader;
  ReadConditionMgr m_receiver_qos_condition;
  Attachment<Processor> m_receiver_qos_attachment;

  // transceiverDefTopic
  TopicMgr<transceiverDefTypeSupport> m_transceiver_def_topic;
  FilteredTopicMgr m_transceiver_def_filtered_topic;
  ReaderMgr<transceiverDefDataReader> m_transceiver_def_reader;
  ReadConditionMgr m_transceiver_def_condition;
  Attachment<Processor> m_transceiver_def_attachment;

  // transceiverQosTopic
  TopicMgr<transceiverQosTypeSupport> m_transceiver_qos_topic;
  WriterMgr<transceiverQosDataWriter> m_transceiver_qos_writer;
  FilteredTopicMgr m_transceiver_qos_filtered_topic;
  ReaderMgr<transceiverQosDataReader> m_transceiver_qos_reader;
  ReadConditionMgr m_transceiver_qos_condition;
  Attachment<Processor> m_transceiver_qos_attachment;

  // transponderDefTopic
  TopicMgr<transponderDefTypeSupport> m_transponder_def_topic;
  FilteredTopicMgr m_transponder_def_filtered_topic;
  ReaderMgr<transponderDefDataReader> m_transponder_def_reader;
  ReadConditionMgr m_transponder_def_condition;
  Attachment<Processor> m_transponder_def_attachment;

  // transponderQosTopic
  TopicMgr<transponderQosTypeSupport> m_transponder_qos_topic;
  WriterMgr<transponderQosDataWriter> m_transponder_qos_writer;
  FilteredTopicMgr m_transponder_qos_filtered_topic;
  ReaderMgr<transponderQosDataReader> m_transponder_qos_reader;
  ReadConditionMgr m_transponder_qos_condition;
  Attachment<Processor> m_transponder_qos_attachment;

  PartitionMap m_partitions;  
};

#endif // PROCESSOR_H
