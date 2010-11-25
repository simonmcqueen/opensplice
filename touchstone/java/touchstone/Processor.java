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

public class Processor {
    private final String m_application_name;
    private final int m_application_id;
    private final int m_group_id;
    private final String m_domain_id;
    /* private int m_random_id; */

    private final Time_tHolder m_time; // REVISIT - locking?

    private final Dispatcher m_dispatcher = new Dispatcher();

    // DomainParticipant
    private final ParticipantMgr m_participant = new ParticipantMgr();

    // DDSTouchStoneReports partition
    private final PublisherMgr m_report_publisher = new PublisherMgr(m_participant);

    // errorReportTopic
    private final TopicMgr m_error_report_topic = new TopicMgr(m_participant,
							       new errorReportTypeSupport());
    private final WriterMgr<errorReportDataWriter> m_error_report_writer =
	new WriterMgr<errorReportDataWriter>(m_report_publisher,
					     m_error_report_topic,
					     errorReportDataWriter.class);

    // discoveryReportTopic
    private final TopicMgr m_discovery_report_topic = new TopicMgr(m_participant,
								   new discoveryReportTypeSupport());
    private final WriterMgr<discoveryReportDataWriter> m_discovery_report_writer =
	new WriterMgr<discoveryReportDataWriter>(m_report_publisher,
						 m_discovery_report_topic,
						 discoveryReportDataWriter.class);

    // receiverReportTopic
    private final TopicMgr m_receiver_report_topic = new TopicMgr(m_participant,
								  new receiverReportTypeSupport());
    private final WriterMgr<receiverReportDataWriter> m_receiver_report_writer =
	new WriterMgr<receiverReportDataWriter>(m_report_publisher,
						m_receiver_report_topic,
						receiverReportDataWriter.class);

    // transceiverReportTopic
    private final TopicMgr m_transceiver_report_topic = new TopicMgr(m_participant,
								     new transceiverReportTypeSupport());
    private final WriterMgr<transceiverReportDataWriter> m_transceiver_report_writer =
	new WriterMgr<transceiverReportDataWriter>(m_report_publisher,
						   m_transceiver_report_topic,
						   transceiverReportDataWriter.class);

    // DDSTouchStoneCommands partition
    PublisherMgr m_command_publisher = new PublisherMgr(m_participant);
    SubscriberMgr m_command_subscriber = new SubscriberMgr(m_participant);

    // transmitterDefTopic
    private final TopicMgr m_transmitter_def_topic = new TopicMgr(m_participant,
								  new transmitterDefTypeSupport());
    private final FilteredTopicMgr m_transmitter_def_filtered_topic = new FilteredTopicMgr(m_transmitter_def_topic);
    private final ReaderMgr<transmitterDefDataReader> m_transmitter_def_reader =
	new ReaderMgr<transmitterDefDataReader>(m_command_subscriber,
						m_transmitter_def_filtered_topic,
						transmitterDefDataReader.class);
    private final ReadConditionMgr m_transmitter_def_condition = new ReadConditionMgr(m_transmitter_def_reader);;
    private final Attachment m_transmitter_def_attachment = new Attachment(m_dispatcher,
									   m_transmitter_def_condition);

    // transmitterQosTopic
    private final TopicMgr m_transmitter_qos_topic = new TopicMgr(m_participant,
								  new transmitterQosTypeSupport());
    private final WriterMgr<transmitterQosDataWriter> m_transmitter_qos_writer =
	new WriterMgr<transmitterQosDataWriter>(m_command_publisher,
						m_transmitter_qos_topic,
						transmitterQosDataWriter.class);
    private final FilteredTopicMgr m_transmitter_qos_filtered_topic = new FilteredTopicMgr(m_transmitter_qos_topic);
    private final ReaderMgr<transmitterQosDataReader> m_transmitter_qos_reader =
	new ReaderMgr<transmitterQosDataReader>(m_command_subscriber,
						m_transmitter_qos_filtered_topic,
						transmitterQosDataReader.class);
    private final ReadConditionMgr m_transmitter_qos_condition = new ReadConditionMgr(m_transmitter_qos_reader);;
    private Attachment m_transmitter_qos_attachment = new Attachment(m_dispatcher,
								     m_transmitter_qos_condition);

    // receiverDefTopic
    private final TopicMgr m_receiver_def_topic = new TopicMgr(m_participant,
							       new receiverDefTypeSupport());
    private final FilteredTopicMgr m_receiver_def_filtered_topic = new FilteredTopicMgr(m_receiver_def_topic);
    private final ReaderMgr<receiverDefDataReader> m_receiver_def_reader =
	new ReaderMgr<receiverDefDataReader>(m_command_subscriber,
					     m_receiver_def_filtered_topic,
					     receiverDefDataReader.class);
    private final ReadConditionMgr m_receiver_def_condition = new ReadConditionMgr(m_receiver_def_reader);;
    private final Attachment m_receiver_def_attachment = new Attachment(m_dispatcher,
									m_receiver_def_condition);

    // receiverQosTopic
    private final TopicMgr m_receiver_qos_topic = new TopicMgr(m_participant,
							       new receiverQosTypeSupport());
    private final WriterMgr<receiverQosDataWriter> m_receiver_qos_writer =
	new WriterMgr<receiverQosDataWriter>(m_command_publisher,
					     m_receiver_qos_topic,
					     receiverQosDataWriter.class);
    private final FilteredTopicMgr m_receiver_qos_filtered_topic = new FilteredTopicMgr(m_receiver_qos_topic);
    private final ReaderMgr<receiverQosDataReader> m_receiver_qos_reader =
	new ReaderMgr<receiverQosDataReader>(m_command_subscriber,
					     m_receiver_qos_filtered_topic,
					     receiverQosDataReader.class);
    private final ReadConditionMgr m_receiver_qos_condition = new ReadConditionMgr(m_receiver_qos_reader);;
    private final Attachment m_receiver_qos_attachment = new Attachment(m_dispatcher,
									m_receiver_qos_condition);

    // transceiverDefTopic
    private final TopicMgr m_transceiver_def_topic = new TopicMgr(m_participant,
								  new transceiverDefTypeSupport());
    private final FilteredTopicMgr m_transceiver_def_filtered_topic = new FilteredTopicMgr(m_transceiver_def_topic);
    private final ReaderMgr<transceiverDefDataReader> m_transceiver_def_reader =
	new ReaderMgr<transceiverDefDataReader>(m_command_subscriber,
						m_transceiver_def_filtered_topic,
						transceiverDefDataReader.class);
    private final ReadConditionMgr m_transceiver_def_condition = new ReadConditionMgr(m_transceiver_def_reader);;
    private final Attachment m_transceiver_def_attachment = new Attachment(m_dispatcher,
									   m_transceiver_def_condition);

    // transceiverQosTopic
    private final TopicMgr m_transceiver_qos_topic = new TopicMgr(m_participant,
								  new transceiverQosTypeSupport());
    private final WriterMgr<transceiverQosDataWriter> m_transceiver_qos_writer =
	new WriterMgr<transceiverQosDataWriter>(m_command_publisher,
						m_transceiver_qos_topic,
						transceiverQosDataWriter.class);
    private final FilteredTopicMgr m_transceiver_qos_filtered_topic = new FilteredTopicMgr(m_transceiver_qos_topic);
    private final ReaderMgr<transceiverQosDataReader> m_transceiver_qos_reader =
	new ReaderMgr<transceiverQosDataReader>(m_command_subscriber,
						m_transceiver_qos_filtered_topic,
						transceiverQosDataReader.class);
    private final ReadConditionMgr m_transceiver_qos_condition = new ReadConditionMgr(m_transceiver_qos_reader);;
    private final Attachment m_transceiver_qos_attachment = new Attachment(m_dispatcher,
									   m_transceiver_qos_condition);

    // transponderDefTopic
    private final TopicMgr m_transponder_def_topic = new TopicMgr(m_participant,
								  new transponderDefTypeSupport());
    private final FilteredTopicMgr m_transponder_def_filtered_topic = new FilteredTopicMgr(m_transponder_def_topic);
    private final ReaderMgr<transponderDefDataReader> m_transponder_def_reader =
	new ReaderMgr<transponderDefDataReader>(m_command_subscriber,
						m_transponder_def_filtered_topic,
						transponderDefDataReader.class);
    private final ReadConditionMgr m_transponder_def_condition = new ReadConditionMgr(m_transponder_def_reader);;
    private final Attachment m_transponder_def_attachment = new Attachment(m_dispatcher,
									   m_transponder_def_condition);

    // transponderQosTopic
    private final TopicMgr m_transponder_qos_topic = new TopicMgr(m_participant,
								  new transponderQosTypeSupport());
    private final WriterMgr<transponderQosDataWriter> m_transponder_qos_writer =
	new WriterMgr<transponderQosDataWriter>(m_command_publisher,
						m_transponder_qos_topic,
						transponderQosDataWriter.class);
    private final FilteredTopicMgr m_transponder_qos_filtered_topic = new FilteredTopicMgr(m_transponder_qos_topic);
    private final ReaderMgr<transponderQosDataReader> m_transponder_qos_reader =
	new ReaderMgr<transponderQosDataReader>(m_command_subscriber,
						m_transponder_qos_filtered_topic,
						transponderQosDataReader.class);
    private final ReadConditionMgr m_transponder_qos_condition = new ReadConditionMgr(m_transponder_qos_reader);;
    private final Attachment m_transponder_qos_attachment = new Attachment(m_dispatcher,
									   m_transponder_qos_condition);

    private final HashMap<Integer, Partition> m_partitions = new HashMap<Integer, Partition>();

    public Processor(String application_name,
		     int application_id,
		     int group_id,
		     String domain_id) {
	// System.out.println("Processor.Processor()");

	m_application_name = application_name;
	m_application_id = application_id;
	m_group_id = group_id;
	m_domain_id = domain_id;
	/* m_random_id = 0; */
	m_time = new Time_tHolder();
    }

    public void run() {
	// System.out.println("Processor.run()");

	try {
	    // DomainParticipant
	    m_participant.create(m_domain_id);

	    // random ID
	    /* m_participant.value().get_current_time(m_time); */
	    /* m_random_id = m_time.value.nanosec; */

	    // reliable+transient topic Qos
	    TopicQosHolder reliable_transient_topic_qos = new TopicQosHolder();
	    m_participant.value().get_default_topic_qos(reliable_transient_topic_qos);
	    reliable_transient_topic_qos.value.reliability.kind = DDS.ReliabilityQosPolicyKind.RELIABLE_RELIABILITY_QOS;
	    reliable_transient_topic_qos.value.durability.kind = DDS.DurabilityQosPolicyKind.TRANSIENT_DURABILITY_QOS;

	    // DDSTouchStoneReports partition
	    m_report_publisher.create_w_partition("DDSTouchStoneReports");

	    // report writer Qos
	    DDS.DataWriterQosHolder report_writer_qos = new DDS.DataWriterQosHolder();
	    m_report_publisher.value().get_default_datawriter_qos(report_writer_qos);
	    m_report_publisher.value().copy_from_topic_qos(report_writer_qos,
							   reliable_transient_topic_qos.value);

	    // errorReportTopic
	    m_error_report_topic.create("errorReportTopic",
					reliable_transient_topic_qos.value);
	    m_error_report_writer.create(report_writer_qos.value);

	    // From this point on, errors are reported via the errorReportTopic.

	    // discoveryReportTopic
	    m_discovery_report_topic.create("discoveryReportTopic");
	    m_discovery_report_writer.create(report_writer_qos.value);

	    // receiverReportTopic
	    m_receiver_report_topic.create("receiverReportTopic");
	    m_receiver_report_writer.create(report_writer_qos.value);

	    // transceiverReportTopic
	    m_transceiver_report_topic.create("transceiverReportTopic");
	    m_transceiver_report_writer.create(report_writer_qos.value);

	    // DDSTouchStoneCommands partition
	    m_command_publisher.create_w_partition("DDSTouchStoneCommands");
	    m_command_subscriber.create_w_partition("DDSTouchStoneCommands");
      
	    // command writer Qos
	    DDS.DataWriterQosHolder command_writer_qos = new DDS.DataWriterQosHolder();
	    m_command_publisher.value().get_default_datawriter_qos(command_writer_qos);
	    m_command_publisher.value().copy_from_topic_qos(command_writer_qos,
							    reliable_transient_topic_qos.value);

	    // command reader Qos
	    DDS.DataReaderQosHolder command_reader_qos = new DDS.DataReaderQosHolder();
	    m_command_subscriber.value().get_default_datareader_qos(command_reader_qos);
	    m_command_subscriber.value().copy_from_topic_qos(command_reader_qos,
							     reliable_transient_topic_qos.value);

	    // filter key
	    String group_id_seq[] = new String[1];
	    group_id_seq[0] = Integer.toString(m_group_id);

	    // transmitterDefTopic
	    m_transmitter_def_topic.create("transmitterDefTopic");
	    m_transmitter_def_filtered_topic.create("transmitterDefFilteredTopic",
						    "group_id = %0",
						    group_id_seq);
	    m_transmitter_def_reader.create();
	    m_transmitter_def_condition.create(NOT_READ_SAMPLE_STATE.value,
					       ANY_VIEW_STATE.value,
					       ANY_INSTANCE_STATE.value);
	    m_transmitter_def_attachment.attach(new Handler() {
		    public boolean handle_condition(Condition condition) {
			return read_transmitter_def(condition);
		    }
		});

	    // transmitterQosTopic
	    m_transmitter_qos_topic.create("transmitterQosTopic",
					   reliable_transient_topic_qos.value);
	    m_transmitter_qos_filtered_topic.create("transmitterQosFilteredTopic",
						    "group_id = %0",
						    group_id_seq);
	    m_transmitter_qos_writer.create(command_writer_qos.value);
	    m_transmitter_qos_reader.create(command_reader_qos.value);
	    m_transmitter_qos_condition.create(NOT_READ_SAMPLE_STATE.value,
					       ANY_VIEW_STATE.value,
					       ANY_INSTANCE_STATE.value);
	    m_transmitter_qos_attachment.attach(new Handler() {
		    public boolean handle_condition(Condition condition) {
			return read_transmitter_qos(condition);
		    }
		});

	    // receiverDefTopic
	    m_receiver_def_topic.create("receiverDefTopic");
	    m_receiver_def_filtered_topic.create("receiverDefFilteredTopic",
						 "group_id = %0",
						 group_id_seq);
	    m_receiver_def_reader.create();
	    m_receiver_def_condition.create(NOT_READ_SAMPLE_STATE.value,
					    ANY_VIEW_STATE.value,
					    ANY_INSTANCE_STATE.value);
	    m_receiver_def_attachment.attach(new Handler() {
		    public boolean handle_condition(Condition condition) {
			return read_receiver_def(condition);
		    }
		});

	    // receiverQosTopic
	    m_receiver_qos_topic.create("receiverQosTopic",
					reliable_transient_topic_qos.value);
	    m_receiver_qos_filtered_topic.create("receiverQosFilteredTopic",
						 "group_id = %0",
						 group_id_seq);
	    m_receiver_qos_writer.create(command_writer_qos.value);
	    m_receiver_qos_reader.create(command_reader_qos.value);
	    m_receiver_qos_condition.create(NOT_READ_SAMPLE_STATE.value,
					    ANY_VIEW_STATE.value,
					    ANY_INSTANCE_STATE.value);
	    m_receiver_qos_attachment.attach(new Handler() {
		    public boolean handle_condition(Condition condition) {
			return read_receiver_qos(condition);
		    }
		});

	    // transceiverDefTopic
	    m_transceiver_def_topic.create("transceiverDefTopic");
	    m_transceiver_def_filtered_topic.create("transceiverDefFilteredTopic",
						    "group_id = %0",
						    group_id_seq);
	    m_transceiver_def_reader.create();
	    m_transceiver_def_condition.create(NOT_READ_SAMPLE_STATE.value,
					       ANY_VIEW_STATE.value,
					       ANY_INSTANCE_STATE.value);
	    m_transceiver_def_attachment.attach(new Handler() {
		    public boolean handle_condition(Condition condition) {
			return read_transceiver_def(condition);
		    }
		});

	    // transceiverQosTopic
	    m_transceiver_qos_topic.create("transceiverQosTopic",
					   reliable_transient_topic_qos.value);
	    m_transceiver_qos_filtered_topic.create("transceiverQosFilteredTopic",
						    "group_id = %0",
						    group_id_seq);
	    m_transceiver_qos_writer.create(command_writer_qos.value);
	    m_transceiver_qos_reader.create(command_reader_qos.value);
	    m_transceiver_qos_condition.create(NOT_READ_SAMPLE_STATE.value,
					       ANY_VIEW_STATE.value,
					       ANY_INSTANCE_STATE.value);
	    m_transceiver_qos_attachment.attach(new Handler() {
		    public boolean handle_condition(Condition condition) {
			return read_transceiver_qos(condition);
		    }
		});

	    // transponderDefTopic
	    m_transponder_def_topic.create("transponderDefTopic");
	    m_transponder_def_filtered_topic.create("transponderDefFilteredTopic",
						    "group_id = %0",
						    group_id_seq);
	    m_transponder_def_reader.create();
	    m_transponder_def_condition.create(NOT_READ_SAMPLE_STATE.value,
					       ANY_VIEW_STATE.value,
					       ANY_INSTANCE_STATE.value);
	    m_transponder_def_attachment.attach(new Handler() {
		    public boolean handle_condition(Condition condition) {
			return read_transponder_def(condition);
		    }
		});

	    // transponderQosTopic
	    m_transponder_qos_topic.create("transponderQosTopic",
					   reliable_transient_topic_qos.value);
	    m_transponder_qos_filtered_topic.create("transponderQosFilteredTopic",
						    "group_id = %0",
						    group_id_seq);
	    m_transponder_qos_writer.create(command_writer_qos.value);
	    m_transponder_qos_reader.create(command_reader_qos.value);
	    m_transponder_qos_condition.create(NOT_READ_SAMPLE_STATE.value,
					       ANY_VIEW_STATE.value,
					       ANY_INSTANCE_STATE.value);
	    m_transponder_qos_attachment.attach(new Handler() {
		    public boolean handle_condition(Condition condition) {
			return read_transponder_qos(condition);
		    }
		});

	    // process commands
	    m_dispatcher.run();
	}
	catch (DDSError error) {
	    report_error(error, 0, 0);
	    System.err.println("Exiting with exception: ");
	    error.printStackTrace();
	}
	finally {
	    // System.out.println("Processor.run() - finally");

	    // transponderQosTopic
	    m_transponder_qos_attachment.detach();
	    m_transponder_qos_condition.destroy();
	    m_transponder_qos_reader.destroy();
	    m_transponder_qos_filtered_topic.destroy();
	    m_transponder_qos_writer.destroy();
	    m_transponder_qos_topic.destroy();

	    // transponderDefTopic
	    m_transponder_def_attachment.detach();
	    m_transponder_def_condition.destroy();
	    m_transponder_def_reader.destroy();
	    m_transponder_def_filtered_topic.destroy();
	    m_transponder_def_topic.destroy();

	    // transceiverQosTopic
	    m_transceiver_qos_attachment.detach();
	    m_transceiver_qos_condition.destroy();
	    m_transceiver_qos_reader.destroy();
	    m_transceiver_qos_filtered_topic.destroy();
	    m_transceiver_qos_writer.destroy();
	    m_transceiver_qos_topic.destroy();

	    // transceiverDefTopic
	    m_transceiver_def_attachment.detach();
	    m_transceiver_def_condition.destroy();
	    m_transceiver_def_reader.destroy();
	    m_transceiver_def_filtered_topic.destroy();
	    m_transceiver_def_topic.destroy();

	    // receiverQosTopic
	    m_receiver_qos_attachment.detach();
	    m_receiver_qos_condition.destroy();
	    m_receiver_qos_reader.destroy();
	    m_receiver_qos_filtered_topic.destroy();
	    m_receiver_qos_writer.destroy();
	    m_receiver_qos_topic.destroy();

	    // receiverDefTopic
	    m_receiver_def_attachment.detach();
	    m_receiver_def_condition.destroy();
	    m_receiver_def_reader.destroy();
	    m_receiver_def_filtered_topic.destroy();
	    m_receiver_def_topic.destroy();

	    // transmitterQosTopic
	    m_transmitter_qos_attachment.detach();
	    m_transmitter_qos_condition.destroy();
	    m_transmitter_qos_reader.destroy();
	    m_transmitter_qos_filtered_topic.destroy();
	    m_transmitter_qos_writer.destroy();
	    m_transmitter_qos_topic.destroy();

	    // transmitterDefTopic
	    m_transmitter_def_attachment.detach();
	    m_transmitter_def_condition.destroy();
	    m_transmitter_def_reader.destroy();
	    m_transmitter_def_filtered_topic.destroy();
	    m_transmitter_def_topic.destroy();

	    // DDSTouchStoneCommands partition
	    m_command_subscriber.destroy();
	    m_command_publisher.destroy();

	    // transceiverReportTopic
	    m_transceiver_report_writer.destroy();
	    m_transceiver_report_topic.destroy();

	    // receiverReportTopic
	    m_receiver_report_writer.destroy();
	    m_receiver_report_topic.destroy();

	    // discoveryReportTopic
	    m_discovery_report_writer.destroy();
	    m_discovery_report_topic.destroy();

	    // errorReportTopic
	    m_error_report_writer.destroy();
	    m_error_report_topic.destroy();

	    // DDSTouchStoneReports partition
	    m_report_publisher.destroy();

	    // DomainParticipant
	    m_participant.destroy();
	}
    }

    public void shutdown() {
	// System.out.println("Processor.shutdown()");
	m_dispatcher.shutdown();
    }

    public void report_error(DDSError error,
			     int partition_id,
			     int entity_id) {
	if (m_error_report_writer.value() != null) {
	    errorReport report = new errorReport();
	    report.application_id = m_application_id;
	    report.partition_id = partition_id;
	    report.entity_id = entity_id;
	    report.message = error.getMessage();
	    int retcode = m_error_report_writer.value().write(report,
							      0);

	    // REVISIT - do we really want to report errors reporting errors?
	    // m_error_report_writer.check(retcode,
	    //                             "errorReportDataWriter::write");
	}
    }

    public String application_name() {
	return m_application_name;
    }

    public int application_id() {
	return m_application_id;
    }

    public int group_id() {
	return m_group_id;
    }

    public String domain_id() {
	return m_domain_id;
    }

    public int random_id() {
	/* return m_random_id; */
        m_participant.value().get_current_time(m_time);
	return m_time.value.nanosec;
    }

    public double get_timestamp() {
	assert(m_participant != null);
	m_participant.value().get_current_time(m_time);
	return to_timestamp(m_time.value);
    }

    public static double to_timestamp(Time_t time) {
	return time.sec * 1000000 + time.nanosec / 1000;
    }

    public WriterMgr<discoveryReportDataWriter> discovery_report_writer() {
	return m_discovery_report_writer;
    }

    public WriterMgr<receiverReportDataWriter> receiver_report_writer() {
	return m_receiver_report_writer;
    }

    public WriterMgr<transceiverReportDataWriter> transceiver_report_writer() {
	return m_transceiver_report_writer;
    }

    public WriterMgr<transmitterQosDataWriter> transmitter_qos_writer() {
	return m_transmitter_qos_writer;
    }

    public ReaderMgr<transmitterQosDataReader> transmitter_qos_reader() {
	return m_transmitter_qos_reader;
    }

    public WriterMgr<receiverQosDataWriter> receiver_qos_writer() {
	return m_receiver_qos_writer;
    }

    public ReaderMgr<receiverQosDataReader> receiver_qos_reader() {
	return m_receiver_qos_reader;
    }

    public WriterMgr<transceiverQosDataWriter> transceiver_qos_writer() {
	return m_transceiver_qos_writer;
    }

    public ReaderMgr<transceiverQosDataReader> transceiver_qos_reader() {
	return m_transceiver_qos_reader;
    }

    public WriterMgr<transponderQosDataWriter> transponder_qos_writer() {
	return m_transponder_qos_writer;
    }

    public ReaderMgr<transponderQosDataReader> transponder_qos_reader() {
	return m_transponder_qos_reader;
    }

    public boolean read_transmitter_def(Condition condition) {
	// System.out.println("Processor::read_transmitter_def()");
	transmitterDefSeqHolder defs = new transmitterDefSeqHolder(); // REVISIT - use class field
	SampleInfoSeqHolder infos = new SampleInfoSeqHolder(); // REVISIT - use class field
	try {
	    int retcode = m_transmitter_def_reader.value().take(defs,
								infos,
								1,
								ANY_SAMPLE_STATE.value,
								ANY_VIEW_STATE.value,
								ANY_INSTANCE_STATE.value);
	    m_transmitter_def_reader.check(retcode,
					   "transmitterDefDataReader::take");
	    assert(defs.value.length == infos.value.length);
	    for (int i = 0; i < defs.value.length; i++) {
		transmitterDef def = defs.value[i];
		SampleInfo info = infos.value[i];
		int partition_id = def.partition_id;
		Partition partition = m_partitions.get(partition_id);
		if (partition == null) {
		    partition = new Partition(this,
					      partition_id);
		    m_partitions.put(partition_id,
				     partition);
		}
		if (info.instance_state == NOT_ALIVE_DISPOSED_INSTANCE_STATE.value ||
		    !info.valid_data) {
		    partition.dispose_transmitter_def(def);
		    if (partition.empty()) {
			m_partitions.remove(partition_id);
			partition.destroy();
		    }
		}
		else {
		    partition.process_transmitter_def(def);
		}
	    }
	}
	catch (DDSError error) {
	    report_error(error,
			 0,
			 0);
	    System.err.println("caught: " + error);
	    error.printStackTrace();
	}
	m_transmitter_def_reader.value().return_loan(defs,
						     infos);
	return true;
    }

    public boolean read_transmitter_qos(Condition condition) {
	// System.out.println("Processor::read_transmitter_qos()");
	transmitterQosSeqHolder qoss = new transmitterQosSeqHolder(); // REVISIT - use class field
	SampleInfoSeqHolder infos = new SampleInfoSeqHolder(); // REVISIT - use class field
	try {
	    int retcode = m_transmitter_qos_reader.value().read_w_condition(qoss,
									    infos,
									    1,
									    m_transmitter_qos_condition.value());
	    m_transmitter_qos_reader.check(retcode,
					   "transmitterQosDataReader::read_w_condition");
	    assert(qoss.value.length == infos.value.length);
	    for (int i = 0; i < qoss.value.length; i++) {
		transmitterQos qos = qoss.value[i];
		SampleInfo info = infos.value[i];
		int partition_id = qos.partition_id;
		Partition partition = m_partitions.get(partition_id);
		if (partition != null) {
		    partition.process_transmitter_qos(qos);
		}
	    }
	}
	catch (DDSError error) {
	    report_error(error,
			 0,
			 0);
	    System.err.println("caught: " + error);
	    error.printStackTrace();
	}
	m_transmitter_qos_reader.value().return_loan(qoss,
						     infos);
	return true;
    }

    public boolean read_receiver_def(Condition condition) {
	// System.out.println("Processor::read_receiver_def()");
	receiverDefSeqHolder defs = new receiverDefSeqHolder(); // REVISIT - use class field
	SampleInfoSeqHolder infos = new SampleInfoSeqHolder(); // REVISIT - use class field
	try {
	    int retcode = m_receiver_def_reader.value().take(defs,
							     infos,
							     1,
							     ANY_SAMPLE_STATE.value,
							     ANY_VIEW_STATE.value,
							     ANY_INSTANCE_STATE.value);
	    m_receiver_def_reader.check(retcode,
					"receiverDefDataReader::take");
	    assert(defs.value.length == infos.value.length);
	    for (int i = 0; i < defs.value.length; i++) {
		receiverDef def = defs.value[i];
		SampleInfo info = infos.value[i];
		int partition_id = def.partition_id;
		Partition partition = m_partitions.get(partition_id);
		if (partition == null) {
		    partition = new Partition(this,
					      partition_id);
		    m_partitions.put(partition_id,
				     partition);
		}
		if (info.instance_state == NOT_ALIVE_DISPOSED_INSTANCE_STATE.value ||
		    !info.valid_data) {
		    partition.dispose_receiver_def(def);
		    if (partition.empty()) {
			m_partitions.remove(partition_id);
			partition.destroy();
		    }
		}
		else {
		    partition.process_receiver_def(def);
		}
	    }
	}
	catch (DDSError error) {
	    report_error(error,
			 0,
			 0);
	    System.err.println("caught: " + error);
	    error.printStackTrace();
	}
	m_receiver_def_reader.value().return_loan(defs,
						  infos);
	return true;
    }

    public boolean read_receiver_qos(Condition condition) {
	// System.out.println("Processor::read_receiver_qos()");
	receiverQosSeqHolder qoss = new receiverQosSeqHolder(); // REVISIT - use class field
	SampleInfoSeqHolder infos = new SampleInfoSeqHolder(); // REVISIT - use class field
	try {
	    int retcode = m_receiver_qos_reader.value().read_w_condition(qoss,
									 infos,
									 1,
									 m_receiver_qos_condition.value());
	    m_receiver_qos_reader.check(retcode,
					"receiverQosDataReader::read_w_condition");
	    assert(qoss.value.length == infos.value.length);
	    for (int i = 0; i < qoss.value.length; i++) {
		receiverQos qos = qoss.value[i];
		SampleInfo info = infos.value[i];
		int partition_id = qos.partition_id;
		Partition partition = m_partitions.get(partition_id);
		if (partition != null) {
		    partition.process_receiver_qos(qos);
		}
	    }
	}
	catch (DDSError error) {
	    report_error(error,
			 0,
			 0);
	    System.err.println("caught: " + error);
	    error.printStackTrace();
	}
	m_receiver_qos_reader.value().return_loan(qoss,
						  infos);
	return true;
    }

    public boolean read_transceiver_def(Condition condition) {
	// System.out.println("Processor::read_transceiver_def()");
	transceiverDefSeqHolder defs = new transceiverDefSeqHolder(); // REVISIT - use class field
	SampleInfoSeqHolder infos = new SampleInfoSeqHolder(); // REVISIT - use class field
	try {
	    int retcode = m_transceiver_def_reader.value().take(defs,
								infos,
								1,
								ANY_SAMPLE_STATE.value,
								ANY_VIEW_STATE.value,
								ANY_INSTANCE_STATE.value);
	    m_transceiver_def_reader.check(retcode,
					   "transceiverDefDataReader::take");
	    assert(defs.value.length == infos.value.length);
	    for (int i = 0; i < defs.value.length; i++) {
		transceiverDef def = defs.value[i];
		SampleInfo info = infos.value[i];
		int partition_id = def.partition_id;
		Partition partition = m_partitions.get(partition_id);
		if (partition == null) {
		    partition = new Partition(this,
					      partition_id);
		    m_partitions.put(partition_id,
				     partition);
		}
		if (info.instance_state == NOT_ALIVE_DISPOSED_INSTANCE_STATE.value ||
		    !info.valid_data) {
		    partition.dispose_transceiver_def(def);
		    if (partition.empty()) {
			m_partitions.remove(partition_id);
			partition.destroy();
		    }
		}
		else {
		    partition.process_transceiver_def(def);
		}
	    }
	}
	catch (DDSError error) {
	    report_error(error,
			 0,
			 0);
	    System.err.println("caught: " + error);
	    error.printStackTrace();
	}
	m_transceiver_def_reader.value().return_loan(defs,
						     infos);
	return true;
    }

    public boolean read_transceiver_qos(Condition condition) {
	// System.out.println("Processor::read_transceiver_qos()");
	transceiverQosSeqHolder qoss = new transceiverQosSeqHolder(); // REVISIT - use class field
	SampleInfoSeqHolder infos = new SampleInfoSeqHolder(); // REVISIT - use class field
	try {
	    int retcode = m_transceiver_qos_reader.value().read_w_condition(qoss,
									    infos,
									    1,
									    m_transceiver_qos_condition.value());
	    m_transceiver_qos_reader.check(retcode,
					   "transceiverQosDataReader::read_w_condition");
	    assert(qoss.value.length == infos.value.length);
	    for (int i = 0; i < qoss.value.length; i++) {
		transceiverQos qos = qoss.value[i];
		SampleInfo info = infos.value[i];
		int partition_id = qos.partition_id;
		Partition partition = m_partitions.get(partition_id);
		if (partition != null) {
		    partition.process_transceiver_qos(qos);
		}
	    }
	}
	catch (DDSError error) {
	    report_error(error,
			 0,
			 0);
	    System.err.println("caught: " + error);
	    error.printStackTrace();
	}
	m_transceiver_qos_reader.value().return_loan(qoss,
						     infos);
	return true;
    }

    public boolean read_transponder_def(Condition condition) {
	// System.out.println("Processor::read_transponder_def()");
	transponderDefSeqHolder defs = new transponderDefSeqHolder(); // REVISIT - use class field
	SampleInfoSeqHolder infos = new SampleInfoSeqHolder(); // REVISIT - use class field
	try {
	    int retcode = m_transponder_def_reader.value().take(defs,
								infos,
								1,
								ANY_SAMPLE_STATE.value,
								ANY_VIEW_STATE.value,
								ANY_INSTANCE_STATE.value);
	    m_transponder_def_reader.check(retcode,
					   "transponderDefDataReader::take");
	    assert(defs.value.length == infos.value.length);
	    for (int i = 0; i < defs.value.length; i++) {
		transponderDef def = defs.value[i];
		SampleInfo info = infos.value[i];
		int partition_id = def.partition_id;
		Partition partition = m_partitions.get(partition_id);
		if (partition == null) {
		    partition = new Partition(this,
					      partition_id);
		    m_partitions.put(partition_id,
				     partition);
		}
		if (info.instance_state == NOT_ALIVE_DISPOSED_INSTANCE_STATE.value ||
		    !info.valid_data) {
		    partition.dispose_transponder_def(def);
		    if (partition.empty()) {
			m_partitions.remove(partition_id);
			partition.destroy();
		    }
		}
		else {
		    partition.process_transponder_def(def);
		}
	    }
	}
	catch (DDSError error) {
	    report_error(error,
			 0,
			 0);
	    System.err.println("caught: " + error);
	    error.printStackTrace();
	}
	m_transponder_def_reader.value().return_loan(defs,
						     infos);
	return true;
    }

    public boolean read_transponder_qos(Condition condition) {
	// System.out.println("Processor::read_transponder_qos()");
	transponderQosSeqHolder qoss = new transponderQosSeqHolder(); // REVISIT - use class field
	SampleInfoSeqHolder infos = new SampleInfoSeqHolder(); // REVISIT - use class field
	try {
	    int retcode = m_transponder_qos_reader.value().read_w_condition(qoss,
									    infos,
									    1,
									    m_transponder_qos_condition.value());
	    m_transponder_qos_reader.check(retcode,
					   "transponderQosDataReader::read_w_condition");
	    assert(qoss.value.length == infos.value.length);
	    for (int i = 0; i < qoss.value.length; i++) {
		transponderQos qos = qoss.value[i];
		SampleInfo info = infos.value[i];
		int partition_id = qos.partition_id;
		Partition partition = m_partitions.get(partition_id);
		if (partition != null) {
		    partition.process_transponder_qos(qos);
		}
	    }
	}
	catch (DDSError error) {
	    report_error(error,
			 0,
			 0);
	    System.err.println("caught: " + error);
	    error.printStackTrace();
	}
	m_transponder_qos_reader.value().return_loan(qoss,
						     infos);
	return true;
    }
}
