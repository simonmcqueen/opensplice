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

public class Transmitter {
    private final Partition m_partition;
    private final QueryConditionMgr m_qos_query;
    private final TopicMgr m_topic;
    private final WriterMgr<throughput_messageDataWriter> m_writer;
    private final Thread m_writer_thread = new Thread() {
	    public void run() { writer_thread(); }
	};
    private boolean m_active = false;
    private int m_config_number = 0;
    private double m_creation_time;
    private transmitterDef m_def;
    private transmitterQos m_qos;
    private final DDS.DataWriterQosHolder m_writer_qos = new DDS.DataWriterQosHolder();

    public Transmitter(Partition partition) {
	m_partition = partition;
	// System.out.println("Transmitter::Transmitter(" + partition_id() + ")");
	m_qos_query = new QueryConditionMgr(qos_reader());
	m_topic = new TopicMgr(participant(),
			       new throughput_messageTypeSupport());
	m_writer = new WriterMgr<throughput_messageDataWriter>(publisher(),
							       m_topic,
							       throughput_messageDataWriter.class);
    }

    public void finalize() {	// useful?
	// System.out.println("Transmitter::finalize(" + partition_id() + ")");
    }

    public void	create(transmitterDef def) {
	m_def = def;
	// System.out.println("Transmitter::create(" + partition_id() + ", " + m_def.transmitter_id + ")");

	// setup QueryCondition for reading this Transmitter's Qos
	String[] params = new String[1];
	params[0] = Integer.toString(m_def.transmitter_id);
	m_qos_query.create(ANY_SAMPLE_STATE.value,
			   ANY_VIEW_STATE.value,
			   ANY_INSTANCE_STATE.value,
			   "transmitter_id = %0",
			   params);

	// setup Transmitter topic
	set_topic();

	// read initial Qos
	transmitterQosSeqHolder qoss = new transmitterQosSeqHolder();
	SampleInfoSeqHolder infos = new SampleInfoSeqHolder();
	// REVISIT - read or take?
	int retcode = qos_reader().value().read_w_condition(qoss,
							    infos,
							    1,
							    m_qos_query.value());
	if (retcode == RETCODE_NO_DATA.value) {
	    // no Qos instance to read, so initialize and write
	    m_qos = new transmitterQos();
	    m_qos.group_id = m_def.group_id;
	    m_qos.transmitter_id = m_def.transmitter_id;
	    m_qos.partition_id = m_def.partition_id;
	    m_qos.qos.latency_budget.duration.sec = 0;
	    m_qos.qos.latency_budget.duration.nanosec = 0;
	    m_qos.qos.transport_priority.value = 0;
	    retcode = qos_writer().value().write(m_qos,
						 0);
	    qos_writer().check(retcode,
			       "transmitterQosDataWriter::write");
	}
	else {
	    qos_reader().check(retcode,
			       "transmitterQosDataReader::read_w_condition");
	    assert(qoss.value.length == 1);
	    assert(infos.value.length == 1);
	    m_qos = qoss.value[0];
	    assert(m_qos.group_id == m_def.group_id);
	    assert(m_qos.transmitter_id == m_def.transmitter_id);
	    assert(m_qos.partition_id == m_def.partition_id);
	}
	qos_reader().value().return_loan(qoss,
					 infos);
	set_qos();

	// start writer thread
	m_active = true;
	m_writer_thread.start();
    }

    public void dispose() {
	// System.out.println("Transmitter::dispose(" + partition_id() + ", " + m_def.transmitter_id + ")");
	m_active = false;
	try {
	    m_writer_thread.join();
	}
	catch (Exception e) {	// REVISIT
	    System.err.println("Caught: " + e);
	}	    
	m_qos_query.destroy();
	m_topic.destroy();
    }

    public void update_def(transmitterDef def) {
	// System.out.println("Transmitter::update_def(" + partition_id() + ", " + m_def.transmitter_id + ")");
	assert(m_def.transmitter_id == def.transmitter_id);
	assert(m_active);

	// check for changes requiring thread restart
	if (m_def.scheduling_class != def.scheduling_class ||
	    m_def.thread_priority != def.thread_priority ||
	    m_def.topic_kind != def.topic_kind ||
	    m_def.topic_id != def.topic_id) {
	    // stop writer thread
	    m_active = false;
	    try {
		m_writer_thread.join();
	    }
	    catch (Exception e) {	// REVISIT
		System.err.println("Caught: " + e);
	    }	    

	    // check for topic change
	    if (m_def.topic_kind != def.topic_kind ||
		m_def.topic_id != def.topic_id) {
		m_def = def;
		set_topic();
		set_qos();
	    }
	    else {
		m_def = def;
	    }

	    // finish updating
	    m_config_number++;
      
	    // restart writer thread
	    m_active = true;
	    m_writer_thread.start();
	}
	else {
	    // update without stopping and restarting thread
	    m_def = def;
	    m_config_number++;
	}
    }

    public void update_qos(transmitterQos qos) {
	// System.out.println("Transmitter::update_qos(" + partition_id() + ", " + m_def.transmitter_id + ")");
	assert(m_def.transmitter_id == qos.transmitter_id);
	assert(m_active);
	set_qos();
	if (m_writer.value() != null) {	// REVISIT - locking
	    int retcode = m_writer.value().set_qos(m_writer_qos.value);
	    m_writer.check(retcode,
			   "throughput_messageDataWriter::set_qos");
	}
	m_config_number++;
    }

    public Partition partition() {
	return m_partition;
    }

    public int partition_id() {
	return partition().partition_id();
    }

    public Processor processor() {
	return partition().processor();
    }
  
    public ParticipantMgr participant() {
	return partition().participant();
    }

    public PublisherMgr publisher() {
	return partition().publisher();
    }

    public ReaderMgr<transmitterQosDataReader> qos_reader() {
	return processor().transmitter_qos_reader();
    }

    public WriterMgr<transmitterQosDataWriter> qos_writer() {
	return processor().transmitter_qos_writer();
    }

    private void set_topic() {
	TopicQosHolder topic_qos = new TopicQosHolder();
	int retcode = participant().value().get_default_topic_qos(topic_qos);
	participant().check(retcode,
			    "Participant::get_default_topic_qos");
	String topic_name = null;
	switch (m_def.topic_kind.value()) {
	case TopicKind._RELIABLE:
	    topic_qos.value.reliability.kind = DDS.ReliabilityQosPolicyKind.RELIABLE_RELIABILITY_QOS;
	    topic_name = "ThroughputTopic_" + m_def.topic_id + "_R";
	    break;
	case TopicKind._TRANSIENT:
	    topic_qos.value.reliability.kind = DDS.ReliabilityQosPolicyKind.RELIABLE_RELIABILITY_QOS;
	    topic_qos.value.durability.kind = DDS.DurabilityQosPolicyKind.TRANSIENT_DURABILITY_QOS;
	    topic_name = "ThroughputTopic_" + m_def.topic_id + "_T";
	    break;
	case TopicKind._PERSISTENT:
	    topic_qos.value.durability.kind = DDS.DurabilityQosPolicyKind.PERSISTENT_DURABILITY_QOS;
	    topic_name = "ThroughputTopic_" + m_def.topic_id + "_P";
	    break;
	case TopicKind._BEST_EFFORT:
	    topic_name = "ThroughputTopic_" + m_def.topic_id + "_B";
	    break;
	}
	m_topic.create(topic_name,
		       topic_qos.value);
    }

    private void set_qos() {
	int retcode = publisher().value().get_default_datawriter_qos(m_writer_qos);
	publisher().check(retcode,
			  "Publisher::get_default_datawriter_qos");
	TopicQosHolder topic_qos = new TopicQosHolder();
	retcode = m_topic.value().get_qos(topic_qos);
	m_topic.check(retcode,
		      "Topic::get_qos");
	retcode = publisher().value().copy_from_topic_qos(m_writer_qos,
							  topic_qos.value);
	publisher().check(retcode,
			  "Publisher::copy_from_topic_qos");
	m_writer_qos.value.latency_budget.duration.sec = m_qos.qos.latency_budget.duration.sec;
	m_writer_qos.value.latency_budget.duration.nanosec = m_qos.qos.latency_budget.duration.nanosec;
	m_writer_qos.value.transport_priority.value = m_qos.qos.transport_priority.value;
        /* For reliable transport, the transmitter writer
         * needs to set extra policies for its resource
         * limits, which avoids excessive memory growth
         * and invokes flow control if needed */
        if (m_writer_qos.value.reliability.kind == DDS.ReliabilityQosPolicyKind.RELIABLE_RELIABILITY_QOS) {
              m_writer_qos.value.history.kind = DDS.HistoryQosPolicyKind.KEEP_ALL_HISTORY_QOS;
              m_writer_qos.value.resource_limits.max_samples = 1;
        }

    }

    private void writer_thread() {
	// System.out.println("Transmitter::writer_thread(" + partition_id() + ", " + m_def.transmitter_id + ")");

	// REVISIT - locking?
	try {
            // Start timing of dataWriter creation now
	    m_creation_time = processor().get_timestamp();
	    // setup DataWriter
	    m_writer.create(m_writer_qos.value);

	    // setup message
	    throughput_message message = new throughput_message();
	    message.application_id = processor().application_id();
	    message.random_id = processor().random_id();
	    message.transmitter_id = m_def.transmitter_id;
	    message.creation_time = m_creation_time;
	    message.creation_duration = processor().get_timestamp() - m_creation_time;
	    message.sequence_number = 0;
	    int last_size = 0;

	    // loop
	    while (m_active) {
		// adjust message size if needed
		if (m_def.message_size != last_size) {
		    int size = 60; // REVISIT  - sizeof(throughput_message) in C or C++
		    size = (m_def.message_size > size) ? m_def.message_size - size : 1;
		    last_size = m_def.message_size;
		    message.payload_data = new char[size];
		    for (int i = 0; i < size; i++) {
			message.payload_data[i] = (char)(65+(((i/2)%26)+(i%2)*32));
		    }
		}

		// update message
		message.sequence_number++;
		message.config_number = m_config_number;
		message.write_timestamp = processor().get_timestamp();
		
		// write burst of messages
                for (int index=0; index<m_def.messages_per_burst; index++) {
                    message.instance_id = index;
		    // write message
		    int retcode = m_writer.value().write(message,
						         0);
		    m_writer.check(retcode,
			           "throughput_messageDataWriter::write");
		    // System.out.println("Transmitter::writer_thread(" + partition_id() + ", " + m_def.transmitter_id
		    //                    + ") wrote messsage with " + message.payload_data.length +" byte payload");
                }

		// sleep
		Thread.sleep(m_def.burst_period);
	    }
	}
	catch (DDSError error) {
	    processor().report_error(error,
				     partition_id(),
				     m_def.transmitter_id);
	    System.err.println("Transmitter writer thread exiting: " + error);
	    error.printStackTrace();
	}
	catch (Exception e) {	// REVISIT - report?
	    System.err.println("Transmitter writer thread exiting: " + e);
	    e.printStackTrace();
	}
	finally {
	    m_writer.destroy();
	}
	// System.out.println("Transmitter::writer_thread(" + partition_id() + ", " + m_def.transmitter_id + ") exiting");
    }
}
