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

public class Transponder {
    private final Partition m_partition;
    private final QueryConditionMgr m_qos_query;
    private final Dispatcher m_dispatcher = new Dispatcher();
    private final TopicMgr m_topic;
    private final ReaderMgr<latency_messageDataReader> m_reader;
    private final StatusConditionMgr m_reader_condition;
    private final Attachment m_reader_attachment;
    private final TopicMgr m_echo_topic;
    private final WriterMgr<latency_messageDataWriter> m_writer;
    private final Thread m_thread = new Thread() {
	    public void run() { thread(); }
	};
    private boolean m_active = false;
    private int m_config_number = 0;
    private transponderDef m_def;
    private transponderQos m_qos;
    private final DDS.DataReaderQosHolder m_reader_qos = new DDS.DataReaderQosHolder();
    private final DDS.DataWriterQosHolder m_writer_qos = new DDS.DataWriterQosHolder();
    private final latency_messageSeqHolder m_reader_messages = new latency_messageSeqHolder();
    private final SampleInfoSeqHolder m_reader_infos = new SampleInfoSeqHolder();

    public Transponder(Partition partition) {
	m_partition = partition;
	// System.out.println("Transponder::Transponder(" + partition_id() + ")");
	m_qos_query = new QueryConditionMgr(qos_reader());
	m_topic = new TopicMgr(participant(),
			       new latency_messageTypeSupport());
	m_reader = new ReaderMgr<latency_messageDataReader>(subscriber(),
							    m_topic,
							    latency_messageDataReader.class);
	m_reader_condition = new StatusConditionMgr(m_reader);
	m_reader_attachment = new Attachment(m_dispatcher,
					     m_reader_condition);
	m_echo_topic = new TopicMgr(participant(),
				    new latency_messageTypeSupport());
	m_writer = new WriterMgr<latency_messageDataWriter>(publisher(),
							    m_echo_topic,
							    latency_messageDataWriter.class);
    }

    public void finalize() {	// useful?
	// System.out.println("Transponder::finalize(" + partition_id() + ")");
    }

    public void	create(transponderDef def) {
	m_def = def;
	// System.out.println("Transponder::create(" + partition_id() + ", " + m_def.transponder_id + ")");

	// setup QueryCondition for reading this Transponder's Qos
	String[] params = new String[1];
	params[0] = Integer.toString(m_def.transponder_id);
	m_qos_query.create(ANY_SAMPLE_STATE.value,
			   ANY_VIEW_STATE.value,
			   ANY_INSTANCE_STATE.value,
			   "transponder_id = %0",
			   params);

	// setup Transponder topic
	set_topics();

	// read initial Qos
	transponderQosSeqHolder qoss = new transponderQosSeqHolder();
	SampleInfoSeqHolder infos = new SampleInfoSeqHolder();
	// REVISIT - read or take?
	int retcode = qos_reader().value().read_w_condition(qoss,
							    infos,
							    1,
							    m_qos_query.value());
	if (retcode == RETCODE_NO_DATA.value) {
	    // no Qos instance to read, so initialize and write
	    m_qos = new transponderQos();
	    m_qos.group_id = m_def.group_id;
	    m_qos.transponder_id = m_def.transponder_id;
	    m_qos.partition_id = m_def.partition_id;
	    m_qos.writer_qos.latency_budget.duration.sec = 0;
	    m_qos.writer_qos.latency_budget.duration.nanosec = 0;
	    m_qos.writer_qos.transport_priority.value = 0;
	    m_qos.reader_qos.history.depth = 1;
	    m_qos.reader_qos.latency_budget.duration.sec = 0;
	    m_qos.reader_qos.latency_budget.duration.nanosec = 0;
	    retcode = qos_writer().value().write(m_qos,
						 0);
	    qos_writer().check(retcode,
			       "transponderQosDataWriter::write");
	}
	else {
	    qos_reader().check(retcode,
			       "transponderQosDataReader::read_w_condition");
	    assert(qoss.value.length == 1);
	    assert(infos.value.length == 1);
	    m_qos = qoss.value[0];
	    assert(m_qos.group_id == m_def.group_id);
	    assert(m_qos.transponder_id == m_def.transponder_id);
	    assert(m_qos.partition_id == m_def.partition_id);
	}
	qos_reader().value().return_loan(qoss,
					 infos);
	set_qos();

	// start thread
	m_active = true;
	m_thread.start();
    }

    public void dispose() {
	// System.out.println("Transponder::dispose(" + partition_id() + ", " + m_def.transponder_id + ")");
	m_active = false;
	m_dispatcher.shutdown();
	try {
	    m_thread.join();
	}
	catch (Exception e) {	// REVISIT
	    System.err.println("Caught: " + e);
	}	    
	m_qos_query.destroy();
	m_echo_topic.destroy();
	m_topic.destroy();
    }

    public void update_def(transponderDef def) {
	// System.out.println("Transponder::update_def(" + partition_id() + ", " + m_def.transponder_id + ")");
	assert(m_def.transponder_id == def.transponder_id);
	assert(m_active);

	// check for changes requiring thread restart
	if (m_def.scheduling_class != def.scheduling_class ||
	    m_def.thread_priority != def.thread_priority ||
	    m_def.topic_kind != def.topic_kind ||
	    m_def.topic_id != def.topic_id) {
	    // stop thread
	    m_active = false;
	    m_dispatcher.shutdown();
	    try {
		m_thread.join();
	    }
	    catch (Exception e) {	// REVISIT
		System.err.println("Caught: " + e);
	    }	    

	    // check for topic change
	    if (m_def.topic_kind != def.topic_kind ||
		m_def.topic_id != def.topic_id) {
		m_def = def;
		set_topics();
		set_qos();
	    }
	    else {
		m_def = def;
	    }

	    // finish updating
	    m_config_number++;
      
	    // restart thread
	    m_active = true;
	    m_thread.start();
	}
	else {
	    // update without stopping or restarting threads
	    m_def = def;
	    m_config_number++;
	}
    }

    public void update_qos(transponderQos qos) {
	// System.out.println("Transponder::update_qos(" + partition_id() + ", " + m_def.transponder_id + ")");
	assert(m_def.transponder_id == qos.transponder_id);
	assert(m_active);
	set_qos();
	if (m_reader.value() != null) {	// REVISIT - locking
	    int retcode = m_reader.value().set_qos(m_reader_qos.value);
	    m_reader.check(retcode,
			   "latency_messageDataReader::set_qos");
	}
	if (m_writer.value() != null) {	// REVISIT - locking
	    int retcode = m_writer.value().set_qos(m_writer_qos.value);
	    m_writer.check(retcode,
			   "latency_messageDataWriter::set_qos");
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

    public SubscriberMgr subscriber() {
	return partition().subscriber();
    }

    public ReaderMgr<transponderQosDataReader> qos_reader() {
	return processor().transponder_qos_reader();
    }

    public WriterMgr<transponderQosDataWriter> qos_writer() {
	return processor().transponder_qos_writer();
    }

    private void set_topics() {
	TopicQosHolder topic_qos = new TopicQosHolder();
	int retcode = participant().value().get_default_topic_qos(topic_qos);
	participant().check(retcode,
			    "Participant::get_default_topic_qos");
	String topic_name = null;
	String echo_topic_name = null;
	switch (m_def.topic_kind.value()) {
	case TopicKind._RELIABLE:
	    topic_qos.value.reliability.kind = DDS.ReliabilityQosPolicyKind.RELIABLE_RELIABILITY_QOS;
	    topic_name = "LatencyTopic_" + m_def.topic_id + "_R";
	    echo_topic_name = "LatencyEchoTopic_" + m_def.topic_id + "_R";
	    break;
	case TopicKind._TRANSIENT:
	    topic_qos.value.reliability.kind = DDS.ReliabilityQosPolicyKind.RELIABLE_RELIABILITY_QOS;
	    topic_qos.value.durability.kind = DDS.DurabilityQosPolicyKind.TRANSIENT_DURABILITY_QOS;
	    topic_name = "LatencyTopic_" + m_def.topic_id + "_T";
	    echo_topic_name = "LatencyEchoTopic_" + m_def.topic_id + "_T";
	    break;
	case TopicKind._PERSISTENT:
	    topic_qos.value.durability.kind = DDS.DurabilityQosPolicyKind.PERSISTENT_DURABILITY_QOS;
	    topic_name = "LatencyTopic_" + m_def.topic_id + "_P";
	    echo_topic_name = "LatencyEchoTopic_" + m_def.topic_id + "_P";
	    break;
	case TopicKind._BEST_EFFORT:
	    topic_name = "LatencyTopic_" + m_def.topic_id + "_B";
	    echo_topic_name = "LatencyEchoTopic_" + m_def.topic_id + "_B";
	    break;
	}
	m_topic.create(topic_name,
		       topic_qos.value);
	m_echo_topic.create(echo_topic_name,
			    topic_qos.value);
    }

    private void set_qos() {
	// set reader Qos
	int retcode = subscriber().value().get_default_datareader_qos(m_reader_qos);
	subscriber().check(retcode,
			   "Subscriber::get_default_datareader_qos");
	TopicQosHolder topic_qos = new TopicQosHolder();
	retcode = m_topic.value().get_qos(topic_qos);
	m_topic.check(retcode,
		      "Topic::get_qos");
	retcode = subscriber().value().copy_from_topic_qos(m_reader_qos,
							   topic_qos.value);
	subscriber().check(retcode,
			   "Subscriber::copy_from_topic_qos");
	m_reader_qos.value.history.depth = m_qos.reader_qos.history.depth;
	m_reader_qos.value.latency_budget.duration.sec = m_qos.reader_qos.latency_budget.duration.sec;
	m_reader_qos.value.latency_budget.duration.nanosec = m_qos.reader_qos.latency_budget.duration.nanosec;

	// set writer Qos
	retcode = publisher().value().get_default_datawriter_qos(m_writer_qos);
	publisher().check(retcode,
			  "Publisher::get_default_datawriter_qos");
	retcode = m_echo_topic.value().get_qos(topic_qos);
	m_echo_topic.check(retcode,
			   "Topic::get_qos");
	retcode = publisher().value().copy_from_topic_qos(m_writer_qos,
							  topic_qos.value);
	publisher().check(retcode,
			  "Publisher::copy_from_topic_qos");
	m_writer_qos.value.latency_budget.duration.sec = m_qos.writer_qos.latency_budget.duration.sec;
	m_writer_qos.value.latency_budget.duration.nanosec = m_qos.writer_qos.latency_budget.duration.nanosec;
	m_writer_qos.value.transport_priority.value = m_qos.writer_qos.transport_priority.value;
    }

    private void thread() {
	// System.out.println("Transponder::thread(" + partition_id() + ", " + m_def.transponder_id + ")");
	// REVISIT - locking?
	try {
	    // setup DataReader
	    m_reader.create(m_reader_qos.value);
	    m_reader_condition.get();
	    m_reader_condition.value().set_enabled_statuses(DATA_AVAILABLE_STATUS.value);
	    m_reader_attachment.attach(new Handler() {
		    public boolean handle_condition(Condition condition) {
			return read_latency_message(condition);
		    }
		});

	    // setup DataWriter
	    m_writer.create(m_writer_qos.value);

	    // process events
	    m_dispatcher.run();
	}
	catch (DDSError error) {
	    processor().report_error(error,
				     partition_id(),
				     m_def.transponder_id);
	    System.err.println("Transponder thread exiting: " + error);
	    error.printStackTrace();
	}
	catch (Exception e) {	// REVISIT - report?
	    System.err.println("Transponder thread exiting: " + e);
	    e.printStackTrace();
	}
	finally {
	    m_reader_attachment.detach();
	    m_reader_condition.release();
	    m_reader.destroy();
	    m_writer.destroy();
	}
	// System.out.println("Transponder::thread(" + partition_id() + ", " + m_def.transponder_id + ") exiting");
    }

    public boolean read_latency_message(Condition condition) {
	// System.out.println("Transponder::read_latency_message(" + partition_id() + ", " + m_def.transponder_id + ")");
	// REVISIT - try/catch?
	int retcode = m_reader.value().take(m_reader_messages,
					    m_reader_infos,
					    1,
					    ANY_SAMPLE_STATE.value,
					    ANY_VIEW_STATE.value,
					    ANY_INSTANCE_STATE.value);
	m_reader.check(retcode,
		       "latency_messageDataReader::take");

	double read_time = processor().get_timestamp();

	int length = m_reader_messages.value.length;
	assert(length == m_reader_infos.value.length);
	for (int i = 0; i < length; i++) {
	    latency_message message = m_reader_messages.value[i];
	    SampleInfo info = m_reader_infos.value[i];
	    // System.out.println("Transponder::read_latency_message(" + partition_id() + ", " + m_def.transponder_id
	    //                    + ") read/writing message " + message.sequence_number
	    //                    + " with " + message.payload_data.length + " byte payload");

	    // generate echo message
	    double write_time = message.write_timestamp;
	    double source_time = Processor.to_timestamp(info.source_timestamp);
	    double arrival_time = read_time; // REVISIT - should be Processor::to_timestamp(info.arrival_timestamp);
	    message.send_latency = read_time - write_time;
	    message.echo_timestamp = processor().get_timestamp();
	    message.source_latency = source_time - write_time;
	    message.arrival_latency = read_time - arrival_time;
	    message.config_number = m_config_number;

	    // write echo message
	    retcode = m_writer.value().write(message,
					     0);
	    m_writer.check(retcode,
			   "latency_messageDataWriter::write");
	}

	m_reader.value().return_loan(m_reader_messages,
				     m_reader_infos);
	return true;
    }
}
