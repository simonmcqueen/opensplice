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

#ifndef PARTITION_H
#define PARTITION_H

/* ddshelp includes */
#include "ParticipantMgr.h"
#include "PublisherMgr.h"
#include "SubscriberMgr.h"
/* DDS vendor abstraction includes */
#include "touchstone_vendor_abstraction_cpp.h"
/* standard includes */
#include <map>

using namespace DDSTouchStone;
using namespace DDSHelp;
using namespace DDS;
using namespace std;

class Processor;
class Transmitter;
class Receiver;
class Transceiver;
class Transponder;

class Partition
{
public:
  Partition(Processor& processor,
	    ULong partition_id);

  ~Partition();

  Processor&
  processor()
  { return m_processor; }

  ULong
  partition_id()
  { return m_partition_id; }

  ParticipantMgr&
  participant()
  { return m_participant; }

  PublisherMgr&
  publisher()
  { return m_publisher; }

  SubscriberMgr&
  subscriber()
  { return m_subscriber; }

  bool
  empty()
  { return m_transmitters.empty() && m_receivers.empty() && m_transceivers.empty() && m_transponders.empty(); }

  void
  process_transmitter_def(const transmitterDef& def);

  void
  dispose_transmitter_def(const transmitterDef& def);

  void
  process_transmitter_qos(const transmitterQos& qos);

  void
  process_receiver_def(const receiverDef& def);

  void
  dispose_receiver_def(const receiverDef& def);

  void
  process_receiver_qos(const receiverQos& qos);

  void
  process_transceiver_def(const transceiverDef& def);

  void
  dispose_transceiver_def(const transceiverDef& def);

  void
  process_transceiver_qos(const transceiverQos& qos);

  void
  process_transponder_def(const transponderDef& def);

  void
  dispose_transponder_def(const transponderDef& def);

  void
  process_transponder_qos(const transponderQos& qos);

private:
  typedef map<ULong, Transmitter*> TransmitterMap;
  typedef map<ULong, Receiver*> ReceiverMap;
  typedef map<ULong, Transceiver*> TransceiverMap;
  typedef map<ULong, Transponder*> TransponderMap;

  Processor& m_processor;
  const ULong m_partition_id;

  ParticipantMgr m_participant;
  PublisherMgr m_publisher;
  SubscriberMgr m_subscriber;
  
  TransmitterMap m_transmitters;
  ReceiverMap m_receivers;
  TransceiverMap m_transceivers;
  TransponderMap m_transponders;
};

#endif // PARTITION_H
