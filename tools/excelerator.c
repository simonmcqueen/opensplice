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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>
#include <math.h>

#include "touchstone_vendor_abstraction_c.h"

#define ERROR(message) \
        printf("Excelerator line %d: %s\n", \
               __LINE__, message)

#define MAX_OUTPUT_LENGTH (250)

/* static char *DDSTouchStoneCommandPartition = "DDSTouchStoneCommands"; */ /* Not used */
static char *DDSTouchStoneReportPartition =  "DDSTouchStoneReports";

static DDS_DomainId_t               ExceleratorDomain      = DDS_DOMAIN_ID_DEFAULT;
static DDS_DomainParticipantFactory ExceleratorFactory     = DDS_OBJECT_NIL;
static DDS_DomainParticipant        ExceleratorParticipant = DDS_OBJECT_NIL;
static DDS_Subscriber               ExceleratorSubscriber  = DDS_OBJECT_NIL;

/* Report TypeSupport */
static DDSTouchStone_transmitterReportTypeSupport trs = DDS_OBJECT_NIL;
static DDSTouchStone_receiverReportTypeSupport    rrs = DDS_OBJECT_NIL;
static DDSTouchStone_transceiverReportTypeSupport srs = DDS_OBJECT_NIL;
static DDSTouchStone_transponderReportTypeSupport xrs = DDS_OBJECT_NIL;
static DDSTouchStone_discoveryReportTypeSupport   drs = DDS_OBJECT_NIL;
static DDSTouchStone_errorReportTypeSupport       ers = DDS_OBJECT_NIL;

/* Report Topics */
static DDS_Topic trt = DDS_OBJECT_NIL;
static DDS_Topic rrt = DDS_OBJECT_NIL;
static DDS_Topic srt = DDS_OBJECT_NIL;
static DDS_Topic xrt = DDS_OBJECT_NIL;
static DDS_Topic drt = DDS_OBJECT_NIL;
static DDS_Topic ert = DDS_OBJECT_NIL;

/* Report Readers */
static DDSTouchStone_transmitterReportDataReader trr = DDS_OBJECT_NIL;
static DDSTouchStone_receiverReportDataReader    rrr = DDS_OBJECT_NIL;
static DDSTouchStone_transceiverReportDataReader srr = DDS_OBJECT_NIL;
static DDSTouchStone_transponderReportDataReader xrr = DDS_OBJECT_NIL;
static DDSTouchStone_discoveryReportDataReader   drr = DDS_OBJECT_NIL;
static DDSTouchStone_errorReportDataReader       err = DDS_OBJECT_NIL;

static DDS_StatusCondition trrc = DDS_OBJECT_NIL;
static DDS_StatusCondition rrrc = DDS_OBJECT_NIL;
static DDS_StatusCondition srrc = DDS_OBJECT_NIL;
static DDS_StatusCondition xrrc = DDS_OBJECT_NIL;
static DDS_StatusCondition drrc = DDS_OBJECT_NIL;
static DDS_StatusCondition errc = DDS_OBJECT_NIL;

static int output_level = 0;
static char* output_file;
static FILE* file;

static char *application_name = NULL;
static int verbose_mode = FALSE;

static void open_file (char* filename)
{
  file = fopen(filename,"a+"); /* apend file (add text to a file or create a file if it does not exist.*/
}

static void write_output (char* output, int level)
{
  fprintf(file,"%s",output);

  if (level > 0)
  {
     fprintf(stdout, "%s", output);
  }
}

static DDSTouchStone_timestamp
get_timestamp_from_dds_time(
    DDS_Time_t *time)
{
    DDSTouchStone_timestamp timestamp;

    timestamp = time->sec * 1000000 +
                time->nanosec / 1000;
    return timestamp;
}

static void
report_reader_status (
    DDSTouchStone_readerStatus *status)
{
    char output[MAX_OUTPUT_LENGTH];
    snprintf(output, MAX_OUTPUT_LENGTH, "%d,%d,%d",
           status->samples_lost,
           status->samples_rejected,
           status->deadlines_missed);
    write_output (output, output_level);
}

static void
report_writer_status (
    DDSTouchStone_writerStatus *status)
{
    char output[MAX_OUTPUT_LENGTH];
    snprintf(output, MAX_OUTPUT_LENGTH,  "%d", status->deadlines_missed);
    write_output (output, output_level);
}

static void
report_metrics (
    DDSTouchStone_metricsReport *metrics)
{
    char output[MAX_OUTPUT_LENGTH];
    snprintf(output, MAX_OUTPUT_LENGTH,  "%3.1f%%,%d,%.0f,%.0f,%.0f,%.2f",
           metrics->percentile,
           metrics->sample_count,
           metrics->minimum,
           metrics->average,
           metrics->maximum,
           metrics->deviation);
    write_output (output, output_level);
}

static void
process_transmitterReport (
    DDSTouchStone_transmitterReport *report,
    DDS_Time_t *source_time,
    DDS_Time_t *arrival_time)
{
    char output[MAX_OUTPUT_LENGTH];
    snprintf(output, MAX_OUTPUT_LENGTH,  "Transmitter,%d,%d,%d,%.0f,%.0f",
           report->application_id,
           report->partition_id,
           report->transmitter_id,
           get_timestamp_from_dds_time(source_time),
           get_timestamp_from_dds_time(arrival_time));
    write_output (output, output_level);
    report_writer_status(&report->writer_status);
    write_output ("\n", output_level);
}

static void
process_receiverReport (
    DDSTouchStone_receiverReport *report,
    DDS_Time_t *source_time,
    DDS_Time_t *arrival_time)
{
    char output[MAX_OUTPUT_LENGTH];
    snprintf(output, MAX_OUTPUT_LENGTH,  "Receiver,%d,%d,%d,%.0f,%.0f,%d,%d,",
           report->application_id,
           report->partition_id,
           report->receiver_id,
           get_timestamp_from_dds_time(source_time),
           get_timestamp_from_dds_time(arrival_time),
           report->read_bytes_per_second,
           report->read_msgs_per_second);
    write_output (output, output_level);
    report_reader_status(&report->reader_status);
    write_output ("\n", output_level);
}

static void
process_transceiverReport (
    DDSTouchStone_transceiverReport *report,
    DDS_Time_t *source_time,
    DDS_Time_t *arrival_time)
{
    char output[MAX_OUTPUT_LENGTH];
    DDSTouchStone_timestamp st,at;

    st = get_timestamp_from_dds_time(source_time);
    at = get_timestamp_from_dds_time(arrival_time);


    if (verbose_mode)
    {

	    snprintf(output, MAX_OUTPUT_LENGTH,  "Transceiver Send End2End Latency,%d,%d,%d,%.0f,%.0f,",
		   report->application_id,
		   report->partition_id,
		   report->transceiver_id,
		   st,at);
	    write_output (output, output_level);

	    report_metrics(&report->send_latency[0]);
	    write_output (",", output_level);
	    report_metrics(&report->send_latency[1]);
	    write_output (",", output_level);
	    report_metrics(&report->send_latency[2]);
	    write_output (",", output_level);
	    report_metrics(&report->send_latency[3]);
	    write_output ("\n", output_level);

	    snprintf(output, MAX_OUTPUT_LENGTH,  "Transceiver Send Source Latency,%d,%d,%d,%.0f,%.0f,",
		   report->application_id,
		   report->partition_id,
		   report->transceiver_id,
		   st,at);
	    write_output (output, output_level);

	    report_metrics(&report->send_source_latency[0]);
	    write_output (",", output_level);
	    report_metrics(&report->send_source_latency[1]);
	    write_output (",", output_level);
	    report_metrics(&report->send_source_latency[2]);
	    write_output (",", output_level);
	    report_metrics(&report->send_source_latency[3]);
	    write_output ("\n", output_level);

	    snprintf(output, MAX_OUTPUT_LENGTH,  "Transceiver Send Arrival Latency,%d,%d,%d,%.0f,%.0f,",
		   report->application_id,
		   report->partition_id,
		   report->transceiver_id,
		   st,at);
	    write_output (output, output_level);

	    report_metrics(&report->send_arrival_latency[0]);
	    write_output (",", output_level);
	    report_metrics(&report->send_arrival_latency[1]);
	    write_output (",", output_level);
	    report_metrics(&report->send_arrival_latency[2]);
	    write_output (",", output_level);
	    report_metrics(&report->send_arrival_latency[3]);
	    write_output ("\n", output_level);

	    snprintf(output, MAX_OUTPUT_LENGTH,  "Transceiver Echo End2End Latency,%d,%d,%d,%.0f,%.0f,",
		   report->application_id,
		   report->partition_id,
		   report->transceiver_id,
		   st,at);
	    write_output (output, output_level);

	    report_metrics(&report->echo_latency[0]);
	    write_output (",", output_level);
	    report_metrics(&report->echo_latency[1]);
	    write_output (",", output_level);
	    report_metrics(&report->echo_latency[2]);
	    write_output (",", output_level);
	    report_metrics(&report->echo_latency[3]);
	    write_output ("\n", output_level);

	    snprintf(output, MAX_OUTPUT_LENGTH,  "Transceiver Echo Source Latency,%d,%d,%d,%.0f,%.0f,",
		   report->application_id,
		   report->partition_id,
		   report->transceiver_id,
		   st,at);
	    write_output (output, output_level);

	    report_metrics(&report->echo_source_latency[0]);
	    write_output (",", output_level);
	    report_metrics(&report->echo_source_latency[1]);
	    write_output (",", output_level);
	    report_metrics(&report->echo_source_latency[2]);
	    write_output (",", output_level);
	    report_metrics(&report->echo_source_latency[3]);
	    write_output ("\n", output_level);

	    snprintf(output, MAX_OUTPUT_LENGTH,  "Transceiver Echo Arrival Latency,%d,%d,%d,%.0f,%.0f,",
		   report->application_id,
		   report->partition_id,
		   report->transceiver_id,
		   st,at);
	    write_output (output, output_level);

	    report_metrics(&report->echo_arrival_latency[0]);
	    write_output (",", output_level);
	    report_metrics(&report->echo_arrival_latency[1]);
	    write_output (",", output_level);
	    report_metrics(&report->echo_arrival_latency[2]);
	    write_output (",", output_level);
	    report_metrics(&report->echo_arrival_latency[3]);
	    write_output ("\n", output_level);
    }

    snprintf(output, MAX_OUTPUT_LENGTH,  "Transceiver Trip Latency,%d,%d,%d,%.0f,%.0f,",
           report->application_id,
           report->partition_id,
           report->transceiver_id,
           st,at);
    write_output (output, output_level);

    report_metrics(&report->trip_latency[0]);
    write_output (",", output_level);
    report_metrics(&report->trip_latency[1]);
    write_output (",", output_level);
    report_metrics(&report->trip_latency[2]);
    write_output (",", output_level);
    report_metrics(&report->trip_latency[3]);
    write_output ("\n", output_level);

    snprintf(output, MAX_OUTPUT_LENGTH,  "Transceiver Inter Arival Time,%d,%d,%d,%.0f,%.0f,",
           report->application_id,
           report->partition_id,
           report->transceiver_id,
           st,at);
    write_output (output, output_level);

    report_metrics(&report->inter_arrival_time[0]);
    write_output (",", output_level);
    report_metrics(&report->inter_arrival_time[1]);
    write_output (",", output_level);
    report_metrics(&report->inter_arrival_time[2]);
    write_output (",", output_level);
    report_metrics(&report->inter_arrival_time[3]);
    write_output ("\n", output_level);

    snprintf(output, MAX_OUTPUT_LENGTH,  "Transceiver status,%d,%d,%d,%.0f,%.0f,",
           report->application_id,
           report->partition_id,
           report->transceiver_id,
           st,at);
    write_output (output, output_level);

    report_writer_status(&report->writer_status);
    report_reader_status(&report->reader_status);
    write_output ("\n", output_level);
}

static void
process_transponderReport (
    DDSTouchStone_transponderReport *report,
    DDS_Time_t *source_time,
    DDS_Time_t *arrival_time)
{
    char output[MAX_OUTPUT_LENGTH];
    snprintf(output, MAX_OUTPUT_LENGTH,  "Transponder,%d,%d,%d,%.0f,%.0f,",
           report->application_id,
           report->partition_id,
           report->transponder_id,
           get_timestamp_from_dds_time(source_time),
           get_timestamp_from_dds_time(arrival_time));
    write_output (output, output_level);

    report_writer_status(&report->writer_status);
    report_reader_status(&report->reader_status);
    write_output ("\n", output_level);
}

static void
process_errorReport (
    DDSTouchStone_errorReport *report,
    DDS_Time_t *source_time,
    DDS_Time_t *arrival_time)
{
    char output[MAX_OUTPUT_LENGTH];
    snprintf(output, MAX_OUTPUT_LENGTH,  "Error,%d,%d,%d,%.0f,%.0f,%s\n",
           report->application_id,
           report->partition_id,
           report->entity_id,
           get_timestamp_from_dds_time(source_time),
           get_timestamp_from_dds_time(arrival_time),
           report->message);
    write_output (output, output_level);
}

static void
process_discoveryReport (
    DDSTouchStone_discoveryReport *report,
    DDS_Time_t *source_time,
    DDS_Time_t *arrival_time)
{
    char output[MAX_OUTPUT_LENGTH];
    switch (report->report_kind) {
    case DDSTouchStone_DataWriterDiscovery:
        snprintf(output, MAX_OUTPUT_LENGTH,  "Discovery DataWriter,%d,%d,%d,%.0f,%.0f,%.0f,%.0f,%d\n",
               report->application_id,
               report->partition_id,
               report->entity_id,
               get_timestamp_from_dds_time(source_time),
               get_timestamp_from_dds_time(arrival_time),
               report->creation_duration,
               report->discovery_time,
               report->samples_missed);
        write_output (output, output_level);
    break;
    case DDSTouchStone_DataReaderDiscovery:
        snprintf(output, MAX_OUTPUT_LENGTH,  "Discovery DataReader,%d,%d,%d,%.0f,%.0f,%.0f,%.0f\n",
               report->application_id,
               report->partition_id,
               report->entity_id,
               get_timestamp_from_dds_time(source_time),
               get_timestamp_from_dds_time(arrival_time),
               report->creation_duration,
               report->discovery_time);
        write_output (output, output_level);
    break;
    }
}

static DDS_boolean
process_command (
    DDS_StatusCondition condition)
{
    DDS_sequence_DDSTouchStone_transmitterReport trl = {0,0,DDS_OBJECT_NIL,FALSE};
    DDS_sequence_DDSTouchStone_receiverReport    rrl = {0,0,DDS_OBJECT_NIL,FALSE};
    DDS_sequence_DDSTouchStone_transceiverReport srl = {0,0,DDS_OBJECT_NIL,FALSE};
    DDS_sequence_DDSTouchStone_transponderReport xrl = {0,0,DDS_OBJECT_NIL,FALSE};
    DDS_sequence_DDSTouchStone_discoveryReport   drl = {0,0,DDS_OBJECT_NIL,FALSE};
    DDS_sequence_DDSTouchStone_errorReport       erl = {0,0,DDS_OBJECT_NIL,FALSE};
    DDS_SampleInfoSeq infoList = { 0, 0, NULL, FALSE };
    DDS_ReturnCode_t  result;
    int length, i;

    if (condition == trrc) {
        result = DDSTouchStone_transmitterReportDataReader_read_w_condition (
                     trr, &trl, &infoList, 1, trrc);
        if (result == DDS_RETCODE_OK) {
            length = trl._length;
            if (length != 0) {
                for (i=0; i<length; i++) {
                    process_transmitterReport(
                        &trl._buffer[i],
                        &infoList._buffer[i].source_timestamp,
                        &infoList._buffer[i].reception_timestamp);
                }
                result = DDSTouchStone_transmitterReportDataReader_return_loan (
                             trr, &trl, &infoList);
            }
        }
    } else if (condition == rrrc) {
        result = DDSTouchStone_receiverReportDataReader_read_w_condition (
                     rrr, &rrl, &infoList, 1, rrrc);
        if (result == DDS_RETCODE_OK) {
            length = rrl._length;
            if (length != 0) {
                for (i=0; i<length; i++) {
                    process_receiverReport(
                        &rrl._buffer[i],
                        &infoList._buffer[i].source_timestamp,
                        &infoList._buffer[i].reception_timestamp);
                }
                result = DDSTouchStone_receiverReportDataReader_return_loan (
                             rrr, &rrl, &infoList);
            }
        }
    } else if (condition == srrc) {
        result = DDSTouchStone_transceiverReportDataReader_read_w_condition (
                     srr, &srl, &infoList, 1, srrc);
        if (result == DDS_RETCODE_OK) {
            length = srl._length;
            if (length != 0) {
                for (i=0; i<length; i++) {
                    process_transceiverReport(
                        &srl._buffer[i],
                        &infoList._buffer[i].source_timestamp,
                        &infoList._buffer[i].reception_timestamp);
                }
                result = DDSTouchStone_transceiverReportDataReader_return_loan (
                             srr, &srl, &infoList);
            }
        }
    } else if (condition == xrrc) {
        result = DDSTouchStone_transponderReportDataReader_read_w_condition (
                     xrr, &xrl, &infoList, 1, xrrc);
        if (result == DDS_RETCODE_OK) {
            length = xrl._length;
            if (length != 0) {
                for (i=0; i<length; i++) {
                    process_transponderReport(
                        &xrl._buffer[i],
                        &infoList._buffer[i].source_timestamp,
                        &infoList._buffer[i].reception_timestamp);
                }
                result = DDSTouchStone_transponderReportDataReader_return_loan (
                             xrr, &xrl, &infoList);
            }
        }
    } else if (condition == drrc) {
        result = DDSTouchStone_discoveryReportDataReader_read_w_condition (
                     drr, &drl, &infoList, 1, drrc);
        if (result == DDS_RETCODE_OK) {
            length = drl._length;
            if (length != 0) {
                for (i=0; i<length; i++) {
                    process_discoveryReport(
                        &drl._buffer[i],
                        &infoList._buffer[i].source_timestamp,
                        &infoList._buffer[i].reception_timestamp);
                }
                result = DDSTouchStone_discoveryReportDataReader_return_loan (
                             drr, &drl, &infoList);
            }
        }
    } else if (condition == errc) {
        result = DDSTouchStone_errorReportDataReader_read_w_condition (
                     err, &erl, &infoList, 1, errc);
        if (result == DDS_RETCODE_OK) {
            length = erl._length;
            if (length != 0) {
                for (i=0; i<length; i++) {
                    process_errorReport(
                        &erl._buffer[i],
                        &infoList._buffer[i].source_timestamp,
                        &infoList._buffer[i].reception_timestamp);
                }
                result = DDSTouchStone_errorReportDataReader_return_loan (
                             err, &erl, &infoList);
            }
        }
    } else {
        ERROR("Waitset Returned an unknown condition");
    }
    return TRUE;
}

static void
printUsage(
    char *name)
{
    printf("Usage: %s [-d] <file_name>\n",name);
}

static void
readCommandLineParams(
    int argc,
    char *argv[])
{
    int currArg = 0;

    application_name = argv[currArg];
    currArg++;

    /* First read options */
#define _PEQUALS_(str) (strcmp(argv[currArg], str) == 0)
    while ((currArg < argc) && ((argv[currArg])[0] == '-')) {
        if _PEQUALS_("-d") {
            output_level = 1;
            currArg++;
	    printf ("Switched on display to screen \n");

        } else if _PEQUALS_("-v") {
            verbose_mode = TRUE;
            currArg++;
	    printf ("Switched on verbose mode\n");
        }
    }
#undef _PEQUALS_

    /* Then read arguments */

    if (argv[currArg] == NULL)
    {
      printf("No output file specified\n");
      printf("Exiting %s\n", application_name);
      exit (-1);
    }
    else
    {
      output_file = argv[currArg];
      printf("Using file: %s\n", output_file);
      printf("Starting %s\n", application_name);
    }
}

int
main (
    int argc,
    char *argv[])
{

    DDS_WaitSet ExceleratorWaitset;
    DDS_Duration_t ExceleratorTimeout = DDS_DURATION_INFINITE;
    DDS_ReturnCode_t result;
    DDS_ConditionSeq *conditionList;
    DDS_StringSeq ExceleratorId;
    DDS_SubscriberQos *sQos;
    DDS_TopicQos *tQos;
    DDS_DataReaderQos *rQos;
    int length, i, proceed;
    char input;

    ExceleratorId._maximum   = 1;
    ExceleratorId._length    = 1;
    ExceleratorId._buffer    = DDS_StringSeq_allocbuf(1);
    ExceleratorId._buffer[0] = DDS_string_dup(argv[1]);

    /*
     * Parse input paramenters
     */
    readCommandLineParams(argc, argv);
    open_file(output_file);

    /*
     * Create WaitSet
     */
    ExceleratorWaitset = DDS_WaitSet__alloc ();

    /*
     * Create participant
     */
    ExceleratorFactory = DDS_DomainParticipantFactory_get_instance ();

    if (ExceleratorFactory == DDS_HANDLE_NIL) {
        printf ("Excelerator %s: ERROR - missing factoy instance\n",
                argv[0]);
        exit (1);
    }

    ExceleratorParticipant = DDS_DomainParticipantFactory_create_participant (
                             ExceleratorFactory,
                             ExceleratorDomain,
                             DDS_PARTICIPANT_QOS_DEFAULT,
                             NULL,
                             DDS_ANY_STATUS);

    if (ExceleratorParticipant == DDS_HANDLE_NIL) {
        printf ("Excelerator %s: ERROR - OpenSplice not running\n",
                argv[0]);
        exit (1);
    }

    sQos = DDS_SubscriberQos__alloc();

    DDS_DomainParticipant_get_default_subscriber_qos (ExceleratorParticipant,
                                                      sQos);

    sQos->partition.name._length = 1;
    sQos->partition.name._maximum = 1;
    sQos->partition.name._buffer = DDS_StringSeq_allocbuf (1);
    sQos->partition.name._buffer[0] = DDS_string_dup(DDSTouchStoneReportPartition);

    ExceleratorSubscriber = DDS_DomainParticipant_create_subscriber (
                             ExceleratorParticipant,
                             sQos,
                             NULL,
                             DDS_ANY_STATUS);

    DDS_free(sQos);

    /*
     * TransmitterReport
     */

    /*  Create Topic */
    trs = DDSTouchStone_transmitterReportTypeSupport__alloc ();

    DDSTouchStone_transmitterReportTypeSupport_register_type (
              trs,
              ExceleratorParticipant,
              "DDSTouchStone::transmitterReport");

    trt = DDS_DomainParticipant_create_topic (
              ExceleratorParticipant,
              "transmitterReportTopic",
              "DDSTouchStone::transmitterReport",
              DDS_TOPIC_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Create datareader */
    trr = DDS_Subscriber_create_datareader (
              ExceleratorSubscriber,
              trt,
              DDS_DATAREADER_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Add datareader readcondition to waitset */

    trrc = DDS_DataReader_create_readcondition (
              trr,
              DDS_NOT_READ_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    result = DDS_WaitSet_attach_condition (ExceleratorWaitset, trrc);

    /*
     * ReceiverReport
     */

    /*  Create Topic */
    rrs = DDSTouchStone_receiverReportTypeSupport__alloc ();

    DDSTouchStone_receiverReportTypeSupport_register_type (
              rrs,
              ExceleratorParticipant,
              "DDSTouchStone::receiverReport");

    rrt = DDS_DomainParticipant_create_topic (
              ExceleratorParticipant,
              "receiverReportTopic",
              "DDSTouchStone::receiverReport",
              DDS_TOPIC_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Create datareader */
    rrr = DDS_Subscriber_create_datareader (
              ExceleratorSubscriber,
              rrt,
              DDS_DATAREADER_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Add datareader readcondition to waitset */

    rrrc = DDS_DataReader_create_readcondition (
              rrr,
              DDS_NOT_READ_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    result = DDS_WaitSet_attach_condition (ExceleratorWaitset, rrrc);

    /*
     * transceiverReport
     */

    /*  Create Topic */
    srs = DDSTouchStone_transceiverReportTypeSupport__alloc ();

    DDSTouchStone_transceiverReportTypeSupport_register_type (
              srs, ExceleratorParticipant,
              "DDSTouchStone::transceiverReport");

    srt = DDS_DomainParticipant_create_topic (
              ExceleratorParticipant,
              "transceiverReportTopic",
              "DDSTouchStone::transceiverReport",
              DDS_TOPIC_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Create datareader */
    srr = DDS_Subscriber_create_datareader (
              ExceleratorSubscriber,
              srt,
              DDS_DATAREADER_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Add datareader readcondition to waitset */

    srrc = DDS_DataReader_create_readcondition (
              srr,
              DDS_NOT_READ_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    result = DDS_WaitSet_attach_condition (ExceleratorWaitset, srrc);

    /*
     * transponderReport
     */

    /*  Create Topic */
    xrs = DDSTouchStone_transponderReportTypeSupport__alloc ();

    DDSTouchStone_transponderReportTypeSupport_register_type (
              xrs, ExceleratorParticipant,
              "DDSTouchStone::transponderReport");

    xrt = DDS_DomainParticipant_create_topic (
              ExceleratorParticipant,
              "transponderReportTopic",
              "DDSTouchStone::transponderReport",
              DDS_TOPIC_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Create datareader */
    xrr = DDS_Subscriber_create_datareader (
              ExceleratorSubscriber,
              xrt,
              DDS_DATAREADER_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Add datareader readcondition to waitset */

    xrrc = DDS_DataReader_create_readcondition (
              xrr,
              DDS_NOT_READ_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    result = DDS_WaitSet_attach_condition (ExceleratorWaitset, xrrc);

    /*  Create Topic */
    drs = DDSTouchStone_discoveryReportTypeSupport__alloc ();

    DDSTouchStone_discoveryReportTypeSupport_register_type (
              drs,
              ExceleratorParticipant,
              "DDSTouchStone::discoveryReport");

    drt = DDS_DomainParticipant_create_topic (
              ExceleratorParticipant,
              "discoveryReportTopic",
              "DDSTouchStone::discoveryReport",
              DDS_TOPIC_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Create datareader */
    drr = DDS_Subscriber_create_datareader (
              ExceleratorSubscriber,
              drt,
              DDS_DATAREADER_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Add datareader readcondition to waitset */

    drrc = DDS_DataReader_create_readcondition (
              drr,
              DDS_NOT_READ_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    result = DDS_WaitSet_attach_condition (ExceleratorWaitset, drrc);

    /*  Create Topic */
    ers = DDSTouchStone_errorReportTypeSupport__alloc ();

    DDSTouchStone_errorReportTypeSupport_register_type (
              ers, ExceleratorParticipant,
              "DDSTouchStone::errorReport");

    tQos = DDS_TopicQos__alloc();

    DDS_DomainParticipant_get_default_topic_qos(ExceleratorParticipant, tQos);
    tQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    tQos->durability.kind = DDS_TRANSIENT_DURABILITY_QOS;

    ert = DDS_DomainParticipant_create_topic (
              ExceleratorParticipant,
              "errorReportTopic",
              "DDSTouchStone::errorReport",
              tQos,
              NULL,
              DDS_ANY_STATUS);

    rQos = DDS_DataReaderQos__alloc();

    DDS_Subscriber_copy_from_topic_qos(ExceleratorSubscriber, rQos, tQos);

    /* Create datareader */
    err = DDS_Subscriber_create_datareader (
              ExceleratorSubscriber,
              ert,
              rQos,
              NULL,
              DDS_ANY_STATUS);

    DDS_free(rQos);
    DDS_free(tQos);

    /* Add datareader readcondition to waitset */

    errc = DDS_DataReader_create_readcondition (
              err,
              DDS_NOT_READ_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    result = DDS_WaitSet_attach_condition (ExceleratorWaitset, errc);

    /*
     * MainLoop processing
     */
    proceed = TRUE;
    while (proceed) {
        conditionList = DDS_ConditionSeq__alloc();
        result = DDS_WaitSet_wait (ExceleratorWaitset,
                                   conditionList,
                                   &ExceleratorTimeout);
        length = conditionList->_length;
        for (i = 0; (i < length) && proceed; i++) {
            proceed = process_command(conditionList->_buffer[i]);
        }

    }
    return 0;
}

