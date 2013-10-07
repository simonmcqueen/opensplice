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
#include "Processor.h"
#include "Partition.h"
/* ddshelp includes */
#include "DDSError.h"
/* OS abstraction includes */
#include "touchstone_os_abstraction.h"
/* standard includes */
#include <iostream>
#include <sstream>
#include <time.h>

Processor::Processor(const char* application_name,
		     ULong application_id,
		     ULong group_id,
		     const DomainId_t& domain_id)
  : m_application_name(application_name),
    m_application_id(application_id),
    m_group_id(group_id),
    m_domain_id(domain_id),
    /* m_random_id(0), */
    m_dispatcher(*this),
    m_participant(),

    // DDSTouchStoneReports partition
    m_report_publisher(m_participant),

    // errorReportTopic
    m_error_report_topic(m_participant),
    m_error_report_writer(m_report_publisher,
			  m_error_report_topic),

    // discoveryReportTopic
    m_discovery_report_topic(m_participant),
    m_discovery_report_writer(m_report_publisher,
			      m_discovery_report_topic),

    // receiverReportTopic
    m_receiver_report_topic(m_participant),
    m_receiver_report_writer(m_report_publisher,
			     m_receiver_report_topic),

    // transceiverReportTopic
    m_transceiver_report_topic(m_participant),
    m_transceiver_report_writer(m_report_publisher,
				m_transceiver_report_topic),

    // DDSTouchStoneCommands partition
    m_command_publisher(m_participant),
    m_command_subscriber(m_participant),

    // transmitterDefTopic
    m_transmitter_def_topic(m_participant),
    m_transmitter_def_filtered_topic(m_transmitter_def_topic),
    m_transmitter_def_reader(m_command_subscriber,
			     m_transmitter_def_filtered_topic),
    m_transmitter_def_condition(m_transmitter_def_reader),
    m_transmitter_def_attachment(m_dispatcher,
				 m_transmitter_def_condition),

    // transmitterQosTopic
    m_transmitter_qos_topic(m_participant),
    m_transmitter_qos_writer(m_command_publisher,
			     m_transmitter_qos_topic),
    m_transmitter_qos_filtered_topic(m_transmitter_qos_topic),
    m_transmitter_qos_reader(m_command_subscriber,
			     m_transmitter_qos_filtered_topic),
    m_transmitter_qos_condition(m_transmitter_qos_reader),
    m_transmitter_qos_attachment(m_dispatcher,
				 m_transmitter_qos_condition),

    // receiverDefTopic
    m_receiver_def_topic(m_participant),
    m_receiver_def_filtered_topic(m_receiver_def_topic),
    m_receiver_def_reader(m_command_subscriber,
			  m_receiver_def_filtered_topic),
    m_receiver_def_condition(m_receiver_def_reader),
    m_receiver_def_attachment(m_dispatcher,
			      m_receiver_def_condition),

    // receiverQosTopic
    m_receiver_qos_topic(m_participant),
    m_receiver_qos_writer(m_command_publisher,
			  m_receiver_qos_topic),
    m_receiver_qos_filtered_topic(m_receiver_qos_topic),
    m_receiver_qos_reader(m_command_subscriber,
			  m_receiver_qos_filtered_topic),
    m_receiver_qos_condition(m_receiver_qos_reader),
    m_receiver_qos_attachment(m_dispatcher,
			      m_receiver_qos_condition),

    // transceiverDefTopic
    m_transceiver_def_topic(m_participant),
    m_transceiver_def_filtered_topic(m_transceiver_def_topic),
    m_transceiver_def_reader(m_command_subscriber,
			     m_transceiver_def_filtered_topic),
    m_transceiver_def_condition(m_transceiver_def_reader),
    m_transceiver_def_attachment(m_dispatcher,
				 m_transceiver_def_condition),

    // transceiverQosTopic
    m_transceiver_qos_topic(m_participant),
    m_transceiver_qos_writer(m_command_publisher,
			     m_transceiver_qos_topic),
    m_transceiver_qos_filtered_topic(m_transceiver_qos_topic),
    m_transceiver_qos_reader(m_command_subscriber,
			     m_transceiver_qos_filtered_topic),
    m_transceiver_qos_condition(m_transceiver_qos_reader),
    m_transceiver_qos_attachment(m_dispatcher,
				 m_transceiver_qos_condition),

    // transponderDefTopic
    m_transponder_def_topic(m_participant),
    m_transponder_def_filtered_topic(m_transponder_def_topic),
    m_transponder_def_reader(m_command_subscriber,
			     m_transponder_def_filtered_topic),
    m_transponder_def_condition(m_transponder_def_reader),
    m_transponder_def_attachment(m_dispatcher,
				 m_transponder_def_condition),

    // transponderQosTopic
    m_transponder_qos_topic(m_participant),
    m_transponder_qos_writer(m_command_publisher,
			     m_transponder_qos_topic),
    m_transponder_qos_filtered_topic(m_transponder_qos_topic),
    m_transponder_qos_reader(m_command_subscriber,
			     m_transponder_qos_filtered_topic),
    m_transponder_qos_condition(m_transponder_qos_reader),
    m_transponder_qos_attachment(m_dispatcher,
				 m_transponder_qos_condition)
{
  // cout << "Processor::Processor()" << endl;
}

Processor::~Processor()
{
  // cout << "Processor::~Processor()" << endl;
}

void
Processor::run()
{
  // cout << "Processor::run()" << endl;

  try
    {
      // create DomainParticipant

      m_participant.create(m_domain_id);

      // set random ID

      Time_t time;
      m_participant->get_current_time(time);
      /* m_random_id = time.nanosec; */

      // create reliable+transient topic Qos

      TopicQos reliable_transient_topic_qos;
      m_participant->get_default_topic_qos(reliable_transient_topic_qos);
      reliable_transient_topic_qos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
      reliable_transient_topic_qos.durability.kind = DDS::TRANSIENT_DURABILITY_QOS;

      // create DDSTouchStoneReports publisher

      m_report_publisher.create_w_partition("DDSTouchStoneReports");

      // create report writer Qos

      DDS::DataWriterQos report_writer_qos;
      m_report_publisher->get_default_datawriter_qos(report_writer_qos);
      m_report_publisher->copy_from_topic_qos(report_writer_qos,
					      reliable_transient_topic_qos);

      // errorReportTopic

      m_error_report_topic.create("errorReportTopic",
				  reliable_transient_topic_qos);
      m_error_report_writer.create(report_writer_qos);

      // From this point on, errors are reported via the errorReportTopic.

      // discoveryReportTopic
      m_discovery_report_topic.create("discoveryReportTopic");
      m_discovery_report_writer.create(report_writer_qos);

      // receiverReportTopic
      m_receiver_report_topic.create("receiverReportTopic");
      m_receiver_report_writer.create(report_writer_qos);

      // transceiverReportTopic
      m_transceiver_report_topic.create("transceiverReportTopic");
      m_transceiver_report_writer.create(report_writer_qos);

      // DDSTouchStoneCommands partition
      m_command_publisher.create_w_partition("DDSTouchStoneCommands");
      m_command_subscriber.create_w_partition("DDSTouchStoneCommands");
      
      // command writer Qos
      DDS::DataWriterQos command_writer_qos;
      m_command_publisher->get_default_datawriter_qos(command_writer_qos);
      m_command_publisher->copy_from_topic_qos(command_writer_qos,
					       reliable_transient_topic_qos);

      // command reader Qos
      DDS::DataReaderQos command_reader_qos;
      m_command_subscriber->get_default_datareader_qos(command_reader_qos);
      m_command_subscriber->copy_from_topic_qos(command_reader_qos,
						reliable_transient_topic_qos);

      // filter key
      stringstream group_id_str;
      group_id_str << m_group_id;
      StringSeq group_id_seq;
      group_id_seq.length(1);
      group_id_seq[0] = group_id_str.str().c_str();

      // transmitterDefTopic
      m_transmitter_def_topic.create("transmitterDefTopic");
      m_transmitter_def_filtered_topic.create("transmitterDefFilteredTopic",
					      "group_id = %0",
					      group_id_seq);

      // transmitterDef reader
      m_transmitter_def_reader.create();
      m_transmitter_def_condition.create(NOT_READ_SAMPLE_STATE,
					 ANY_VIEW_STATE,
					 ANY_INSTANCE_STATE);
      m_transmitter_def_attachment.attach(&Processor::read_transmitter_def);

      // transmitterQosTopic
      m_transmitter_qos_topic.create("transmitterQosTopic",
				     reliable_transient_topic_qos);
      m_transmitter_qos_filtered_topic.create("transmitterQosFilteredTopic",
					      "group_id = %0",
					      group_id_seq);

      // transmitterQos writer
      m_transmitter_qos_writer.create(command_writer_qos);

      // transmitterQos reader
      m_transmitter_qos_reader.create(command_reader_qos);
      m_transmitter_qos_condition.create(NOT_READ_SAMPLE_STATE,
					 ANY_VIEW_STATE,
					 ANY_INSTANCE_STATE);
      m_transmitter_qos_attachment.attach(&Processor::read_transmitter_qos);

      // receiverDefTopic
      m_receiver_def_topic.create("receiverDefTopic");
      m_receiver_def_filtered_topic.create("receiverDefFilteredTopic",
					   "group_id = %0",
					   group_id_seq);

      // receiverDef reader
      m_receiver_def_reader.create();
      m_receiver_def_condition.create(NOT_READ_SAMPLE_STATE,
				      ANY_VIEW_STATE,
				      ANY_INSTANCE_STATE);
      m_receiver_def_attachment.attach(&Processor::read_receiver_def);

      // receiverQosTopic
      m_receiver_qos_topic.create("receiverQosTopic",
				  reliable_transient_topic_qos);
      m_receiver_qos_filtered_topic.create("receiverQosFilteredTopic",
					   "group_id = %0",
					   group_id_seq);

      // receiverQos writer
      m_receiver_qos_writer.create(command_writer_qos);

      // receiverQos reader
      m_receiver_qos_reader.create(command_reader_qos);
      m_receiver_qos_condition.create(NOT_READ_SAMPLE_STATE,
				      ANY_VIEW_STATE,
				      ANY_INSTANCE_STATE);
      m_receiver_qos_attachment.attach(&Processor::read_receiver_qos);

      // transceiverDefTopic
      m_transceiver_def_topic.create("transceiverDefTopic");
      m_transceiver_def_filtered_topic.create("transceiverDefFilteredTopic",
					      "group_id = %0",
					      group_id_seq);

      // transceiverDef reader
      m_transceiver_def_reader.create();
      m_transceiver_def_condition.create(NOT_READ_SAMPLE_STATE,
					 ANY_VIEW_STATE,
					 ANY_INSTANCE_STATE);
      m_transceiver_def_attachment.attach(&Processor::read_transceiver_def);

      // transceiverQosTopic
      m_transceiver_qos_topic.create("transceiverQosTopic",
				     reliable_transient_topic_qos);
      m_transceiver_qos_filtered_topic.create("transceiverQosFilteredTopic",
					      "group_id = %0",
					      group_id_seq);

      // transceiverQos writer
      m_transceiver_qos_writer.create(command_writer_qos);

      // transceiverQos reader
      m_transceiver_qos_reader.create(command_reader_qos);
      m_transceiver_qos_condition.create(NOT_READ_SAMPLE_STATE,
					 ANY_VIEW_STATE,
					 ANY_INSTANCE_STATE);
      m_transceiver_qos_attachment.attach(&Processor::read_transceiver_qos);

      // transponderDefTopic
      m_transponder_def_topic.create("transponderDefTopic");
      m_transponder_def_filtered_topic.create("transponderDefFilteredTopic",
					      "group_id = %0",
					      group_id_seq);

      // transponderDef reader
      m_transponder_def_reader.create();
      m_transponder_def_condition.create(NOT_READ_SAMPLE_STATE,
					 ANY_VIEW_STATE,
					 ANY_INSTANCE_STATE);
      m_transponder_def_attachment.attach(&Processor::read_transponder_def);

      // transponderQosTopic
      m_transponder_qos_topic.create("transponderQosTopic",
				     reliable_transient_topic_qos);
      m_transponder_qos_filtered_topic.create("transponderQosFilteredTopic",
					      "group_id = %0",
					      group_id_seq);

      // transponderQos writer
      m_transponder_qos_writer.create(command_writer_qos);

      // transponderQos reader
      m_transponder_qos_reader.create(command_reader_qos);
      m_transponder_qos_condition.create(NOT_READ_SAMPLE_STATE,
					 ANY_VIEW_STATE,
					 ANY_INSTANCE_STATE);
      m_transponder_qos_attachment.attach(&Processor::read_transponder_qos);

      // ...

      m_dispatcher.run();
    }
  catch (const DDSError& error)
    {
      report_error(error);
      cerr << "Exiting: " << error << endl;
      exit(1);
    }
}

void
Processor::report_error(const DDSError& error,
			ULong partition_id,
			ULong entity_id)
			      
{
  if (m_error_report_writer)
    {
      stringstream message;
      message << error << endl;

      errorReport report;
      report.application_id = m_application_id;
      report.partition_id = partition_id;
      report.entity_id = entity_id;
      report.message = message.str().c_str();
      ReturnCode_t retcode = m_error_report_writer->write(report,
							  0);
      m_error_report_writer.check(retcode,
				  "errorReportDataWriter::write");
    }
}

timestamp
Processor::get_timestamp()
{
  Time_t time;
  assert(m_participant);
  m_participant->get_current_time(time);
  return to_timestamp(time);
}

ULong
Processor::random_id()
{
  Time_t time;
  assert(m_participant);
  m_participant->get_current_time(time);
  return time.nanosec;
}

void
Processor::msec_sleep(msec_t milliseconds)
{
  touchstone_os_time delay = {milliseconds / 1000,
		 (milliseconds % 1000) * 1000000};
  touchstone_os_nanoSleep(delay);
}

bool
Processor::read_transmitter_def(Condition_ptr condition)
{
  // cout << "Processor::read_transmitter_def()" << endl;
  transmitterDefSeq defs;
  SampleInfoSeq infos;
  try
    {
      ReturnCode_t retcode = m_transmitter_def_reader->take(defs,
							    infos,
							    1,
							    ANY_SAMPLE_STATE,
							    ANY_VIEW_STATE,
							    ANY_INSTANCE_STATE);
      m_transmitter_def_reader.check(retcode,
				     "transmitterDefDataReader::take");
      
      ULong length = defs.length();
      assert(length == infos.length());
      for (ULong i = 0; i < length; i++)
	{
	  const transmitterDef& def(defs[i]);
	  const SampleInfo& info(infos[i]);

	  Partition* partition = 0;
	  PartitionMap::iterator iter(m_partitions.find(def.partition_id));
	  if (iter == m_partitions.end())
	    {
	      partition = new Partition(*this,
					def.partition_id);
	      m_partitions.insert(std::make_pair(def.partition_id,
					    partition));
	    }
	  else
	    {
	      partition = (*iter).second;
	    }
	  assert(partition);

	  if (info.instance_state == NOT_ALIVE_DISPOSED_INSTANCE_STATE ||
	      !info.valid_data)
	    {
	      partition->dispose_transmitter_def(def);
	      if (partition->empty())
		{
		  m_partitions.erase(def.partition_id);
		  delete partition;
		}
	    }
	  else
	    {
	      partition->process_transmitter_def(def);
	    }
	}
    }
  catch (const DDSError &error)
    {
      report_error(error);
      cerr << "Continuing: " << error << endl;
    }
  m_transmitter_def_reader->return_loan(defs,
					infos);

  return true;
}

bool
Processor::read_transmitter_qos(Condition_ptr condition)
{
  // cout << "Processor::read_transmitter_qos()" << endl;
  transmitterQosSeq qoss;
  SampleInfoSeq infos;
  try
    {
      // REVISIT - read or take?
      ReturnCode_t retcode = m_transmitter_qos_reader->read_w_condition(qoss,
									infos,
									1,
									m_transmitter_qos_condition);
      m_transmitter_qos_reader.check(retcode,
				     "transmitterQosDataReader::read_w_condition");
      
      ULong length = qoss.length();
      assert(length == infos.length());
      for (ULong i = 0; i < length; i++)
	{
	  const transmitterQos& qos(qoss[i]);
	  const SampleInfo& info(infos[i]);

	  PartitionMap::iterator iter(m_partitions.find(qos.partition_id));
	  if (iter != m_partitions.end())
	    {
	      Partition* partition = (*iter).second;
	      assert(partition);
	      partition->process_transmitter_qos(qos);
	    }
	}
    }
  catch (const DDSError &error)
    {
      report_error(error);
      cerr << "Continuing: " << error << endl;
    }
  m_transmitter_qos_reader->return_loan(qoss,
					infos);

  return true;
}

bool
Processor::read_receiver_def(Condition_ptr condition)
{
  // cout << "Processor::read_receiver_def()" << endl;
  receiverDefSeq defs;
  SampleInfoSeq infos;
  try
    {
      ReturnCode_t retcode = m_receiver_def_reader->take(defs,
							 infos,
							 1,
							 ANY_SAMPLE_STATE,
							 ANY_VIEW_STATE,
							 ANY_INSTANCE_STATE);
      m_receiver_def_reader.check(retcode,
				  "receiverDefDataReader::take");
      
      ULong length = defs.length();
      assert(length == infos.length());
      for (ULong i = 0; i < length; i++)
	{
	  const receiverDef& def(defs[i]);
	  const SampleInfo& info(infos[i]);

	  Partition* partition = 0;
	  PartitionMap::iterator iter(m_partitions.find(def.partition_id));
	  if (iter == m_partitions.end())
	    {
	      partition = new Partition(*this,
					def.partition_id);
	      m_partitions.insert(std::make_pair(def.partition_id,
					    partition));
	    }
	  else
	    {
	      partition = (*iter).second;
	    }
	  assert(partition);

	  if (info.instance_state == NOT_ALIVE_DISPOSED_INSTANCE_STATE ||
	      !info.valid_data)
	    {
	      partition->dispose_receiver_def(def);
	      if (partition->empty())
		{
		  m_partitions.erase(def.partition_id);
		  delete partition;
		}
	    }
	  else
	    {
	      partition->process_receiver_def(def);
	    }
	}
    }
  catch (const DDSError &error)
    {
      report_error(error);
      cerr << "Continuing: " << error << endl;
    }
  m_receiver_def_reader->return_loan(defs,
				     infos);

  return true;
}

bool
Processor::read_receiver_qos(Condition_ptr condition)
{
  // cout << "Processor::read_receiver_qos()" << endl;
  receiverQosSeq qoss;
  SampleInfoSeq infos;
  try
    {
      // REVISIT - read or take?
      ReturnCode_t retcode = m_receiver_qos_reader->read_w_condition(qoss,
								     infos,
								     1,
								     m_receiver_qos_condition);
      m_receiver_qos_reader.check(retcode,
				  "receiverQosDataReader::read_w_condition");
      
      ULong length = qoss.length();
      assert(length == infos.length());
      for (ULong i = 0; i < length; i++)
	{
	  const receiverQos& qos(qoss[i]);
	  const SampleInfo& info(infos[i]);

	  PartitionMap::iterator iter(m_partitions.find(qos.partition_id));
	  if (iter != m_partitions.end())
	    {
	      Partition* partition = (*iter).second;
	      assert(partition);
	      partition->process_receiver_qos(qos);
	    }
	}
    }
  catch (const DDSError &error)
    {
      report_error(error);
      cerr << "Continuing: " << error << endl;
    }
  m_receiver_qos_reader->return_loan(qoss,
				     infos);

  return true;
}

bool
Processor::read_transceiver_def(Condition_ptr condition)
{
  // cout << "Processor::read_transceiver_def()" << endl;
  transceiverDefSeq defs;
  SampleInfoSeq infos;
  try
    {
      ReturnCode_t retcode = m_transceiver_def_reader->take(defs,
							    infos,
							    1,
							    ANY_SAMPLE_STATE,
							    ANY_VIEW_STATE,
							    ANY_INSTANCE_STATE);
      m_transceiver_def_reader.check(retcode,
				     "transceiverDefDataReader::take");
      
      ULong length = defs.length();
      assert(length == infos.length());
      for (ULong i = 0; i < length; i++)
	{
	  const transceiverDef& def(defs[i]);
	  const SampleInfo& info(infos[i]);

	  Partition* partition = 0;
	  PartitionMap::iterator iter(m_partitions.find(def.partition_id));
	  if (iter == m_partitions.end())
	    {
	      partition = new Partition(*this,
					def.partition_id);
	      m_partitions.insert(std::make_pair(def.partition_id,
					    partition));
	    }
	  else
	    {
	      partition = (*iter).second;
	    }
	  assert(partition);

	  if (info.instance_state == NOT_ALIVE_DISPOSED_INSTANCE_STATE ||
	      !info.valid_data)
	    {
	      partition->dispose_transceiver_def(def);
	      if (partition->empty())
		{
		  m_partitions.erase(def.partition_id);
		  delete partition;
		}
	    }
	  else
	    {
	      partition->process_transceiver_def(def);
	    }
	}
    }
  catch (const DDSError &error)
    {
      report_error(error);
      cerr << "Continuing: " << error << endl;
    }
  m_transceiver_def_reader->return_loan(defs,
					infos);

  return true;
}

bool
Processor::read_transceiver_qos(Condition_ptr condition)
{
  // cout << "Processor::read_transceiver_qos()" << endl;
  transceiverQosSeq qoss;
  SampleInfoSeq infos;
  try
    {
      // REVISIT - read or take?
      ReturnCode_t retcode = m_transceiver_qos_reader->read_w_condition(qoss,
									infos,
									1,
									m_transceiver_qos_condition);
      m_transceiver_qos_reader.check(retcode,
				     "transceiverQosDataReader::read_w_condition");
      
      ULong length = qoss.length();
      assert(length == infos.length());
      for (ULong i = 0; i < length; i++)
	{
	  const transceiverQos& qos(qoss[i]);
	  const SampleInfo& info(infos[i]);

	  PartitionMap::iterator iter(m_partitions.find(qos.partition_id));
	  if (iter != m_partitions.end())
	    {
	      Partition* partition = (*iter).second;
	      assert(partition);
	      partition->process_transceiver_qos(qos);
	    }
	}
    }
  catch (const DDSError &error)
    {
      report_error(error);
      cerr << "Continuing: " << error << endl;
    }
  m_transceiver_qos_reader->return_loan(qoss,
					infos);

  return true;
}

bool
Processor::read_transponder_def(Condition_ptr condition)
{
  // cout << "Processor::read_transponder_def()" << endl;
  transponderDefSeq defs;
  SampleInfoSeq infos;
  try
    {
      ReturnCode_t retcode = m_transponder_def_reader->take(defs,
							    infos,
							    1,
							    ANY_SAMPLE_STATE,
							    ANY_VIEW_STATE,
							    ANY_INSTANCE_STATE);
      m_transponder_def_reader.check(retcode,
				     "transponderDefDataReader::take");
      
      ULong length = defs.length();
      assert(length == infos.length());
      for (ULong i = 0; i < length; i++)
	{
	  const transponderDef& def(defs[i]);
	  const SampleInfo& info(infos[i]);

	  Partition* partition = 0;
	  PartitionMap::iterator iter(m_partitions.find(def.partition_id));
	  if (iter == m_partitions.end())
	    {
	      partition = new Partition(*this,
					def.partition_id);
	      m_partitions.insert(std::make_pair(def.partition_id,
					    partition));
	    }
	  else
	    {
	      partition = (*iter).second;
	    }
	  assert(partition);

	  if (info.instance_state == NOT_ALIVE_DISPOSED_INSTANCE_STATE ||
	      !info.valid_data)
	    {
	      partition->dispose_transponder_def(def);
	      if (partition->empty())
		{
		  m_partitions.erase(def.partition_id);
		  delete partition;
		}
	    }
	  else
	    {
	      partition->process_transponder_def(def);
	    }
	}
    }
  catch (const DDSError &error)
    {
      report_error(error);
      cerr << "Continuing: " << error << endl;
    }
  m_transponder_def_reader->return_loan(defs,
					infos);

  return true;
}

bool
Processor::read_transponder_qos(Condition_ptr condition)
{
  // cout << "Processor::read_transponder_qos()" << endl;
  transponderQosSeq qoss;
  SampleInfoSeq infos;
  try
    {
      // REVISIT - read or take?
      ReturnCode_t retcode = m_transponder_qos_reader->read_w_condition(qoss,
									infos,
									1,
									m_transponder_qos_condition);
      m_transponder_qos_reader.check(retcode,
				     "transponderQosDataReader::read_w_condition");
      
      ULong length = qoss.length();
      assert(length == infos.length());
      for (ULong i = 0; i < length; i++)
	{
	  const transponderQos& qos(qoss[i]);
	  const SampleInfo& info(infos[i]);

	  PartitionMap::iterator iter(m_partitions.find(qos.partition_id));
	  if (iter != m_partitions.end())
	    {
	      Partition* partition = (*iter).second;
	      assert(partition);
	      partition->process_transponder_qos(qos);
	    }
	}
    }
  catch (const DDSError &error)
    {
      report_error(error);
      cerr << "Continuing: " << error << endl;
    }
  m_transponder_qos_reader->return_loan(qoss,
					infos);

  return true;
}
