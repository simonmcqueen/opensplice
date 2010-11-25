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

/* touchstone includes */
#include "Partition.h"
#include "Processor.h"
#include "Transmitter.h"
#include "Receiver.h"
#include "Transceiver.h"
#include "Transponder.h"
/* ddshelp includes */
#include "DDSError.h"
/* standard includes */
#include <iostream>
#include <sstream>

Partition::Partition(Processor& processor,
		     ULong partition_id)
  : m_processor(processor),
    m_partition_id(partition_id),
    m_participant(),
    m_publisher(m_participant),
    m_subscriber(m_participant)
{
  // cout << "Partition::Partition(" << m_partition_id << ")" << endl;

  // REVISIT - move to init function for consistency with Actors?

  // setup DomainParticipant
  m_participant.create(m_processor.domain_id());

  // setup Publisher and Subscriber
  stringstream id;
  id << m_partition_id;
  m_publisher.create_w_partition(id.str().c_str());
  m_subscriber.create_w_partition(id.str().c_str());
}

Partition::~Partition()
{
  // cout << "Partition::~Partition(" << m_partition_id << ")" << endl;
}

void
Partition::process_transmitter_def(const transmitterDef& def)
{
  TransmitterMap::iterator iter(m_transmitters.find(def.transmitter_id));
  if (iter == m_transmitters.end())
    {
      Transmitter* transmitter = new Transmitter(*this);
      m_transmitters.insert(make_pair(def.transmitter_id,
				      transmitter));
      transmitter->create(def);
    }
  else
    {
      Transmitter* transmitter = (*iter).second;
      assert(transmitter);
      transmitter->update_def(def);
    }
}

void
Partition::dispose_transmitter_def(const transmitterDef& def)
{
  TransmitterMap::iterator iter(m_transmitters.find(def.transmitter_id));
  if (iter != m_transmitters.end())
    {
      Transmitter* transmitter = (*iter).second;
      assert(transmitter);
      transmitter->dispose();
      delete transmitter;
      m_transmitters.erase(def.transmitter_id);
    }
}

void
Partition::process_transmitter_qos(const transmitterQos& qos)
{
  TransmitterMap::iterator iter(m_transmitters.find(qos.transmitter_id));
  if (iter != m_transmitters.end())
    {
      Transmitter* transmitter = (*iter).second;
      assert(transmitter);
      transmitter->update_qos(qos);
    }
}

void
Partition::process_receiver_def(const receiverDef& def)
{
  ReceiverMap::iterator iter(m_receivers.find(def.receiver_id));
  if (iter == m_receivers.end())
    {
      Receiver* receiver = new Receiver(*this);
      m_receivers.insert(make_pair(def.receiver_id,
				   receiver));
      receiver->create(def);
    }
  else
    {
      Receiver* receiver = (*iter).second;
      assert(receiver);
      receiver->update_def(def);
    }
}

void
Partition::dispose_receiver_def(const receiverDef& def)
{
  ReceiverMap::iterator iter(m_receivers.find(def.receiver_id));
  if (iter != m_receivers.end())
    {
      Receiver* receiver = (*iter).second;
      assert(receiver);
      receiver->dispose();
      delete receiver;
      m_receivers.erase(def.receiver_id);
    }
}

void
Partition::process_receiver_qos(const receiverQos& qos)
{
  ReceiverMap::iterator iter(m_receivers.find(qos.receiver_id));
  if (iter != m_receivers.end())
    {
      Receiver* receiver = (*iter).second;
      assert(receiver);
      receiver->update_qos(qos);
    }
}

void
Partition::process_transceiver_def(const transceiverDef& def)
{
  TransceiverMap::iterator iter(m_transceivers.find(def.transceiver_id));
  if (iter == m_transceivers.end())
    {
      Transceiver* transceiver = new Transceiver(*this);
      m_transceivers.insert(make_pair(def.transceiver_id,
				      transceiver));
      transceiver->create(def);
    }
  else
    {
      Transceiver* transceiver = (*iter).second;
      assert(transceiver);
      transceiver->update_def(def);
    }
}

void
Partition::dispose_transceiver_def(const transceiverDef& def)
{
  TransceiverMap::iterator iter(m_transceivers.find(def.transceiver_id));
  if (iter != m_transceivers.end())
    {
      Transceiver* transceiver = (*iter).second;
      assert(transceiver);
      transceiver->dispose();
      delete transceiver;
      m_transceivers.erase(def.transceiver_id);
    }
}

void
Partition::process_transceiver_qos(const transceiverQos& qos)
{
  TransceiverMap::iterator iter(m_transceivers.find(qos.transceiver_id));
  if (iter != m_transceivers.end())
    {
      Transceiver* transceiver = (*iter).second;
      assert(transceiver);
      transceiver->update_qos(qos);
    }
}

void
Partition::process_transponder_def(const transponderDef& def)
{
  TransponderMap::iterator iter(m_transponders.find(def.transponder_id));
  if (iter == m_transponders.end())
    {
      Transponder* transponder = new Transponder(*this);
      m_transponders.insert(make_pair(def.transponder_id,
				      transponder));
      transponder->create(def);
    }
  else
    {
      Transponder* transponder = (*iter).second;
      assert(transponder);
      transponder->update_def(def);
    }
}

void
Partition::dispose_transponder_def(const transponderDef& def)
{
  TransponderMap::iterator iter(m_transponders.find(def.transponder_id));
  if (iter != m_transponders.end())
    {
      Transponder* transponder = (*iter).second;
      assert(transponder);
      transponder->dispose();
      delete transponder;
      m_transponders.erase(def.transponder_id);
    }
}

void
Partition::process_transponder_qos(const transponderQos& qos)
{
  TransponderMap::iterator iter(m_transponders.find(qos.transponder_id));
  if (iter != m_transponders.end())
    {
      Transponder* transponder = (*iter).second;
      assert(transponder);
      transponder->update_qos(qos);
    }
}
