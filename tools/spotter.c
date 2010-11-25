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
        printf("Spotter line %d: %s\n", \
               __LINE__, message)

static char *DDSTouchStoneReportPartition =  "DDSTouchStoneReports";

static DDS_DomainId_t               SpotterDomain      = DDS_OBJECT_NIL;
static DDS_DomainParticipantFactory SpotterFactory     = DDS_OBJECT_NIL;
static DDS_DomainParticipant        SpotterParticipant = DDS_OBJECT_NIL;
static DDS_Subscriber               SpotterSubscriber  = DDS_OBJECT_NIL;

/* Report TypeSupport */
static DDSTouchStone_discoveryReportTypeSupport drs = DDS_OBJECT_NIL;

/* Report Topics */
static DDS_Topic drt = DDS_OBJECT_NIL;

/* Report Readers */
static DDSTouchStone_discoveryReportDataReader drr = DDS_OBJECT_NIL;

static DDS_StatusCondition drrc = DDS_OBJECT_NIL;

static void
process_discoveryReport (
    DDSTouchStone_discoveryReport *report)
{
    switch (report->report_kind) {
    case DDSTouchStone_DataWriterDiscovery:
        printf("Discovered DataWriter (%d,%d,%d): \n"
               "    creation time: %.2f usec\n"
               "    discovery time: %.2f usec\n",
               report->application_id,
               report->partition_id,
               report->entity_id,
               report->creation_duration,
               report->discovery_time);
        if (report->discovery_time >= report->creation_duration) {
            printf("    So discovered %.2f usec after creation\n",
                   report->discovery_time - report->creation_duration);
        } else {
            printf("    So discovered %.2f usec before creation was finished\n",
                   report->discovery_time - report->creation_duration);
        }
        printf("    writer messages lost before discovered: %d\n",
               report->samples_missed);
    break;
    case DDSTouchStone_DataReaderDiscovery:
        printf("Discovered DataReader (%d,%d,%d): \n"
               "    creation time: %.2f usec\n"
               "    discovery time: %.2f usec\n",
               report->application_id,
               report->partition_id,
               report->entity_id,
               report->creation_duration,
               report->discovery_time);
        if (report->creation_duration <= report->discovery_time) {
            printf("    So discovered %.2f usec after creation\n",
                   report->discovery_time - report->creation_duration);
        } else {
            printf("    So discovered %.2f usec before creation was finished\n",
                   report->creation_duration - report->discovery_time);
        }
    break;
    }
}

static DDS_boolean
process_command (
    DDS_StatusCondition condition)
{
    DDS_sequence_DDSTouchStone_discoveryReport drl = {0,0,DDS_OBJECT_NIL,FALSE};
    DDS_SampleInfoSeq infoList = { 0, 0, NULL, FALSE };
    DDS_ReturnCode_t  result;
    int length, i;

    if (condition == drrc) {
        result = DDSTouchStone_discoveryReportDataReader_read_w_condition (
                     drr, &drl, &infoList, 1, drrc);
        if (result == DDS_RETCODE_OK) {
            length = drl._length;
            if (length != 0) {
                for (i=0; i<length; i++) {
                    process_discoveryReport(&drl._buffer[i]);
                }
                result = DDSTouchStone_discoveryReportDataReader_return_loan (
                             drr, &drl, &infoList);
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
    DDS_WaitSet SpotterWaitset;
    DDS_Duration_t SpotterTimeout = DDS_DURATION_INFINITE;
    DDS_ReturnCode_t result;
    DDS_ConditionSeq *conditionList;
    DDS_SubscriberQos *sQos;
    int length, i, proceed;

    /*
     * Create WaitSet
     */
    SpotterWaitset = DDS_WaitSet__alloc ();

    /*
     * Create participant
     */
    SpotterFactory = DDS_DomainParticipantFactory_get_instance ();

    if (SpotterFactory == DDS_HANDLE_NIL) {
        printf ("Spotter %s: ERROR - missing factoy instance\n",
                argv[0]);
        exit (1);
    }

    SpotterParticipant = DDS_DomainParticipantFactory_create_participant (
                             SpotterFactory,
                             SpotterDomain,
                             DDS_PARTICIPANT_QOS_DEFAULT,
                             NULL,
                             DDS_ANY_STATUS);

    if (SpotterParticipant == DDS_HANDLE_NIL) {
        printf ("Spotter %s: ERROR - OpenSplice not running\n",
                argv[0]);
        exit (1);
    }

    sQos = DDS_SubscriberQos__alloc();

    DDS_DomainParticipant_get_default_subscriber_qos (SpotterParticipant,
                                                      sQos);

    sQos->partition.name._length = 1;
    sQos->partition.name._maximum = 1;
    sQos->partition.name._buffer = DDS_StringSeq_allocbuf (1);
    sQos->partition.name._buffer[0] = DDS_string_dup(DDSTouchStoneReportPartition);

    SpotterSubscriber = DDS_DomainParticipant_create_subscriber (
                             SpotterParticipant,
                             sQos,
                             NULL,
                             DDS_ANY_STATUS);

    DDS_free(sQos);

    /*
     * discoveryReport
     */

    /*  Create Topic */
    drs = DDSTouchStone_discoveryReportTypeSupport__alloc ();

    DDSTouchStone_discoveryReportTypeSupport_register_type (
              drs,
              SpotterParticipant,
              "DDSTouchStone::discoveryReport");

    drt = DDS_DomainParticipant_create_topic (
              SpotterParticipant,
              "discoveryReportTopic",
              "DDSTouchStone::discoveryReport",
              DDS_TOPIC_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Create datareader */
    drr = DDS_Subscriber_create_datareader (
              SpotterSubscriber,
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

    result = DDS_WaitSet_attach_condition (SpotterWaitset, drrc);

    /*
     * MainLoop processing
     */
    proceed = TRUE;
    while (proceed) {
        conditionList = DDS_ConditionSeq__alloc();
        result = DDS_WaitSet_wait (SpotterWaitset,
                                   conditionList,
                                   &SpotterTimeout);
        length = conditionList->_length;
        for (i = 0; (i < length) && proceed; i++) {
            proceed = process_command(conditionList->_buffer[i]);
        }
        DDS_free(conditionList);
    }
    return 0;
}

