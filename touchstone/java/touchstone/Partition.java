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

package touchstone;

import DDSTouchStone.*;
import ddshelp.*;
import DDS.*;
import java.util.*;

public class Partition {
    private final Processor m_processor;
    private final int m_partition_id;
    private final ParticipantMgr m_participant = new ParticipantMgr();
    private final PublisherMgr m_publisher = new PublisherMgr(m_participant);
    private final SubscriberMgr m_subscriber = new SubscriberMgr(m_participant);
    private final HashMap<Integer, Transmitter> m_transmitters = new HashMap<Integer, Transmitter>();
    private final HashMap<Integer, Receiver> m_receivers = new HashMap<Integer, Receiver>();
    private final HashMap<Integer, Transceiver> m_transceivers = new HashMap<Integer, Transceiver>();
    private final HashMap<Integer, Transponder> m_transponders = new HashMap<Integer, Transponder>();

    public Partition(Processor processor,
		     int partition_id) {
	m_processor = processor;
	m_partition_id = partition_id;
	// System.out.println("Partition::Partition(" + m_partition_id + ")");

	// REVISIT - move to init function for consistency with Actors?

	// setup DomainParticipant
	m_participant.create(m_processor.domain_id());

	// setup Publisher and Subscriber
	String id = Integer.toString(m_partition_id);
	m_publisher.create_w_partition(id);
	m_subscriber.create_w_partition(id);
    }

    public void finalize() {	// useful?
	// System.out.println("Partition::finalize(" + m_partition_id + ")");
	destroy();
    }

    public void destroy() {
	// System.out.println("Partition::destroy(" + m_partition_id + ")");
	m_subscriber.destroy();
	m_publisher.destroy();
	m_participant.destroy();
    }

    public Processor processor() {
	return m_processor;
    }

    public int partition_id() {
	return m_partition_id;
    }

    public ParticipantMgr participant() {
	return m_participant;
    }

    public PublisherMgr publisher() {
	return m_publisher;
    }

    public SubscriberMgr subscriber() {
	return m_subscriber;
    }

    public boolean empty() {
	return m_transmitters.isEmpty() && m_receivers.isEmpty() && m_transceivers.isEmpty() && m_transponders.isEmpty();
    }

    public void process_transmitter_def(transmitterDef def) {
	Transmitter transmitter = m_transmitters.get(def.transmitter_id);
	if (transmitter == null) {
	    transmitter = new Transmitter(this);
	    m_transmitters.put(def.transmitter_id,
			       transmitter);
	    transmitter.create(def);
	}
	else {
	    transmitter.update_def(def);
	}
    }

    public void dispose_transmitter_def(transmitterDef def) {
	Transmitter transmitter = m_transmitters.get(def.transmitter_id);
	if (transmitter != null) {
	    transmitter.dispose();
	    m_transmitters.remove(def.transmitter_id);
	}
    }

    public void process_transmitter_qos(transmitterQos qos) {
	Transmitter transmitter = m_transmitters.get(qos.transmitter_id);
	if (transmitter != null) {
	    transmitter.update_qos(qos);
	}
    }

    public void process_receiver_def(receiverDef def) {
	Receiver receiver = m_receivers.get(def.receiver_id);
	if (receiver == null) {
	    receiver = new Receiver(this);
	    m_receivers.put(def.receiver_id,
			    receiver);
	    receiver.create(def);
	}
	else {
	    receiver.update_def(def);
	}
    }

    public void dispose_receiver_def(receiverDef def) {
	Receiver receiver = m_receivers.get(def.receiver_id);
	if (receiver != null) {
	    receiver.dispose();
	    m_receivers.remove(def.receiver_id);
	}
    }

    public void process_receiver_qos(receiverQos qos) {
	Receiver receiver = m_receivers.get(qos.receiver_id);
	if (receiver != null) {
	    receiver.update_qos(qos);
	}
    }

    public void process_transceiver_def(transceiverDef def) {
	Transceiver transceiver = m_transceivers.get(def.transceiver_id);
	if (transceiver == null) {
	    transceiver = new Transceiver(this);
	    m_transceivers.put(def.transceiver_id,
			       transceiver);
	    transceiver.create(def);
	}
	else {
	    transceiver.update_def(def);
	}
    }

    public void dispose_transceiver_def(transceiverDef def) {
	Transceiver transceiver = m_transceivers.get(def.transceiver_id);
	if (transceiver != null) {
	    transceiver.dispose();
	    m_transceivers.remove(def.transceiver_id);
	}
    }

    public void process_transceiver_qos(transceiverQos qos) {
	Transceiver transceiver = m_transceivers.get(qos.transceiver_id);
	if (transceiver != null) {
	    transceiver.update_qos(qos);
	}
    }

    public void process_transponder_def(transponderDef def) {
	Transponder transponder = m_transponders.get(def.transponder_id);
	if (transponder == null) {
	    transponder = new Transponder(this);
	    m_transponders.put(def.transponder_id,
			       transponder);
	    transponder.create(def);
	}
	else {
	    transponder.update_def(def);
	}
    }

    public void dispose_transponder_def(transponderDef def) {
	Transponder transponder = m_transponders.get(def.transponder_id);
	if (transponder != null) {
	    transponder.dispose();
	    m_transponders.remove(def.transponder_id);
	}
    }

    public void process_transponder_qos(transponderQos qos) {
	Transponder transponder = m_transponders.get(qos.transponder_id);
	if (transponder != null) {
	    transponder.update_qos(qos);
	}
    }
}
