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

public class Receiver {

    private static class AliveWriterKey {
      public int random_id;

      public AliveWriterKey() {
          random_id = 0;
      }

      public AliveWriterKey(AliveWriterKey other_key) {
          random_id = other_key.random_id;
      }

      public boolean equals(Object other) {
          if (other instanceof AliveWriterKey) {
              AliveWriterKey other_key = (AliveWriterKey)other;
              return random_id == other_key.random_id;
          }
          else {
              return false;
          }
      }

      public int hashCode() {
          return random_id;
      }
    }

    private static class AliveWriterData {
      public double start_time;
    }

    private final Partition m_partition;
    private final QueryConditionMgr m_qos_query;
    private final Dispatcher m_dispatcher = new Dispatcher();
    private final TopicMgr m_topic;
    private final ReaderMgr<throughput_messageDataReader> m_reader;
    private final StatusConditionMgr m_reader_condition;
    private final Attachment m_reader_attachment;
    private final Thread m_reader_thread = new Thread() {
            public void run() { reader_thread(); }
        };
    private final Thread m_report_thread = new Thread() {
            public void run() { report_thread(); }
        };
    private boolean m_active = false;
    private boolean m_has_been_discovered = false;
    private int m_byte_count = 0;
    private int m_msg_count = 0;
    private int m_config_number = 0;
    private double m_creation_time;
    private double m_creation_duration;
    private receiverDef m_def;
    private receiverQos m_qos;
    private final DDS.DataReaderQosHolder m_reader_qos = new DDS.DataReaderQosHolder();
    private final throughput_messageSeqHolder m_reader_messages = new throughput_messageSeqHolder();
    private final SampleInfoSeqHolder m_reader_infos = new SampleInfoSeqHolder();
    private final HashMap<AliveWriterKey, AliveWriterData> m_alive_writers = new HashMap<AliveWriterKey, AliveWriterData>();
    private final AliveWriterKey m_alive_writer_key = new AliveWriterKey();

    public Receiver(Partition partition) {
        m_partition = partition;
        // System.out.println("Receiver::Receiver(" + partition_id() + ")");
        m_qos_query = new QueryConditionMgr(qos_reader());
        m_topic = new TopicMgr(participant(),
                               new throughput_messageTypeSupport());
        m_reader = new ReaderMgr<throughput_messageDataReader>(subscriber(),
                                                               m_topic,
                                                               throughput_messageDataReader.class);
        m_reader_condition = new StatusConditionMgr(m_reader);
        m_reader_attachment = new Attachment(m_dispatcher,
                                             m_reader_condition);
    }

    public void finalize() {        // useful?
        // System.out.println("Receiver::finalize(" + partition_id() + ")");
    }

    public void        create(receiverDef def) {
        m_def = def;
        // System.out.println("Receiver::create(" + partition_id() + ", " + m_def.receiver_id + ")");

        // setup QueryCondition for reading this Receiver's Qos
        String[] params = new String[1];
        params[0] = Integer.toString(m_def.receiver_id);
        m_qos_query.create(ANY_SAMPLE_STATE.value,
                           ANY_VIEW_STATE.value,
                           ANY_INSTANCE_STATE.value,
                           "receiver_id = %0",
                           params);

        // setup Receiver topic
        set_topic();

        // read initial Qos
        receiverQosSeqHolder qoss = new receiverQosSeqHolder();
        SampleInfoSeqHolder infos = new SampleInfoSeqHolder();
        // REVISIT - read or take?
        int retcode = qos_reader().value().read_w_condition(qoss,
                                                            infos,
                                                            1,
                                                            m_qos_query.value());
        if (retcode == RETCODE_NO_DATA.value) {
            // no Qos instance to read, so initialize and write
            m_qos = new receiverQos();
            m_qos.group_id = m_def.group_id;
            m_qos.receiver_id = m_def.receiver_id;
            m_qos.partition_id = m_def.partition_id;
            m_qos.qos.history.depth = 1;
            m_qos.qos.latency_budget.duration.sec = 0;
            m_qos.qos.latency_budget.duration.nanosec = 0;
            retcode = qos_writer().value().write(m_qos,
                                                 0);
            qos_writer().check(retcode,
                               "receiverQosDataWriter::write");
        }
        else {
            qos_reader().check(retcode,
                               "receiverQosDataReader::read_w_condition");
            assert(qoss.value.length == 1);
            assert(infos.value.length == 1);
            m_qos = qoss.value[0];
            assert(m_qos.group_id == m_def.group_id);
            assert(m_qos.receiver_id == m_def.receiver_id);
            assert(m_qos.partition_id == m_def.partition_id);
        }
        qos_reader().value().return_loan(qoss,
                                         infos);
        set_qos();

        // start threads
        m_active = true;
        m_reader_thread.start();
        m_report_thread.start();
    }

    public void dispose() {
        // System.out.println("Receiver::dispose(" + partition_id() + ", " + m_def.receiver_id + ")");
        m_active = false;
        m_dispatcher.shutdown();
        try {
            m_reader_thread.join();
            m_report_thread.join();
        }
        catch (Exception e) {        // REVISIT
            System.err.println("Caught: " + e);
        }            
        m_qos_query.destroy();
        m_topic.destroy();
    }

    public void update_def(receiverDef def) {
        // System.out.println("Receiver::update_def(" + partition_id() + ", " + m_def.receiver_id + ")");
        assert(m_def.receiver_id == def.receiver_id);
        assert(m_active);

        // check for changes requiring thread restart
        if (m_def.scheduling_class != def.scheduling_class ||
            m_def.thread_priority != def.thread_priority ||
            m_def.topic_kind != def.topic_kind ||
            m_def.topic_id != def.topic_id) {
            // stop threads
            m_active = false;
            m_dispatcher.shutdown();
            try {
                m_reader_thread.join();
                m_report_thread.join();
            }
            catch (Exception e) {        // REVISIT
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
      
            // restart threads
            m_active = true;
            m_reader_thread.start();
            m_report_thread.start();
        }
        else {
            // update without stopping or restarting threads
            m_def = def;
            m_config_number++;
        }
    }

    public void update_qos(receiverQos qos) {
        // System.out.println("Receiver::update_qos(" + partition_id() + ", " + m_def.receiver_id + ")");
        assert(m_def.receiver_id == qos.receiver_id);
        assert(m_active);
        set_qos();
        if (m_reader.value() != null) {        // REVISIT - locking
            int retcode = m_reader.value().set_qos(m_reader_qos.value);
            m_reader.check(retcode,
                           "throughput_messageDataReader::set_qos");
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

    public SubscriberMgr subscriber() {
        return partition().subscriber();
    }

    public ReaderMgr<receiverQosDataReader> qos_reader() {
        return processor().receiver_qos_reader();
    }

    public WriterMgr<receiverQosDataWriter> qos_writer() {
        return processor().receiver_qos_writer();
    }

    public WriterMgr<discoveryReportDataWriter> discovery_report_writer() {
        return processor().discovery_report_writer();
    }

    public WriterMgr<receiverReportDataWriter> receiver_report_writer() {
        return processor().receiver_report_writer();
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
        m_reader_qos.value.history.depth = m_qos.qos.history.depth;
        m_reader_qos.value.latency_budget.duration.sec = m_qos.qos.latency_budget.duration.sec;
        m_reader_qos.value.latency_budget.duration.nanosec = m_qos.qos.latency_budget.duration.nanosec;
    }

    private void reader_thread() {
        // System.out.println("Receiver::reader_thread(" + partition_id() + ", " + m_def.receiver_id + ")");
        // REVISIT - locking?
        try {
            // Start timing now
            m_creation_time = processor().get_timestamp();
            // setup DataReader
            m_reader.create(m_reader_qos.value);
            m_reader_condition.get();
            m_reader_condition.value().set_enabled_statuses(DATA_AVAILABLE_STATUS.value);
            m_reader_attachment.attach(new Handler() {
                    public boolean handle_condition(Condition condition) {
                        return read_throughput_message(condition);
                    }
                });
            m_creation_duration = processor().get_timestamp() - m_creation_time;

            // process events
            m_dispatcher.run();
        }
        catch (DDSError error) {
            processor().report_error(error,
                                     partition_id(),
                                     m_def.receiver_id);
            System.err.println("Receiver reader thread exiting: " + error);
            error.printStackTrace();
        }
        catch (Exception e) {        // REVISIT - report?
            System.err.println("Receiver reader thread exiting: " + e);
            e.printStackTrace();
        }
        finally {
            m_reader_attachment.detach();
            m_reader_condition.release();
            m_reader.destroy();
        }
        // System.out.println("Receiver::reader_thread(" + partition_id() + ", " + m_def.receiver_id + ") exiting");
    }

    public boolean read_throughput_message(Condition condition) {
        // System.out.println("Receiver::read_throughput_message(" + partition_id() + ", " + m_def.receiver_id + ")");
        double timestamp = processor().get_timestamp();

        // REVISIT - try/catch?
        int retcode = m_reader.value().take(m_reader_messages,
                                            m_reader_infos,
                                            LENGTH_UNLIMITED.value,
                                            ANY_SAMPLE_STATE.value,
                                            ANY_VIEW_STATE.value,
                                            ANY_INSTANCE_STATE.value);
        m_reader.check(retcode,
                       "throughput_messageDataReader::take");

        int length = m_reader_messages.value.length;
        assert(length == m_reader_infos.value.length);

        if (length > 0) {
            // process throughput
            throughput_message message = m_reader_messages.value[0];
            // REVISIT - sizeof(throughput_message) in C or C++
            long size = 60 + message.payload_data.length;
            // REVISIT - lock?
            m_byte_count += (size * length);
            m_msg_count += length;

            for (int i = 0; i < length; i++) {
                message = m_reader_messages.value[i];
                SampleInfo info = m_reader_infos.value[i];
                // System.out.println("Receiver::read_throughput_message(" + partition_id() + ", " + m_def.receiver_id
                //                    + ") read message with " + message.payload_data.length + " byte payload");

                // process discovery
                if (info.view_state == NEW_VIEW_STATE.value) {
                    m_alive_writer_key.random_id = message.random_id;
                    AliveWriterData data = m_alive_writers.get(m_alive_writer_key);
                    if (data == null) {
                        /* This is an unknown writer for this reader instance */
                        data = new AliveWriterData();
                        m_alive_writers.put(new AliveWriterKey(m_alive_writer_key), data);

                        boolean has_been_reported_before = false;
                        discoveryReport discovery = new discoveryReport(); // REVISIT - use class field
                        discovery.partition_id = m_def.partition_id;
                        discovery.samples_missed = message.sequence_number - 1;
                        if (message.creation_time < m_creation_time) {
                            // writer is older than reader so report reader discovery time
                            if (!m_has_been_discovered) {
                                discovery.report_kind = discoveryKind.DataReaderDiscovery;
                                discovery.discovery_time = timestamp - m_creation_time;
                                discovery.creation_duration = m_creation_duration;
                                discovery.application_id = processor().application_id();
                                discovery.entity_id = m_def.receiver_id;
                                m_has_been_discovered = true;
                            } else {
                                has_been_reported_before = true;
                            }
                        } else {
                            // reader is older than writer so report writer discovery time
                            discovery.report_kind = discoveryKind.DataWriterDiscovery;
                            discovery.discovery_time = message.write_timestamp - message.creation_time;
                            discovery.creation_duration = message.creation_duration;
                            discovery.application_id =  message.application_id;
                            discovery.entity_id = message.transmitter_id;
                        }

                        if (!has_been_reported_before) {
                            // write discovery report
                            retcode = discovery_report_writer().value().write(discovery,
                                                                              0);
                            discovery_report_writer().check(retcode,
                                                            "DiscoveryReportDataWriter::write");
                        }
                    }
                }
            }
        }

        m_reader.value().return_loan(m_reader_messages,
                                     m_reader_infos);
        return true;
    }

    private void report_thread() {
        // System.out.println("Receiver::report_thread(" + partition_id() + ", " + m_def.receiver_id + ")");
        // REVISIT - locking?
        try {
            // setup message
            receiverReport report = new receiverReport();
            report.application_id = processor().application_id();
            report.receiver_id = m_def.receiver_id;
            report.partition_id = partition_id();
            SampleLostStatusHolder sl_status = new SampleLostStatusHolder();
            SampleRejectedStatusHolder sr_status = new SampleRejectedStatusHolder();
            RequestedDeadlineMissedStatusHolder rdm_status = new RequestedDeadlineMissedStatusHolder();

            // loop
            while (m_active) {
                // init report
                report.reader_status.samples_lost = 0;
                report.reader_status.samples_rejected = 0;
                report.reader_status.deadlines_missed = 0;

                // sleep
                Thread.sleep(m_def.report_period);

                // analyze metrics
                report.read_bytes_per_second = (int)(1000.0 * m_byte_count / m_def.report_period);
                report.read_msgs_per_second = (int)(1000.0 * m_msg_count / m_def.report_period);
                m_byte_count = 0;
                m_msg_count = 0;

                // get reader status
                if (m_reader.value() != null) { // REVISIT - lock
                    int retcode = m_reader.value().get_sample_lost_status(sl_status);
                    m_reader.check(retcode,
                                   "throughput_messageDataReader::get_sample_lost_status");
                    report.reader_status.samples_lost = sl_status.value.total_count_change;

                    retcode = m_reader.value().get_sample_rejected_status(sr_status);
                    m_reader.check(retcode,
                                   "throughput_messageDataReader::get_sample_rejected_status");
                    report.reader_status.samples_rejected = sr_status.value.total_count_change;

                    retcode = m_reader.value().get_requested_deadline_missed_status(rdm_status);
                    m_reader.check(retcode,
                                   "throughput_messageDataReader::get_requested_deadline_missed_status");
                    report.reader_status.deadlines_missed = rdm_status.value.total_count_change;
                }

                // write receiver report
                // System.out.println("Receiver::report_thread(" + partition_id() + ", " + m_def.receiver_id
                //                    + ") writing report");
                report.config_number = m_config_number;
                int retcode = receiver_report_writer().value().write(report,
                                                                     0);
                receiver_report_writer().check(retcode,
                                               "ReceiverReportDataWriter::write");
            }
        }
        catch (DDSError error) {
            processor().report_error(error,
                                     partition_id(),
                                     m_def.receiver_id);
            System.err.println("Receiver report thread exiting: " + error);
            error.printStackTrace();
        }
        catch (Exception e) {        // REVISIT - report?
            System.err.println("Receiver report thread exiting: " + e);
            e.printStackTrace();
        }
        // System.out.println("Receiver::report_thread(" + partition_id() + ", " + m_def.receiver_id + ") exiting");
    }
}
