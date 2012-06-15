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
#include "Transponder.h"
#include "Partition.h"
#include "Processor.h"
/* ddshelp includes */
#include "DDSError.h"
/* standard includes */
#include <iostream>
#include <sstream>

Transponder::Transponder(Partition& partition)
  : m_partition(partition),
    m_qos_query(qos_reader()),
    m_dispatcher(*this),
    m_topic(participant()),
    m_reader(subscriber(),
	     m_topic),
    m_reader_condition(m_reader),
    m_reader_attachment(m_dispatcher,
			m_reader_condition),
    m_echo_topic(participant()),
    m_writer(publisher(),
	     m_echo_topic),
    m_thread(*this),
    m_active(false),
    m_config_number(0)
{
  // cout << "Transponder::Transponder(" << partition_id() << ")" << endl;
}

Transponder::~Transponder()
{
  // cout << "Transponder::~Transponder(" << partition_id() << ", " << m_def.transponder_id << ")" << endl;
}

void
Transponder::create(const transponderDef& def)
{
  m_def = def;
  // cout << "Transponder::create(" << partition_id() << ", " << m_def.transponder_id << ")" << endl;

  // setup QueryCondition for reading this Transponder's Qos

  stringstream id;
  id << m_def.transponder_id;
  StringSeq params;
  params.length(1);
  params[0] = id.str().c_str();
  m_qos_query.create(ANY_SAMPLE_STATE,
		     ANY_VIEW_STATE,
		     ANY_INSTANCE_STATE,
		     "transponder_id = %0",
		     params);

  // setup Transponder topic

  set_topics();

  // read initial Qos

  transponderQosSeq qoss;
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
      m_qos.transponder_id = m_def.transponder_id;
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
			 "transponderQosDataWriter::write");
    }
  else
    {
      qos_reader().check(retcode,
			 "transponderQosDataReader::read_w_condition");

      assert(qoss.length() == 1);
      assert(infos.length() == 1);
      m_qos = qoss[0];
      assert(m_qos.group_id == m_def.group_id);
      assert(m_qos.transponder_id == m_def.transponder_id);
      assert(m_qos.partition_id == m_def.partition_id);
    }
  qos_reader()->return_loan(qoss,
			    infos);

  set_qos();

  // start thread

  m_active = true;
  m_thread.create(&Transponder::thread);
}

void
Transponder::dispose()
{
  // cout << "Transponder::dispose(" << partition_id() << ", " << m_def.transponder_id << ")" << endl;

  m_active = false;
  m_dispatcher.shutdown();
  m_thread.join();
}
  
void
Transponder::update_def(const transponderDef& def)
{
  // cout << "Transponder::update_def(" << partition_id() << ", " << m_def.transponder_id << ")" << endl;

  assert(m_def.transponder_id == def.transponder_id);
  assert(m_active);

  // check for changes requiring thread restart

  if (m_def.scheduling_class != def.scheduling_class ||
      m_def.thread_priority != def.thread_priority ||
      m_def.topic_kind != def.topic_kind ||
      m_def.topic_id != def.topic_id)
    {
      // stop thread

      m_active = false;
      m_dispatcher.shutdown();
      m_thread.join();

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
      
      // restart thread

      m_active = true;
      m_thread.create(&Transponder::thread);
    }
  else
    {
      // update without stopping and restarting threads

      m_def = def;
      m_config_number++;
    }
}

void
Transponder::update_qos(const transponderQos& qos)
{
  // cout << "Transponder::update_qos(" << partition_id() << ", " << m_def.transponder_id << ")" << endl;

  assert(m_def.transponder_id == qos.transponder_id);
  assert(m_active);

  set_qos();

  if (m_reader)			// REVISIT - locking
    {
      ReturnCode_t retcode = m_reader->set_qos(m_reader_qos);
      m_reader.check(retcode,
		     "latency_messageDataReader::set_qos");
    }

  if (m_writer)			// REVISIT - locking
    {
      ReturnCode_t retcode = m_writer->set_qos(m_writer_qos);
      m_writer.check(retcode,
		     "latency_messageDataWriter::set_qos");
    }

  m_config_number++;
}

void
Transponder::set_topics()
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
Transponder::set_qos()
{
  // set reader Qos
  
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

  m_reader_qos.history.depth = m_qos.reader_qos.history.depth;
  m_reader_qos.latency_budget.duration.sec = m_qos.reader_qos.latency_budget.duration.sec;
  m_reader_qos.latency_budget.duration.nanosec = m_qos.reader_qos.latency_budget.duration.nanosec;

  // set writer Qos

  retcode = publisher()->get_default_datawriter_qos(m_writer_qos);
  publisher().check(retcode,
		    "Publisher::get_default_datawriter_qos");

  retcode = m_echo_topic->get_qos(topic_qos);
  m_echo_topic.check(retcode,
		     "Topic::get_qos");

  retcode = publisher()->copy_from_topic_qos(m_writer_qos,
					     topic_qos);
  publisher().check(retcode,
		    "Publisher::copy_from_topic_qos");

  m_writer_qos.latency_budget.duration.sec = m_qos.writer_qos.latency_budget.duration.sec;
  m_writer_qos.latency_budget.duration.nanosec = m_qos.writer_qos.latency_budget.duration.nanosec;
  m_writer_qos.transport_priority.value = m_qos.writer_qos.transport_priority.value;

}

void
Transponder::thread()
{
  // cout << "Transponder::thread(" << partition_id() << ", " << m_def.transponder_id << ")" << endl;

  // REVISIT - locking?

  try
    {
      // setup DataReader

      m_reader.create(m_reader_qos);
      m_reader_condition.get();
      m_reader_condition->set_enabled_statuses(DATA_AVAILABLE_STATUS);
      m_reader_attachment.attach(&Transponder::read_latency_message);

      // setup DataWriter

      m_writer.create(m_writer_qos);

      // process events

      m_dispatcher.run();
    }
  catch (const DDSError& error)
    {
      processor().report_error(error,
			       partition_id(),
			       m_def.transponder_id);
      cerr << "Transponder thread exiting: " << error << endl;
    }

  m_reader_attachment.detach();
  m_reader_condition.release();
  m_reader.destroy();

  // cout << "Transponder::thread(" << partition_id() << ", " << m_def.transponder_id << ") exiting" << endl;
}

bool
Transponder::read_latency_message(Condition_ptr condition)
{
  // cout << "Transponder::read_latency_message(" << partition_id() << ", " << m_def.transponder_id << ")" << endl;

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
      latency_message& message(m_reader_messages[i]);
      const SampleInfo& info(m_reader_infos[i]);

      // cout << "Transponder::read_latency_message(" << partition_id() << ", " << m_def.transponder_id
      //      << ") read/writing message " << message.sequence_number
      //      << " with " << message.payload_data.length() << " byte payload" << endl;

      // generate echo message

      timestamp write_time = message.write_timestamp;
      timestamp source_time = Processor::to_timestamp(info.source_timestamp);
      timestamp arrival_time = read_time; // REVISIT - should be Processor::to_timestamp(info.reception_timestamp);

      message.send_latency = read_time - write_time;
      message.echo_timestamp = processor().get_timestamp();
      message.source_latency = source_time - write_time;
      message.arrival_latency = read_time - arrival_time;
      message.config_number = m_config_number;

      // write echo message
      ReturnCode_t retcode = m_writer->write(message,
					     0);
      m_writer.check(retcode,
		     "latency_messageDataWriter::write");
    }

  m_reader->return_loan(m_reader_messages,
			m_reader_infos);

  return true;
}
