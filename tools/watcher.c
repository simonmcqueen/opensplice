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
        printf("Watcher line %d: %s\n", \
               __LINE__, message)

/* static char *DDSTouchStoneCommandPartition = "DDSTouchStoneCommands"; */ /* Not used */
static char *DDSTouchStoneReportPartition =  "DDSTouchStoneReports";

static DDS_DomainId_t               WatcherDomain      = DDS_DOMAIN_ID_DEFAULT;
static DDS_DomainParticipantFactory WatcherFactory     = DDS_OBJECT_NIL;
static DDS_DomainParticipant        WatcherParticipant = DDS_OBJECT_NIL;
static DDS_Subscriber               WatcherSubscriber  = DDS_OBJECT_NIL;

/* Report TypeSupport */
static DDSTouchStone_transmitterReportTypeSupport trs = DDS_OBJECT_NIL;
static DDSTouchStone_receiverReportTypeSupport    rrs = DDS_OBJECT_NIL;
static DDSTouchStone_transceiverReportTypeSupport srs = DDS_OBJECT_NIL;
static DDSTouchStone_transponderReportTypeSupport xrs = DDS_OBJECT_NIL;

/* Report Topics */
static DDS_Topic trt = DDS_OBJECT_NIL;
static DDS_Topic rrt = DDS_OBJECT_NIL;
static DDS_Topic srt = DDS_OBJECT_NIL;
static DDS_Topic xrt = DDS_OBJECT_NIL;

/* Report Readers */
static DDSTouchStone_transmitterReportDataReader trr = DDS_OBJECT_NIL;
static DDSTouchStone_receiverReportDataReader    rrr = DDS_OBJECT_NIL;
static DDSTouchStone_transceiverReportDataReader srr = DDS_OBJECT_NIL;
static DDSTouchStone_transponderReportDataReader xrr = DDS_OBJECT_NIL;

static DDS_StatusCondition trrc = DDS_OBJECT_NIL;
static DDS_StatusCondition rrrc = DDS_OBJECT_NIL;
static DDS_StatusCondition srrc = DDS_OBJECT_NIL;
static DDS_StatusCondition xrrc = DDS_OBJECT_NIL;

static char *application_name = NULL;
static int verbose_mode = FALSE;

static void
report_reader_status (
    DDSTouchStone_readerStatus *status)
{
    if (status->samples_lost > 0) {
        printf("    Lost %d Samples\n", status->samples_lost);
    }
    if (status->samples_rejected > 0) {
        printf("    Rejected %d Samples\n", status->samples_rejected);
    }
    if (status->deadlines_missed > 0) {
        printf("    Missed %d Deadlines\n", status->deadlines_missed);
    }
}

static void
report_writer_status (
    DDSTouchStone_writerStatus *status)
{
    if (status->deadlines_missed > 0) {
        printf("    Missed %d Deadlines\n", status->deadlines_missed);
    }
}

static void
report_metrics (
    DDSTouchStone_metricsReport *metrics)
{
    printf("    %3.1f%% : cnt= %d, "
           "min= %.0f, avg= %.0f, max= %.0f, dev= %.2f\n",
           metrics->percentile,
           metrics->sample_count,
           metrics->minimum,
           metrics->average,
           metrics->maximum,
           metrics->deviation);
}

static void
process_transmitterReport (
    DDSTouchStone_transmitterReport *report)
{
    printf("Received Transmitter report from (%d,%d,%d)\n",
           report->application_id,
           report->partition_id,
           report->transmitter_id);
    report_writer_status(&report->writer_status);
}

static void
process_receiverReport (
    DDSTouchStone_receiverReport *report)
{
    printf("Received receiver report from (%d,%d,%d)\n",
           report->application_id,
           report->partition_id,
           report->receiver_id);
    printf("    Read       = %d bytes/sec\n",report->read_bytes_per_second);
    printf("    Read       = %d samples/sec\n",report->read_msgs_per_second);
/*
    if (report->read < report->troughput) {
        printf("    %d bytes/sec overwritten (KEEP_LAST policy)\n",
               report->troughput - report->read);
    }
*/
    report_reader_status(&report->reader_status);
    printf("\n");
}

static void
process_transceiverReport (
    DDSTouchStone_transceiverReport *report)
{
    printf("Received Transceiver report from (%d,%d,%d)\n",
           report->application_id,
           report->partition_id,
           report->transceiver_id);

    if (verbose_mode)
    {
	    printf("    Send latency: \n");
	    report_metrics(&report->send_latency[0]);
	    report_metrics(&report->send_latency[1]);
	    report_metrics(&report->send_latency[2]);
	    report_metrics(&report->send_latency[3]);
	    printf("    Send Source latency: \n");
	    report_metrics(&report->send_source_latency[0]);
	    report_metrics(&report->send_source_latency[1]);
	    report_metrics(&report->send_source_latency[2]);
	    report_metrics(&report->send_source_latency[3]);
	    printf("    Send Delivery latency: \n");
	    report_metrics(&report->send_trip_latency[0]);
	    report_metrics(&report->send_trip_latency[1]);
	    report_metrics(&report->send_trip_latency[2]);
	    report_metrics(&report->send_trip_latency[3]);
	    printf("    Send Arrival latency: \n");
	    report_metrics(&report->send_arrival_latency[0]);
	    report_metrics(&report->send_arrival_latency[1]);
	    report_metrics(&report->send_arrival_latency[2]);
	    report_metrics(&report->send_arrival_latency[3]);
	    printf("    Echo latency: \n");
	    report_metrics(&report->echo_latency[0]);
	    report_metrics(&report->echo_latency[1]);
	    report_metrics(&report->echo_latency[2]);
	    report_metrics(&report->echo_latency[3]);
	    printf("    Echo Source latency: \n");
	    report_metrics(&report->echo_source_latency[0]);
	    report_metrics(&report->echo_source_latency[1]);
	    report_metrics(&report->echo_source_latency[2]);
	    report_metrics(&report->echo_source_latency[3]);
	    printf("    Echo Delivery latency: \n");
	    report_metrics(&report->echo_trip_latency[0]);
	    report_metrics(&report->echo_trip_latency[1]);
	    report_metrics(&report->echo_trip_latency[2]);
	    report_metrics(&report->echo_trip_latency[3]);
	    printf("    Echo Arrival latency: \n");
	    report_metrics(&report->echo_arrival_latency[0]);
	    report_metrics(&report->echo_arrival_latency[1]);
	    report_metrics(&report->echo_arrival_latency[2]);
	    report_metrics(&report->echo_arrival_latency[3]);
	}
	    printf("    Trip latency: \n");
	    report_metrics(&report->trip_latency[0]);
	    report_metrics(&report->trip_latency[1]);
	    report_metrics(&report->trip_latency[2]);
	    report_metrics(&report->trip_latency[3]);
	    printf("    Inter arrival time: \n");
	    report_metrics(&report->inter_arrival_time[0]);
	    report_metrics(&report->inter_arrival_time[1]);
	    report_metrics(&report->inter_arrival_time[2]);
	    report_metrics(&report->inter_arrival_time[3]);
	    report_writer_status(&report->writer_status);
	    report_reader_status(&report->reader_status);

}

static void
process_transponderReport (
    DDSTouchStone_transponderReport *report)
{
    printf("Received Transponder report from (%d,%d,%d)\n",
           report->application_id,
           report->partition_id,
           report->transponder_id);
    report_writer_status(&report->writer_status);
    report_reader_status(&report->reader_status);
}

static DDS_boolean
process_command (
    DDS_StatusCondition condition)
{
    DDS_sequence_DDSTouchStone_transmitterReport trl = {0,0,DDS_OBJECT_NIL,FALSE};
    DDS_sequence_DDSTouchStone_receiverReport    rrl = {0,0,DDS_OBJECT_NIL,FALSE};
    DDS_sequence_DDSTouchStone_transceiverReport srl = {0,0,DDS_OBJECT_NIL,FALSE};
    DDS_sequence_DDSTouchStone_transponderReport xrl = {0,0,DDS_OBJECT_NIL,FALSE};
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
                    process_transmitterReport(&trl._buffer[i]);
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
                    process_receiverReport(&rrl._buffer[i]);
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
                    process_transceiverReport(&srl._buffer[i]);
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
                    process_transponderReport(&xrl._buffer[i]);
                }
                result = DDSTouchStone_transponderReportDataReader_return_loan (
                             xrr, &xrl, &infoList);
            }
        }
    } else {
        ERROR("Waitset Returned an unknown condition");
    }
    return TRUE;
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
        if _PEQUALS_("-v") {
            verbose_mode = TRUE;
            currArg++;
        }
    }
#undef _PEQUALS_

    if (verbose_mode) {
        printf("Switched on verbose mode\n");
    }
}


int
main (
    int argc,
    char *argv[])
{
    DDS_WaitSet WatcherWaitset;
    DDS_Duration_t WatcherTimeout = DDS_DURATION_INFINITE;
    DDS_ReturnCode_t result;
    DDS_ConditionSeq *conditionList;
    DDS_StringSeq WatcherId;
    DDS_SubscriberQos *sQos;
    int length, i, proceed;

    WatcherId._maximum   = 1;
    WatcherId._length    = 1;
    WatcherId._buffer    = DDS_StringSeq_allocbuf(1);
    WatcherId._buffer[0] = DDS_string_dup(argv[1]);

    readCommandLineParams(argc, argv);

    /*
     * Create WaitSet
     */
    WatcherWaitset = DDS_WaitSet__alloc ();

    /*
     * Create participant
     */
    WatcherFactory = DDS_DomainParticipantFactory_get_instance ();

    if (WatcherFactory == DDS_HANDLE_NIL) {
        printf ("Watcher %s: ERROR - missing factoy instance\n",
                argv[0]);
        exit (1);
    }

    WatcherParticipant = DDS_DomainParticipantFactory_create_participant (
                             WatcherFactory,
                             WatcherDomain,
                             DDS_PARTICIPANT_QOS_DEFAULT,
                             NULL,
                             DDS_ANY_STATUS);

    if (WatcherParticipant == DDS_HANDLE_NIL) {
        printf ("Watcher %s: ERROR - OpenSplice not running\n",
                argv[0]);
        exit (1);
    }

    sQos = DDS_SubscriberQos__alloc();

    DDS_DomainParticipant_get_default_subscriber_qos (WatcherParticipant,
                                                      sQos);

    sQos->partition.name._length = 1;
    sQos->partition.name._maximum = 1;
    sQos->partition.name._buffer = DDS_StringSeq_allocbuf (1);
    sQos->partition.name._buffer[0] = DDS_string_dup(DDSTouchStoneReportPartition);

    WatcherSubscriber = DDS_DomainParticipant_create_subscriber (
                             WatcherParticipant,
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
              WatcherParticipant,
              "DDSTouchStone::transmitterReport");

    trt = DDS_DomainParticipant_create_topic (
              WatcherParticipant,
              "transmitterReportTopic",
              "DDSTouchStone::transmitterReport",
              DDS_TOPIC_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Create datareader */
    trr = DDS_Subscriber_create_datareader (
              WatcherSubscriber,
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

    result = DDS_WaitSet_attach_condition (WatcherWaitset, trrc);

    /*
     * ReceiverReport
     */

    /*  Create Topic */
    rrs = DDSTouchStone_receiverReportTypeSupport__alloc ();

    DDSTouchStone_receiverReportTypeSupport_register_type (
              rrs,
              WatcherParticipant,
              "DDSTouchStone::receiverReport");

    rrt = DDS_DomainParticipant_create_topic (
              WatcherParticipant,
              "receiverReportTopic",
              "DDSTouchStone::receiverReport",
              DDS_TOPIC_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Create datareader */
    rrr = DDS_Subscriber_create_datareader (
              WatcherSubscriber,
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

    result = DDS_WaitSet_attach_condition (WatcherWaitset, rrrc);

    /*
     * transceiverReport
     */

    /*  Create Topic */
    srs = DDSTouchStone_transceiverReportTypeSupport__alloc ();

    DDSTouchStone_transceiverReportTypeSupport_register_type (
              srs, WatcherParticipant,
              "DDSTouchStone::transceiverReport");

    srt = DDS_DomainParticipant_create_topic (
              WatcherParticipant,
              "transceiverReportTopic",
              "DDSTouchStone::transceiverReport",
              DDS_TOPIC_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Create datareader */
    srr = DDS_Subscriber_create_datareader (
              WatcherSubscriber,
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

    result = DDS_WaitSet_attach_condition (WatcherWaitset, srrc);

    /*
     * transponderReport
     */

    /*  Create Topic */
    xrs = DDSTouchStone_transponderReportTypeSupport__alloc ();

    DDSTouchStone_transponderReportTypeSupport_register_type (
              xrs, WatcherParticipant,
              "DDSTouchStone::transponderReport");

    xrt = DDS_DomainParticipant_create_topic (
              WatcherParticipant,
              "transponderReportTopic",
              "DDSTouchStone::transponderReport",
              DDS_TOPIC_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Create datareader */
    xrr = DDS_Subscriber_create_datareader (
              WatcherSubscriber,
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

    result = DDS_WaitSet_attach_condition (WatcherWaitset, xrrc);

    /*
     * MainLoop processing
     */
    proceed = TRUE;
    while (proceed) {
        conditionList = DDS_ConditionSeq__alloc();
        result = DDS_WaitSet_wait (WatcherWaitset,
                                   conditionList,
                                   &WatcherTimeout);
        length = conditionList->_length;
        for (i = 0; (i < length) && proceed; i++) {
            proceed = process_command(conditionList->_buffer[i]);
        }
        DDS_free(conditionList);
    }
    return 0;
}

