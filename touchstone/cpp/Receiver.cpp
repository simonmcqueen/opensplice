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
#include "Receiver.h"
#include "Partition.h"
#include "Processor.h"
/* ddshelp includes */
#include "DDSError.h"
/* standard includes */
#include <iostream>
#include <sstream>

Receiver::Receiver(Partition& partition)
  : m_partition(partition),
    m_qos_query(qos_reader()),
    m_dispatcher(*this),
    m_topic(participant()),
    m_reader(subscriber(),
             m_topic),
    m_reader_condition(m_reader),
    m_reader_attachment(m_dispatcher,
                        m_reader_condition),
    m_reader_thread(*this),
    m_report_thread(*this),
    m_active(false),
    m_config_number(0),
    m_byte_count(0),
    m_msgs_count(0)
{
  // cout << "Receiver::Receiver(" << partition_id() << ")" << endl;
}

Receiver::~Receiver()
{
  // cout << "Receiver::~Receiver(" << partition_id() << ", " << m_def.receiver_id << ")" << endl;
}

void
Receiver::create(const receiverDef& def)
{
  m_def = def;
  // cout << "Receiver::create(" << partition_id() << ", " << m_def.receiver_id << ")" << endl;

  // setup QueryCondition for reading this Receiver's Qos

  stringstream id;
  id << m_def.receiver_id;
  StringSeq params;
  params.length(1);
  params[0] = id.str().c_str();
  m_qos_query.create(ANY_SAMPLE_STATE,
                     ANY_VIEW_STATE,
                     ANY_INSTANCE_STATE,
                     "receiver_id = %0",
                     params);

  // setup Receiver topic

  set_topic();

  // read initial Qos

  receiverQosSeq qoss;
  SampleInfoSeq infos;
  // REVISIT - read or take?
  ReturnCode_t retcode = qos_reader()->read_w_condition(qoss,
                                                        infos,
                                                        1,
                                                        m_qos_query);
  if (retcode == RETCODE_NO_DATA)
    {
      // no Qos instance to read, so initialize and write
      m_qos.group_id = m_def.group_id;
      m_qos.receiver_id = m_def.receiver_id;
      m_qos.partition_id = m_def.partition_id;
      m_qos.qos.latency_budget.duration.sec = 0;
      m_qos.qos.latency_budget.duration.nanosec = 0;
      m_qos.qos.history.depth = 1;

      retcode = qos_writer()->write(m_qos,
                                    0);
      qos_writer().check(retcode,
                         "receiverQosDataWriter::write");
    }
  else
    {
      qos_reader().check(retcode,
                         "receiverQosDataReader::read_w_condition");

      assert(qoss.length() == 1);
      assert(infos.length() == 1);
      m_qos = qoss[0];
      assert(m_qos.group_id == m_def.group_id);
      assert(m_qos.receiver_id == m_def.receiver_id);
      assert(m_qos.partition_id == m_def.partition_id);
    }
  qos_reader()->return_loan(qoss,
                            infos);

  set_qos();

  // start threads

  m_active = true;
  m_reader_thread.create(&Receiver::reader_thread);
  m_report_thread.create(&Receiver::report_thread);
}

void
Receiver::dispose()
{
  // cout << "Receiver::dispose(" << partition_id() << ", " << m_def.receiver_id << ")" << endl;

  m_active = false;
  m_dispatcher.shutdown();
  m_reader_thread.join();
  m_report_thread.join();
}
  
void
Receiver::update_def(const receiverDef& def)
{
  // cout << "Receiver::update_def(" << partition_id() << ", " << m_def.receiver_id << ")" << endl;

  assert(m_def.receiver_id == def.receiver_id);
  assert(m_active);

  // check for changes requiring thread restart

  if (m_def.scheduling_class != def.scheduling_class ||
      m_def.thread_priority != def.thread_priority ||
      m_def.topic_kind != def.topic_kind ||
      m_def.topic_id != def.topic_id)
    {
      // stop threads

      m_active = false;
      m_dispatcher.shutdown();
      m_reader_thread.join();
      m_report_thread.join();

      // check for topic change

      if (m_def.topic_kind != def.topic_kind ||
          m_def.topic_id != def.topic_id)
        {
          m_def = def;
          set_topic();
          set_qos();
        }
      else
        {
          m_def = def;
        }

      // finish updating

      m_config_number++;
      
      // restart threads

      m_active = true;
      m_reader_thread.create(&Receiver::reader_thread);
      m_report_thread.create(&Receiver::report_thread);
    }
  else
    {
      // update without stopping and restarting threads

      m_def = def;
      m_config_number++;
    }
}

void
Receiver::update_qos(const receiverQos& qos)
{
  // cout << "Receiver::update_qos(" << partition_id() << ", " << m_def.receiver_id << ")" << endl;

  assert(m_def.receiver_id == qos.receiver_id);
  assert(m_active);

  set_qos();

  if (m_reader)                        // REVISIT - locking
    {
      ReturnCode_t retcode = m_reader->set_qos(m_reader_qos);
      m_reader.check(retcode,
                     "throughput_messageDataReader::set_qos");
    }

  m_config_number++;
}

void
Receiver::set_topic()
{
  TopicQos topic_qos;
  ReturnCode_t retcode = participant()->get_default_topic_qos(topic_qos);
  participant().check(retcode,
                      "Participant::get_default_topic_qos");

  stringstream topic_name;
  switch (m_def.topic_kind)
    {
    case RELIABLE:
      topic_qos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
      topic_name << "ThroughputTopic_" << m_def.topic_id << "_R";
      break;
    case TRANSIENT:
      topic_qos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
      topic_qos.durability.kind = DDS::TRANSIENT_DURABILITY_QOS;
      topic_name << "ThroughputTopic_" << m_def.topic_id << "_T";
      break;
    case PERSISTENT:
      topic_qos.durability.kind = DDS::PERSISTENT_DURABILITY_QOS;
      topic_name << "ThroughputTopic_" << m_def.topic_id << "_P";
      break;
    case BEST_EFFORT:
      topic_name << "ThroughputTopic_" << m_def.topic_id << "_B";
      break;
    }

  m_topic.create(topic_name.str().c_str(),
                 topic_qos);
}

void
Receiver::set_qos()
{
  ReturnCode_t retcode = subscriber()->get_default_datareader_qos(m_reader_qos);
  subscriber().check(retcode,
                     "Subscriber::get_default_datareader_qos");

  TopicQos topic_qos;
  retcode = m_topic->get_qos(topic_qos);
  m_topic.check(retcode,
                "Topic::get_qos");

  retcode = subscriber()->copy_from_topic_qos(m_reader_qos,
                                              topic_qos);
  subscriber().check(retcode,
                     "Subscriber::copy_from_topic_qos");

  m_reader_qos.history.depth = m_qos.qos.history.depth;
  m_reader_qos.latency_budget.duration.sec = m_qos.qos.latency_budget.duration.sec;
  m_reader_qos.latency_budget.duration.nanosec = m_qos.qos.latency_budget.duration.nanosec;
}

void
Receiver::reader_thread()
{
  // cout << "Receiver::reader_thread(" << partition_id() << ", " << m_def.receiver_id << ")" << endl;

  // REVISIT - locking?

  try
    {
      // setup DataReader

      m_creation_time = processor().get_timestamp();
      m_reader.create(m_reader_qos);
      m_reader_condition.get();
      m_reader_condition->set_enabled_statuses(DATA_AVAILABLE_STATUS);
      m_reader_attachment.attach(&Receiver::read_throughput_message);

      m_creation_duration = processor().get_timestamp() - m_creation_time;

      // process events

      m_dispatcher.run();
    }
  catch (const DDSError& error)
    {
      processor().report_error(error,
                               partition_id(),
                               m_def.receiver_id);
      cerr << "Receiver reader thread exiting: " << error << endl;
    }

  m_reader_attachment.detach();
  m_reader_condition.release();
  m_reader.destroy();

  // cout << "Receiver::reader_thread(" << partition_id() << ", " << m_def.receiver_id << ") exiting" << endl;
}

bool
Receiver::read_throughput_message(Condition_ptr condition)
{
  // cout << "Receiver::read_throughput_message(" << partition_id() << ", " << m_def.receiver_id << ")" << endl;

  timestamp stamp = processor().get_timestamp();
  
  // REVISIT - try/catch?

  ReturnCode_t retcode = m_reader->take(m_reader_messages,
                                        m_reader_infos,
                                        DDS::LENGTH_UNLIMITED,
                                        ANY_SAMPLE_STATE,
                                        ANY_VIEW_STATE,
                                        ANY_INSTANCE_STATE);
  m_reader.check(retcode,
                 "throughput_messageDataReader::take");

  ULong length = m_reader_messages.length();
  assert(length == m_reader_infos.length());

  if (length > 0) 
    {
      // REVISIT - same as in C?
      m_byte_count += length * (sizeof(throughput_message) +
                         m_reader_messages[0].payload_data.length());
      m_msgs_count += length;

      // process discovery
      for (ULong i = 0; i < length; i++)
        {
          const throughput_message& message(m_reader_messages[i]);
          const SampleInfo& info(m_reader_infos[i]);

          if (info.view_state == NEW_VIEW_STATE)
            {
              AliveWriterKey key = {message.random_id};
              AliveWriterData& alive_writer = m_alive_writers[key];
              bool has_been_reported_before = FALSE;
  
              if (alive_writer.start_time == 0)
                { // this is a newly discovered instance
                  discoveryReport discovery;
                  discovery.partition_id = m_def.partition_id;
                  discovery.samples_missed = message.sequence_number - 1;
    
                  alive_writer.start_time = message.creation_time;
                  if (message.creation_time < m_creation_time)
                    {
                      // writer is older than reader so report reader discovery time
                      if (!m_receiver_was_discovered)
                        {
                          discovery.report_kind = DataReaderDiscovery;
                          discovery.discovery_time = stamp - m_creation_time;
                          discovery.creation_duration = m_creation_duration;
                          discovery.application_id = processor().application_id();
                          discovery.entity_id = m_def.receiver_id;
                          m_receiver_was_discovered = true;
                        } else {
                          has_been_reported_before = true;
                        }
                    }
                  else
                    {
                      // reader is older than writer so report writer discovery time
      
                      discovery.report_kind = DataWriterDiscovery;
                      discovery.discovery_time = message.write_timestamp - message.creation_time;
                      discovery.creation_duration = message.creation_duration;
                      discovery.application_id =  message.application_id;
                      discovery.entity_id = message.transmitter_id;
                    }
      
                  if (!has_been_reported_before)
                    {
    
                      // write discovery report
    
                      retcode = discovery_report_writer()->write(discovery,
                                                                 0);
    
                      discovery_report_writer().check(retcode,
                                                    "DiscoveryReportDataWriter::write");
                    }
                }
            }
        }
    }


  m_reader->return_loan(m_reader_messages,
                        m_reader_infos);

  return true;
}

void
Receiver::report_thread()
{
  // cout << "Receiver::report_thread(" << partition_id() << ", " << m_def.receiver_id << ")" << endl;

  // REVISIT - locking?

  try
    {
      // setup message

      receiverReport report;
      report.application_id = processor().application_id();
      report.receiver_id = m_def.receiver_id;
      report.partition_id = partition_id();

      SampleLostStatus sl_status;
      SampleRejectedStatus sr_status;
      RequestedDeadlineMissedStatus rdm_status;

      while (m_active)
        {
          // init report

          report.reader_status.samples_lost = 0;
          report.reader_status.samples_rejected = 0;
          report.reader_status.deadlines_missed = 0;

          // sleep

          Processor::msec_sleep(m_def.report_period);

          // analyze metrics

          report.read_bytes_per_second = (ULong)(1000.0 * m_byte_count / m_def.report_period);
          report.read_msgs_per_second = (ULong)(1000.0 * m_msgs_count / m_def.report_period);
          m_byte_count = 0;
          m_msgs_count = 0;

          // get reader status

          if (m_reader)                // REVISIT - lock
            {
              ReturnCode_t retcode = m_reader->get_sample_lost_status(sl_status);
              m_reader.check(retcode,
                             "throughput_messageDataReader::get_sample_lost_status");
              report.reader_status.samples_lost = sl_status.total_count_change;

              retcode = m_reader->get_sample_rejected_status(sr_status);
              m_reader.check(retcode,
                             "throughput_messageDataReader::get_sample_rejected_status");
              report.reader_status.samples_rejected = sr_status.total_count_change;

              retcode = m_reader->get_requested_deadline_missed_status(rdm_status);
              m_reader.check(retcode,
                             "throughput_messageDataReader::get_requested_deadline_missed_status");
              report.reader_status.deadlines_missed = rdm_status.total_count_change;
            }

          // write receiver report

          // cout << "Receiver::report_thread(" << partition_id() << ", " << m_def.receiver_id
          //      << ") writing report" << endl;

          report.config_number = m_config_number;
          ReturnCode_t retcode = receiver_report_writer()->write(report,
                                                                 0);
          receiver_report_writer().check(retcode,
                                         "ReceiverReportDataWriter::write");
        }
    }
  catch (const DDSError& error)
    {
      processor().report_error(error,
                               partition_id(),
                               m_def.receiver_id);
      cerr << "Receiver report thread exiting: " << error << endl;
    }

  // cout << "Receiver::report_thread(" << partition_id() << ", " << m_def.receiver_id << ") exiting" << endl;
}
