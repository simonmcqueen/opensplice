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

#ifndef TRANSMITTER_H
#define TRANSMITTER_H

/* touchstone includes */
#include "Partition.h"
#include "Processor.h"
/* ddshelp includes */
#include "ConditionMgr.h"
#include "TopicMgr.h"
#include "ReaderMgr.h"
#include "WriterMgr.h"
#include "Thread.h"
/* DDS vendor abstraction includes */
#include "touchstone_vendor_abstraction_cpp.h"

using namespace DDSTouchStone;
using namespace DDSHelp;
using namespace DDS;
using namespace std;

class Transmitter
{
public:
  explicit
  Transmitter(Partition& partition);

  ~Transmitter();

  void
  create(const transmitterDef& def);

  void
  dispose();

  void
  update_def(const transmitterDef& def);

  void
  update_qos(const transmitterQos& qos);
  
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

  ReaderMgr<transmitterQosDataReader>&
  qos_reader()
  { return processor().transmitter_qos_reader(); }

  WriterMgr<transmitterQosDataWriter>&
  qos_writer()
  { return processor().transmitter_qos_writer(); }

private:
  void
  set_topic();

  void
  set_qos();

  void
  writer_thread();

  Partition& m_partition;
  QueryConditionMgr m_qos_query;
  TopicMgr<throughput_messageTypeSupport> m_topic;
  WriterMgr<throughput_messageDataWriter> m_writer;
  Thread<Transmitter> m_writer_thread;
  bool m_active;
  int m_config_number;
  timestamp m_creation_time;
  transmitterDef m_def;
  transmitterQos m_qos;
  DDS::DataWriterQos m_writer_qos;
};

#endif // TRANSMITTER_H
