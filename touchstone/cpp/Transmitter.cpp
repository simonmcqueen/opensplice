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
#include "Transmitter.h"
#include "Partition.h"
#include "Processor.h"
/* ddshelp includes */
#include "DDSError.h"
/* standard includes */
#include <iostream>
#include <sstream>

/* Translate 0,1,2,...,26,27,28,... into 'A','B','C',...,'a','b','c' */
#define PAYLOAD_ITEM_FROM_UINT(i) \
    (char)(65+(((i/2)%26)+(i%2)*32))


Transmitter::Transmitter(Partition& partition)
  : m_partition(partition),
    m_qos_query(qos_reader()),
    m_topic(participant()),
    m_writer(publisher(),
	     m_topic),
    m_writer_thread(*this),
    m_active(false),
    m_config_number(0)
{
  // cout << "Transmitter::Transmitter(" << partition_id() << ")" << endl;
}

Transmitter::~Transmitter()
{
  // cout << "Transmitter::~Transmitter(" << partition_id() << ", " << m_def.transmitter_id << ")" << endl;
}

void
Transmitter::create(const transmitterDef& def)
{
  m_def = def;
  // cout << "Transmitter::create(" << partition_id() << ", " << m_def.transmitter_id << ")" << endl;

  // setup QueryCondition for reading this Transmitter's Qos
  stringstream id;
  id << m_def.transmitter_id;
  StringSeq params;
  params.length(1);
  params[0] = id.str().c_str();
  m_qos_query.create(ANY_SAMPLE_STATE,
		     ANY_VIEW_STATE,
		     ANY_INSTANCE_STATE,
		     "transmitter_id = %0",
		     params);

  // setup Transmitter topic
  set_topic();

  // read initial Qos
  transmitterQosSeq qoss;
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
      m_qos.transmitter_id = m_def.transmitter_id;
      m_qos.partition_id = m_def.partition_id;
      m_qos.qos.latency_budget.duration.sec = 0;
      m_qos.qos.latency_budget.duration.nanosec = 0;
      m_qos.qos.transport_priority.value = 0;

      retcode = qos_writer()->write(m_qos,
				    0);
      qos_writer().check(retcode,
			 "transmitterQosDataWriter::write");
    }
  else
    {
      qos_reader().check(retcode,
			 "transmitterQosDataReader::read_w_condition");

      assert(qoss.length() == 1);
      assert(infos.length() == 1);
      m_qos = qoss[0];
      assert(m_qos.group_id == m_def.group_id);
      assert(m_qos.transmitter_id == m_def.transmitter_id);
      assert(m_qos.partition_id == m_def.partition_id);
    }
  qos_reader()->return_loan(qoss,
			    infos);

  set_qos();

  // start writer thread
  m_active = true;
  m_writer_thread.create(&Transmitter::writer_thread);
}

void
Transmitter::dispose()
{
  // cout << "Transmitter::dispose(" << partition_id() << ", " << m_def.transmitter_id << ")" << endl;

  m_active = false;
  m_writer_thread.join();
}
  
void
Transmitter::update_def(const transmitterDef& def)
{
  // cout << "Transmitter::update_def(" << partition_id() << ", " << m_def.transmitter_id << ")" << endl;

  assert(m_def.transmitter_id == def.transmitter_id);
  assert(m_active);

  // check for changes requiring thread restart
  if (m_def.scheduling_class != def.scheduling_class ||
      m_def.thread_priority != def.thread_priority ||
      m_def.topic_kind != def.topic_kind ||
      m_def.topic_id != def.topic_id)
    {
      // stop writer thread
      m_active = false;
      m_writer_thread.join();

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
      
      // restart writer thread
      m_active = true;
      m_writer_thread.create(&Transmitter::writer_thread);
    }
  else
    {
      // update without stopping and restarting thread
      m_def = def;
      m_config_number++;
    }
}

void
Transmitter::update_qos(const transmitterQos& qos)
{
  // cout << "Transmitter::update_qos(" << partition_id() << ", " << m_def.transmitter_id << ")" << endl;

  assert(m_def.transmitter_id == qos.transmitter_id);
  assert(m_active);

  set_qos();

  if (m_writer)			// REVISIT - locking
    {
      ReturnCode_t retcode = m_writer->set_qos(m_writer_qos);
      m_writer.check(retcode,
		     "throughput_messageDataWriter::set_qos");
    }

  m_config_number++;
}

void
Transmitter::set_topic()
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
Transmitter::set_qos()
{
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

  m_writer_qos.latency_budget.duration.sec = m_qos.qos.latency_budget.duration.sec;
  m_writer_qos.latency_budget.duration.nanosec = m_qos.qos.latency_budget.duration.nanosec;
  m_writer_qos.transport_priority.value = m_qos.qos.transport_priority.value;
  /* For reliable transport, the transmitter writer
   * needs to set extra policies for its resource
   * limits, which avoids excessive memory growth
   * and invokes flow control if needed */
  if (m_writer_qos.reliability.kind == DDS::RELIABLE_RELIABILITY_QOS) {
        m_writer_qos.history.kind = DDS::KEEP_ALL_HISTORY_QOS;
        m_writer_qos.resource_limits.max_samples = 1;
  }

}

void
Transmitter::writer_thread()
{
  // cout << "Transmitter::writer_thread(" << partition_id() << ", " << m_def.transmitter_id << ")" << endl;

  // REVISIT - locking?

  try
    {
      // Freeze time
      m_creation_time = processor().get_timestamp();
      // and setup DataWriter
      m_writer.create(m_writer_qos);

      // setup message
      throughput_message message;
      message.application_id = processor().application_id();
      message.random_id = processor().random_id();
      message.transmitter_id = m_def.transmitter_id;
      message.creation_time = m_creation_time;
      message.creation_duration = processor().get_timestamp() - m_creation_time;
      message.sequence_number = 0;
      ULong last_size = 0;

      while (m_active)
	{
	  // adjust message size if needed
	  if (m_def.message_size != last_size)
	    {
	      ULong size = sizeof(throughput_message); // REVISIT - same as size in C?
	      // cout << "sizeof(throughput_message) = " << size << endl; // REVISIT - temporary
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

	  // write burst of messages
          for (unsigned int index = 0; index < m_def.messages_per_burst; index++) {
              message.instance_id = index;
	      ReturnCode_t retcode = m_writer->write(message,
						     0);
	      m_writer.check(retcode,
			 "throughput_messageDataWriter::write");
	      // cout << "Transmitter::writer_thread(" << partition_id() << ", " << m_def.transmitter_id
	      //      << ") wrote messsage with " << message.payload_data.length() << " byte payload" << endl;
	  }
	      

	  // sleep
	  Processor::msec_sleep(m_def.burst_period);
	}
    }
  catch (const DDSError& error)
    {
      processor().report_error(error,
			       partition_id(),
			       m_def.transmitter_id);
      cerr << "Transmitter writer thread exiting: " << error << endl;
    }

  m_writer.destroy();

  // cout << "Transmitter::writer_thread(" << partition_id() << ", " << m_def.transmitter_id << ") exiting" << endl;
}
