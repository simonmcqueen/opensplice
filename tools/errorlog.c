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
        printf("ErrorLog line %d: %s\n", \
               __LINE__, message)

/* static char *DDSTouchStoneCommandPartition = "DDSTouchStoneCommands"; */ /* Not used */
static char *DDSTouchStoneReportPartition =  "DDSTouchStoneReports";

static DDS_DomainId_t               ErrorLogDomain      = DDS_DOMAIN_ID_DEFAULT;
static DDS_DomainParticipantFactory ErrorLogFactory     = DDS_OBJECT_NIL;
static DDS_DomainParticipant        ErrorLogParticipant = DDS_OBJECT_NIL;
static DDS_Subscriber               ErrorLogSubscriber  = DDS_OBJECT_NIL;

/* Report TypeSupport */
static DDSTouchStone_errorReportTypeSupport       ers = DDS_OBJECT_NIL;

/* Report Topics */
static DDS_Topic ert = DDS_OBJECT_NIL;

/* Report Writers */
static DDSTouchStone_errorReportDataReader       err = DDS_OBJECT_NIL;

static DDS_StatusCondition errc = DDS_OBJECT_NIL;

static void
process_errorReport (
    DDSTouchStone_errorReport *report)
{
    printf("Received Error report from (%d,%d,%d)\nMessage: %s\n",
           report->application_id,
           report->partition_id,
           report->entity_id,
           report->message);
}

static DDS_boolean
process_command (
    DDS_StatusCondition condition)
{
    DDS_sequence_DDSTouchStone_errorReport       erl = {0,0,DDS_OBJECT_NIL,FALSE};
    DDS_SampleInfoSeq infoList = { 0, 0, NULL, FALSE };
    DDS_ReturnCode_t  result;
    int length, i;

    if (condition == errc) {
        result = DDSTouchStone_errorReportDataReader_read_w_condition (
                     err, &erl, &infoList, 1, errc);
        if (result == DDS_RETCODE_OK) {
            length = erl._length;
            if (length != 0) {
                for (i=0; i<length; i++) {
                    process_errorReport(&erl._buffer[i]);
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

int
main (
    int argc,
    char *argv[])
{
    DDS_WaitSet ErrorLogWaitset;
    DDS_Duration_t ErrorLogTimeout = DDS_DURATION_INFINITE;
    DDS_ReturnCode_t result;
    DDS_ConditionSeq *conditionList;
    DDS_StringSeq ErrorLogId;
    DDS_TopicQos *tQos;
    DDS_SubscriberQos *sQos;
    DDS_DataReaderQos *rQos;
    int length, i, proceed;

    ErrorLogId._maximum   = 1;
    ErrorLogId._length    = 1;
    ErrorLogId._buffer    = DDS_StringSeq_allocbuf(1);
    ErrorLogId._buffer[0] = DDS_string_dup(argv[1]);

    /*
     * Create WaitSet
     */
    ErrorLogWaitset = DDS_WaitSet__alloc ();

    /*
     * Create participant
     */
    ErrorLogFactory = DDS_DomainParticipantFactory_get_instance ();

    if (ErrorLogFactory == DDS_HANDLE_NIL) {
        printf ("ErrorLog %s: ERROR - missing factoy instance\n",
                argv[0]);
        exit (1);
    }

    ErrorLogParticipant = DDS_DomainParticipantFactory_create_participant (
                             ErrorLogFactory,
                             ErrorLogDomain,
                             DDS_PARTICIPANT_QOS_DEFAULT,
                             NULL,
                             DDS_ANY_STATUS);

    if (ErrorLogParticipant == DDS_HANDLE_NIL) {
        printf ("ErrorLog %s: ERROR - OpenSplice not running\n",
                argv[0]);
        exit (1);
    }

    sQos = DDS_SubscriberQos__alloc();

    DDS_DomainParticipant_get_default_subscriber_qos (ErrorLogParticipant,
                                                      sQos);

    sQos->partition.name._length = 1;
    sQos->partition.name._maximum = 1;
    sQos->partition.name._buffer = DDS_StringSeq_allocbuf (1);
    sQos->partition.name._buffer[0] = DDS_string_dup(DDSTouchStoneReportPartition);

    ErrorLogSubscriber = DDS_DomainParticipant_create_subscriber (
                             ErrorLogParticipant,
                             sQos,
                             NULL,
                             DDS_ANY_STATUS);

    DDS_free(sQos);

    /*
     * errorReport
     */

    /*  Create Topic */
    ers = DDSTouchStone_errorReportTypeSupport__alloc ();

    DDSTouchStone_errorReportTypeSupport_register_type (
              ers, ErrorLogParticipant,
              "DDSTouchStone::errorReport");

    tQos = DDS_TopicQos__alloc();

    DDS_DomainParticipant_get_default_topic_qos(ErrorLogParticipant, tQos);
    tQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    tQos->durability.kind = DDS_TRANSIENT_DURABILITY_QOS;

    ert = DDS_DomainParticipant_create_topic (
              ErrorLogParticipant,
              "errorReportTopic",
              "DDSTouchStone::errorReport",
              tQos,
              NULL,
              DDS_ANY_STATUS);

    rQos = DDS_DataReaderQos__alloc();

    DDS_Subscriber_copy_from_topic_qos(ErrorLogSubscriber, rQos, tQos);

    /* Create datareader */
    err = DDS_Subscriber_create_datareader (
              ErrorLogSubscriber,
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

    result = DDS_WaitSet_attach_condition (ErrorLogWaitset, errc);

    /*
     * MainLoop processing
     */
    proceed = TRUE;
    while (proceed) {
        conditionList = DDS_ConditionSeq__alloc();
        result = DDS_WaitSet_wait (ErrorLogWaitset,
                                   conditionList,
                                   &ErrorLogTimeout);
        length = conditionList->_length;
        for (i = 0; (i < length) && proceed; i++) {
            proceed = process_command(conditionList->_buffer[i]);
        }
        DDS_free(conditionList);
    }
    return 0;
}

