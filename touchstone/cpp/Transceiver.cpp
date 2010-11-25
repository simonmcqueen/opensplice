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
#include "Transceiver.h"
#include "Partition.h"
#include "Processor.h"
/* ddshelp includes */
#include "DDSError.h"
/* standard includes */
#include <iostream>
#include <sstream>
#include <algorithm>

/* Translate 0,1,2,...,26,27,28,... into 'A','B','C',...,'a','b','c' */
#define PAYLOAD_ITEM_FROM_UINT(i) \
    (char)(65+(((i/2)%26)+(i%2)*32))

Transceiver::Transceiver(Partition& partition)
  : m_partition(partition),
    m_qos_query(qos_reader()),
    m_dispatcher(*this),
    m_topic(participant()),
    m_writer(publisher(),
	     m_topic),
    m_echo_topic(participant()),
    m_reader(subscriber(),
	     m_echo_topic),
    m_reader_condition(m_reader),
    m_reader_attachment(m_dispatcher,
			m_reader_condition),
    m_writer_thread(*this),
    m_reader_thread(*this),
    m_report_thread(*this),
    m_writer_active(false),
    m_reader_active(false),
    m_report_active(false),
    m_config_number(0)
{
  // cout << "Transceiver::Transceiver(" << partition_id() << ")" << endl;
}

Transceiver::~Transceiver()
{
  // cout << "Transceiver::~Transceiver(" << partition_id() << ", " << m_def.transceiver_id << ")" << endl;
}

void
Transceiver::create(const transceiverDef& def)
{
  m_def = def;
  // cout << "Transceiver::create(" << partition_id() << ", " << m_def.transceiver_id << ")" << endl;

  // setup QueryCondition for reading this Transceiver's Qos

  stringstream id;
  id << m_def.transceiver_id;
  StringSeq params;
  params.length(1);
  params[0] = id.str().c_str();
  m_qos_query.create(ANY_SAMPLE_STATE,
		     ANY_VIEW_STATE,
		     ANY_INSTANCE_STATE,
		     "transceiver_id = %0",
		     params);

  // setup Transceiver topic

  set_topics();

  // read initial Qos

  transceiverQosSeq qoss;
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
      m_qos.transceiver_id = m_def.transceiver_id;
      m_qos.partition_id = m_def.partition_id;
      m_qos.writer_qos.latency_budget.duration.sec = 0;
      m_qos.writer_qos.latency_budget.duration.nanosec = 0;
      m_qos.writer_qos.transport_priority.value = 0;
      m_qos.reader_qos.history.depth = 1;
      m_qos.reader_qos.latency_budget.duration.sec = 0;
      m_qos.reader_qos.latency_budget.duration.nanosec = 0;

      retcode = qos_writer()->write(m_qos,
				    0);
      qos_writer().check(retcode,
			 "transceiverQosDataWriter::write");
    }
  else
    {
      qos_reader().check(retcode,
			 "transceiverQosDataReader::read_w_condition");

      assert(qoss.length() == 1);
      assert(infos.length() == 1);
      m_qos = qoss[0];
      assert(m_qos.group_id == m_def.group_id);
      assert(m_qos.transceiver_id == m_def.transceiver_id);
      assert(m_qos.partition_id == m_def.partition_id);
    }
  qos_reader()->return_loan(qoss,
			    infos);

  set_qos();

  // start threads

  m_writer_active = true;
  m_writer_thread.create(&Transceiver::writer_thread);

  m_reader_active = true;
  m_reader_thread.create(&Transceiver::reader_thread);

  m_report_active = true;
  m_report_thread.create(&Transceiver::report_thread);
}

void
Transceiver::dispose()
{
  // cout << "Transceiver::dispose(" << partition_id() << ", " << m_def.transceiver_id << ")" << endl;

  m_writer_active = false;
  m_reader_active = false;
  m_report_active = false;
  m_dispatcher.shutdown();
  m_writer_thread.join();
  m_reader_thread.join();
  m_report_thread.join();
}
  
void
Transceiver::update_def(const transceiverDef& def)
{
  // cout << "Transceiver::update_def(" << partition_id() << ", " << m_def.transceiver_id << ")" << endl;

  assert(m_def.transceiver_id == def.transceiver_id);
  assert(m_writer_active);
  assert(m_reader_active);
  assert(m_report_active);

  // check for changes requiring thread restart

  if (m_def.scheduling_class != def.scheduling_class ||
      m_def.thread_priority != def.thread_priority ||
      m_def.topic_kind != def.topic_kind ||
      m_def.topic_id != def.topic_id)
    {
      // stop threads

      m_writer_active = false;
      m_reader_active = false;
      m_report_active = false;
      m_dispatcher.shutdown();
      m_writer_thread.join();
      m_reader_thread.join();
      m_report_thread.join();

      // check for topic change

      if (m_def.topic_kind != def.topic_kind ||
	  m_def.topic_id != def.topic_id)
	{
	  m_def = def;
	  set_topics();
	  set_qos();
	}
      else
	{
	  m_def = def;
	}

      // finish updating

      m_config_number++;
      
      // restart threads

      m_writer_active = true;
      m_writer_thread.create(&Transceiver::writer_thread);

      m_reader_active = true;
      m_reader_thread.create(&Transceiver::reader_thread);

      m_report_active = true;
      m_report_thread.create(&Transceiver::report_thread);

    }
  else
    {
      // update without stopping and restarting threads

      m_def = def;
      m_config_number++;
    }
}

void
Transceiver::update_qos(const transceiverQos& qos)
{
  // cout << "Transceiver::update_qos(" << partition_id() << ", " << m_def.transceiver_id << ")" << endl;

  assert(m_def.transceiver_id == qos.transceiver_id);
  assert(m_writer_active);
  assert(m_reader_active);
  assert(m_report_active);

  set_qos();

  if (m_writer)			// REVISIT - locking
    {
      ReturnCode_t retcode = m_writer->set_qos(m_writer_qos);
      m_writer.check(retcode,
		     "latency_messageDataWriter::set_qos");
    }

  if (m_reader)			// REVISIT - locking
    {
      ReturnCode_t retcode = m_reader->set_qos(m_reader_qos);
      m_reader.check(retcode,
		     "latency_messageDataReader::set_qos");
    }

  m_config_number++;
}

void
Transceiver::set_topics()
{
  TopicQos topic_qos;
  ReturnCode_t retcode = participant()->get_default_topic_qos(topic_qos);
  participant().check(retcode,
		      "Participant::get_default_topic_qos");

  stringstream topic_name;
  stringstream echo_topic_name;
  switch (m_def.topic_kind)
    {
    case RELIABLE:
      topic_qos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
      topic_name << "LatencyTopic_" << m_def.topic_id << "_R";
      echo_topic_name << "LatencyEchoTopic_" << m_def.topic_id << "_R";
      break;
    case TRANSIENT:
      topic_qos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
      topic_qos.durability.kind = DDS::TRANSIENT_DURABILITY_QOS;
      topic_name << "LatencyTopic_" << m_def.topic_id << "_T";
      echo_topic_name << "LatencyEchoTopic_" << m_def.topic_id << "_T";
      break;
    case PERSISTENT:
      topic_qos.durability.kind = DDS::PERSISTENT_DURABILITY_QOS;
      topic_name << "LatencyTopic_" << m_def.topic_id << "_P";
      echo_topic_name << "LatencyEchoTopic_" << m_def.topic_id << "_P";
      break;
    case BEST_EFFORT:
      topic_name << "LatencyTopic_" << m_def.topic_id << "_B";
      echo_topic_name << "LatencyEchoTopic_" << m_def.topic_id << "_B";
      break;
    }

  m_topic.create(topic_name.str().c_str(),
		 topic_qos);

  m_echo_topic.create(echo_topic_name.str().c_str(),
		      topic_qos);
}

void
Transceiver::set_qos()
{
  // set writer Qos

  ReturnCode_t retcode = publisher()->get_default_datawriter_qos(m_writer_qos);
  publisher().check(retcode,
		    "Publisher::get_default_datawriter_qos");

  TopicQos topic_qos;
  retcode = m_topic->get_qos(topic_qos);
  m_topic.check(retcode,
		"Topic::get_qos");

  retcode = publisher()->copy_from_topic_qos(m_writer_qos,
					     topic_qos);
  publisher().check(retcode,
		    "Publisher::copy_from_topic_qos");

  m_writer_qos.latency_budget.duration.sec = m_qos.writer_qos.latency_budget.duration.sec;
  m_writer_qos.latency_budget.duration.nanosec = m_qos.writer_qos.latency_budget.duration.nanosec;
  m_writer_qos.transport_priority.value = m_qos.writer_qos.transport_priority.value;

  // set reader Qos
  
  retcode = subscriber()->get_default_datareader_qos(m_reader_qos);
  subscriber().check(retcode,
		     "Subscriber::get_default_datareader_qos");

  retcode = m_echo_topic->get_qos(topic_qos);
  m_echo_topic.check(retcode,
		     "Topic::get_qos");

  retcode = subscriber()->copy_from_topic_qos(m_reader_qos,
					      topic_qos);
  subscriber().check(retcode,
		     "Subscriber::copy_from_topic_qos");

  m_reader_qos.history.depth = m_qos.reader_qos.history.depth;
  m_reader_qos.latency_budget.duration.sec = m_qos.reader_qos.latency_budget.duration.sec;
  m_reader_qos.latency_budget.duration.nanosec = m_qos.reader_qos.latency_budget.duration.nanosec;
}

void
Transceiver::writer_thread()
{
  // cout << "Transceiver::writer_thread(" << partition_id() << ", " << m_def.transceiver_id << ")" << endl;

  // REVISIT - locking?

  try
    {
      // setup DataWriter
      m_writer.create(m_writer_qos);

      // setup message
      latency_message message;
      message.application_id = processor().application_id();
      message.random_id = processor().random_id();
      message.transceiver_id = m_def.transceiver_id;
      message.sequence_number = 0;
      ULong last_size = 0;

      while (m_writer_active)
	{
	  // adjust message size if needed
	  if (m_def.message_size != last_size)
	    {
	      ULong size = sizeof(latency_message); // REVISIT - same as size in C?
	      // cout << "sizeof(latency_message) = " << size << endl; // REVISIT - temporary
	      size = (m_def.message_size > size) ? m_def.message_size - size : 1;
	      last_size = m_def.message_size;
	      message.payload_data.length(size);
	      for (ULong i = 0; i < size; i++)
		{
		  message.payload_data[i] = PAYLOAD_ITEM_FROM_UINT(i);
		}
	    }

	  // update message
	  message.sequence_number++;
	  message.config_number = m_config_number;
	  message.write_timestamp = processor().get_timestamp();

	  // write message
	  ReturnCode_t retcode = m_writer->write(message,
						 0);
	  m_writer.check(retcode,
			 "latency_messageDataWriter::write");
	  // cout << "Transceiver::writer_thread(" << partition_id() << ", " << m_def.transceiver_id
	  //      << ") wrote messsage " << message.sequence_number
	  //      << " with " << message.payload_data.length() << " byte payload" << endl;

	  // sleep
	  Processor::msec_sleep(m_def.write_period);
	}

    }
  catch (const DDSError& error)
    {
      processor().report_error(error,
			       partition_id(),
			       m_def.transceiver_id);
      cerr << "Transceiver writer thread exiting: " << error << endl;
    }

  m_writer.destroy();

  // cout << "Transceiver::writer_thread(" << partition_id() << ", " << m_def.transceiver_id << ") exiting" << endl;
}


void
Transceiver::reader_thread()
{
  // cout << "Transceiver::reader_thread(" << partition_id() << ", " << m_def.transceiver_id << ")" << endl;

  // REVISIT - locking?

  try
    {
      // setup DataReader

      m_reader.create(m_reader_qos);
      m_reader_condition.get();
      m_reader_condition->set_enabled_statuses(DATA_AVAILABLE_STATUS);
      m_reader_attachment.attach(&Transceiver::read_latency_message);

      // process events

      m_previous_time = 0;
      m_dispatcher.run();
    }
  catch (const DDSError& error)
    {
      processor().report_error(error,
			       partition_id(),
			       m_def.transceiver_id);
      cerr << "Transceiver reader thread exiting: " << error << endl;
    }

  m_reader_attachment.detach();
  m_reader_condition.release();
  m_reader.destroy();

  // cout << "Transceiver::reader_thread(" << partition_id() << ", " << m_def.transceiver_id << ") exiting" << endl;
}

bool
Transceiver::read_latency_message(Condition_ptr condition)
{
  // cout << "Transceiver::read_latency_message(" << partition_id() << ", " << m_def.transceiver_id << ")" << endl;

  // REVISIT - try/catch?

  ReturnCode_t retcode = m_reader->take(m_reader_messages,
					m_reader_infos,
					1,
					ANY_SAMPLE_STATE,
					ANY_VIEW_STATE,
					ANY_INSTANCE_STATE);
  m_reader.check(retcode,
		 "latency_messageDataReader::take");

  timestamp read_time = processor().get_timestamp();

  ULong length = m_reader_messages.length();
  assert(length == m_reader_infos.length());
  for (ULong i = 0; i < length; i++)
    {
      const latency_message& message(m_reader_messages[i]);
      const SampleInfo& info(m_reader_infos[i]);

      // cout << "Transceiver::read_latency_message(" << partition_id() << ", " << m_def.transceiver_id
      //      << ") read echo message " << message.sequence_number
      //      << " with " << message.payload_data.length() << " byte payload" << endl;

      timestamp write_time = message.write_timestamp;
      timestamp echo_time = message.echo_timestamp;
      timestamp source_time = Processor::to_timestamp(info.source_timestamp);
      timestamp arrival_time = read_time; // REVISIT - should be Processor::to_timestamp(info.arrival_timestamp);

      m_send_latency.add(message.send_latency);
      m_echo_latency.add(read_time - echo_time);
      m_trip_latency.add(read_time - write_time);
      m_send_source_latency.add(message.source_latency);
      m_send_arrival_latency.add(message.arrival_latency);
      m_send_trip_latency.add(message.send_latency - message.source_latency - message.arrival_latency);
      m_echo_source_latency.add(source_time - echo_time);
      m_echo_arrival_latency.add(read_time - arrival_time);
      m_echo_trip_latency.add(arrival_time - source_time);

      if (m_previous_time != 0)
	{
	  m_inter_arrival_time.add(read_time - m_previous_time);
	}
      m_previous_time = read_time;
    }

  // REVISIT - set m_previous_time to 0 if exception caught?

  m_reader->return_loan(m_reader_messages,
			m_reader_infos);

  return true;
}

void
Transceiver::report_thread()
{
  // cout << "Transceiver::report_thread(" << partition_id() << ", " << m_def.transceiver_id << ")" << endl;

  // REVISIT - locking?

  try
    {
      // setup message

      transceiverReport report;
      report.application_id = processor().application_id();
      report.transceiver_id = m_def.transceiver_id;
      report.partition_id = partition_id();

      SampleLostStatus sl_status;
      SampleRejectedStatus sr_status;
      RequestedDeadlineMissedStatus rdm_status;
      OfferedDeadlineMissedStatus odm_status;

      while (m_report_active)
	{
	  // init report

	  report.reader_status.samples_lost = 0;
	  report.reader_status.samples_rejected = 0;
	  report.reader_status.deadlines_missed = 0;
	  report.writer_status.deadlines_missed = 0;

	  // init metrics

	  m_send_latency.init();
	  m_echo_latency.init();
	  m_trip_latency.init();
	  m_send_source_latency.init();
	  m_send_arrival_latency.init();
	  m_send_trip_latency.init();
	  m_echo_source_latency.init();
	  m_echo_arrival_latency.init();
	  m_echo_trip_latency.init();
	  m_inter_arrival_time.init();

	  // sleep

	  Processor::msec_sleep(m_def.report_period);

	  // analyze metrics

	  m_send_latency.analyze(report.send_latency);
	  m_echo_latency.analyze(report.echo_latency);
	  m_trip_latency.analyze(report.trip_latency);
	  m_send_source_latency.analyze(report.send_source_latency);
	  m_send_arrival_latency.analyze(report.send_arrival_latency);
	  m_send_trip_latency.analyze(report.send_trip_latency);
	  m_echo_source_latency.analyze(report.echo_source_latency);
	  m_echo_arrival_latency.analyze(report.echo_arrival_latency);
	  m_echo_trip_latency.analyze(report.echo_trip_latency);
	  m_inter_arrival_time.analyze(report.inter_arrival_time);
	  
	  // get reader status

	  if (m_reader)		// REVISIT - lock
	    {
	      ReturnCode_t retcode = m_reader->get_sample_lost_status(sl_status);
	      m_reader.check(retcode,
			     "latency_messageDataReader::get_sample_lost_status");
	      report.reader_status.samples_lost = sl_status.total_count_change;

	      retcode = m_reader->get_sample_rejected_status(sr_status);
	      m_reader.check(retcode,
			     "latency_messageDataReader::get_sample_rejected_status");
	      report.reader_status.samples_rejected = sr_status.total_count_change;

	      retcode = m_reader->get_requested_deadline_missed_status(rdm_status);
	      m_reader.check(retcode,
			     "latency_messageDataReader::get_requested_deadline_missed_status");
	      report.reader_status.deadlines_missed = rdm_status.total_count_change;
	    }

	  // get writer status

	  if (m_writer)		// REVISIT - lock
	    {
	      ReturnCode_t retcode = m_writer->get_offered_deadline_missed_status(odm_status);
	      m_reader.check(retcode,
			     "latency_messageDataReader::get_offered_deadline_missed_status");
	      report.writer_status.deadlines_missed = odm_status.total_count_change;
	    }

	  // write transceiver report

	  // cout << "Transceiver::report_thread(" << partition_id() << ", " << m_def.transceiver_id
	  //      << ") writing report" << endl;

	  report.config_number = m_config_number;
	  ReturnCode_t retcode = transceiver_report_writer()->write(report,
								    0);
	  transceiver_report_writer().check(retcode,
					    "TransceiverReportDataWriter::write");
	}

    }
  catch (const DDSError& error)
    {
      processor().report_error(error,
			       partition_id(),
			       m_def.transceiver_id);
      cerr << "Transceiver report thread exiting: " << error << endl;
    }

  // cout << "Transceiver::report_thread(" << partition_id() << ", " << m_def.transceiver_id << ") exiting" << endl;
}

void
Transceiver::Metrics::analyze(Reports& reports)
{
  sort(m_stamps.begin(),
       m_stamps.end());

  report(100.0,
	 reports[0]);

  report(99.9,
	 reports[1]);

  report(99.0,
	 reports[2]);

  report(90.0,
	 reports[3]);
}

void
Transceiver::Metrics::report(double percentile,
			     metricsReport& report)
{
  report.percentile = percentile;

  int count = m_stamps.size();
  if (!count)
    {
      report.sample_count = 0;
      report.minimum = 0;
      report.average = 0;
      report.maximum = 0;
      report.deviation = 0;
    }
  else
    {
      int range = int((count * percentile + 50.0) / 100.0); // REVISIT - C version doesn't round

      double sum = 0.0;
      for (int i = 0; i < range; i++)
	{
	  sum += m_stamps[i];
	}
      double average = sum / range;

      sum = 0.0;
      for (int i = 0; i < range; i++)
	{
	  double diff = m_stamps[i] - average;
	  sum += (diff * diff);
	}

      report.sample_count = range;
      report.minimum = m_stamps[0];
      report.average = average;
      assert(range > 0);
      report.maximum = m_stamps[range - 1];
      report.deviation = sqrt(sum / double(range));
      
    }
}
