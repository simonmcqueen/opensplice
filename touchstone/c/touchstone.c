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

#include "touchstone_os_abstraction.h"
#include "touchstone_vendor_abstraction_c.h"

#include "metrics.h"
#include "alive_writer.h"

static char *DDS_TOUCHSTONE_VERSION = "1.3";
static char *DDS_TOUCHSTONE_DATE = "Feb 5, 2009";

static char *DDS_RESULT[] = {
        "DDS_RETCODE_OK","DDS_RETCODE_ERROR","DDS_RETCODE_UNSUPPORTED",
        "DDS_RETCODE_BAD_PARAMETER","DDS_RETCODE_PRECONDITION_NOT_MET",
        "DDS_RETCODE_OUT_OF_RESOURCES","DDS_RETCODE_NOT_ENABLED",
        "DDS_RETCODE_IMMUTABLE_POLICY","DDS_RETCODE_INCONSISTENT_POLICY",
        "DDS_RETCODE_ALREADY_DELETED","DDS_RETCODE_TIMEOUT",
        "DDS_RETCODE_NODATA"};

static char *OS_RESULT[] = {
        "OS_resultSucces", "OS_resultFail"};

static char message[480];
static DDSTouchStone_errorReport error_msg = {0,0,0,message};

static DDS_Duration_t DELAY = {1,0};

#define REPORT_ERROR(aid,pid,eid) \
        error_msg.application_id = aid; \
        error_msg.partition_id = pid; \
        error_msg.entity_id = eid; \
        DDSTouchStone_errorReportDataWriter_write ( \
                erw, &error_msg, DDS_HANDLE_NIL)

#define ENTITY_ERROR(pid,eid,msg) \
        sprintf((void *)error_msg.message, \
                "%s line %d:\n    %s\n", \
                application_name, __LINE__, \
                msg); \
        REPORT_ERROR(application_id,pid,eid)

#define OS_ERROR(pid,eid,msg,code) \
        sprintf((void *)error_msg.message, \
                "%s line %d:\n    %s => %s\n", \
                application_name, __LINE__, \
                msg, OS_RESULT[code]); \
        REPORT_ERROR(application_id,pid,eid)

#define DDS_ERROR(pid,eid,msg,code) \
        sprintf((void *)error_msg.message, \
                "%s line %d:\n    %s => %s\n", \
                application_name, __LINE__, \
                msg, DDS_RESULT[code]); \
        REPORT_ERROR(application_id,pid,eid)

#define DDS_INFO(pid,eid,msg,details) \
        printf( "%s line %d:\n    %s => %s\n", \
                application_name, __LINE__, \
                msg, details);


/* Translate 0,1,2,3,4,5 into 'A','a','B','b','C','c'.. */
#define PAYLOAD_ITEM_FROM_UINT(i) \
    (char)(65+(((i/2)%26)+(i%2)*32))

static char *DDSTouchStoneCommandPartition = "DDSTouchStoneCommands";
static char *DDSTouchStoneReportPartition =  "DDSTouchStoneReports";

static char *application_name = "Unknown";
static long application_id = 0;
static long group_id = 0;
/* static unsigned long random_id = 0; */

static DDS_DomainId_t               DDSTouchStoneDomain      = DDS_DOMAIN_ID_DEFAULT;
static DDS_DomainParticipantFactory DDSTouchStoneFactory     = DDS_OBJECT_NIL;
static DDS_DomainParticipant        DDSTouchStoneParticipant = DDS_OBJECT_NIL;
static DDS_Subscriber               DDSTouchStoneSubscriber  = DDS_OBJECT_NIL;
static DDS_Publisher                DDSTouchStonePublisher   = DDS_OBJECT_NIL;
static DDS_Publisher                ReportPublisher   = DDS_OBJECT_NIL;

static DDSTouchStone_transmitterDefDataReader tdr = DDS_OBJECT_NIL;
static DDSTouchStone_transmitterQosDataReader tqr = DDS_OBJECT_NIL;
static DDSTouchStone_receiverDefDataReader    rdr = DDS_OBJECT_NIL;
static DDSTouchStone_receiverQosDataReader    rqr = DDS_OBJECT_NIL;
static DDSTouchStone_transceiverDefDataReader sdr = DDS_OBJECT_NIL;
static DDSTouchStone_transceiverQosDataReader sqr = DDS_OBJECT_NIL;
static DDSTouchStone_transponderDefDataReader xdr = DDS_OBJECT_NIL;
static DDSTouchStone_transponderQosDataReader xqr = DDS_OBJECT_NIL;

static DDSTouchStone_transmitterDefTypeSupport tds = DDS_OBJECT_NIL;
static DDSTouchStone_transmitterQosTypeSupport tqs = DDS_OBJECT_NIL;
static DDSTouchStone_receiverDefTypeSupport    rds = DDS_OBJECT_NIL;
static DDSTouchStone_receiverQosTypeSupport    rqs = DDS_OBJECT_NIL;
static DDSTouchStone_transceiverDefTypeSupport sds = DDS_OBJECT_NIL;
static DDSTouchStone_transceiverQosTypeSupport sqs = DDS_OBJECT_NIL;
static DDSTouchStone_transponderDefTypeSupport xds = DDS_OBJECT_NIL;
static DDSTouchStone_transponderQosTypeSupport xqs = DDS_OBJECT_NIL;

static DDS_Topic tdt = DDS_OBJECT_NIL;
static DDS_Topic tqt = DDS_OBJECT_NIL;
static DDS_Topic rdt = DDS_OBJECT_NIL;
static DDS_Topic rqt = DDS_OBJECT_NIL;
static DDS_Topic sdt = DDS_OBJECT_NIL;
static DDS_Topic sqt = DDS_OBJECT_NIL;
static DDS_Topic xdt = DDS_OBJECT_NIL;
static DDS_Topic xqt = DDS_OBJECT_NIL;

static DDS_ContentFilteredTopic tdf = DDS_OBJECT_NIL;
static DDS_ContentFilteredTopic tqf = DDS_OBJECT_NIL;
static DDS_ContentFilteredTopic rdf = DDS_OBJECT_NIL;
static DDS_ContentFilteredTopic rqf = DDS_OBJECT_NIL;
static DDS_ContentFilteredTopic sdf = DDS_OBJECT_NIL;
static DDS_ContentFilteredTopic sqf = DDS_OBJECT_NIL;
static DDS_ContentFilteredTopic xdf = DDS_OBJECT_NIL;
static DDS_ContentFilteredTopic xqf = DDS_OBJECT_NIL;

static DDS_StatusCondition tdrc = DDS_OBJECT_NIL;
static DDS_StatusCondition tqrc = DDS_OBJECT_NIL;
static DDS_StatusCondition rdrc = DDS_OBJECT_NIL;
static DDS_StatusCondition rqrc = DDS_OBJECT_NIL;
static DDS_StatusCondition sdrc = DDS_OBJECT_NIL;
static DDS_StatusCondition sqrc = DDS_OBJECT_NIL;
static DDS_StatusCondition xdrc = DDS_OBJECT_NIL;
static DDS_StatusCondition xqrc = DDS_OBJECT_NIL;

static DDSTouchStone_transmitterQosDataWriter tqw = DDS_OBJECT_NIL;
static DDSTouchStone_receiverQosDataWriter    rqw = DDS_OBJECT_NIL;
static DDSTouchStone_transceiverQosDataWriter sqw = DDS_OBJECT_NIL;
static DDSTouchStone_transponderQosDataWriter xqw = DDS_OBJECT_NIL;

/* static DDS_DataWriterQos *defaultTransmitterQos = DDS_OBJECT_NIL; */ /* Not used */
/* static DDS_DataReaderQos *defaultReceiverQos    = DDS_OBJECT_NIL; */ /* Not used */

/* Report TypeSupport */
static DDSTouchStone_transmitterReportTypeSupport trs = DDS_OBJECT_NIL;
static DDSTouchStone_receiverReportTypeSupport    rrs = DDS_OBJECT_NIL;
static DDSTouchStone_transceiverReportTypeSupport srs = DDS_OBJECT_NIL;
static DDSTouchStone_transponderReportTypeSupport xrs = DDS_OBJECT_NIL;
static DDSTouchStone_errorReportTypeSupport       ers = DDS_OBJECT_NIL;
static DDSTouchStone_discoveryReportTypeSupport   drs = DDS_OBJECT_NIL;

/* Report Topics */
static DDS_Topic trt = DDS_OBJECT_NIL;
static DDS_Topic rrt = DDS_OBJECT_NIL;
static DDS_Topic srt = DDS_OBJECT_NIL;
static DDS_Topic xrt = DDS_OBJECT_NIL;
static DDS_Topic ert = DDS_OBJECT_NIL;
static DDS_Topic drt = DDS_OBJECT_NIL;

/* Report Writers */
static DDSTouchStone_transmitterReportDataWriter trw = DDS_OBJECT_NIL;
static DDSTouchStone_receiverReportDataWriter    rrw = DDS_OBJECT_NIL;
static DDSTouchStone_transceiverReportDataWriter srw = DDS_OBJECT_NIL;
static DDSTouchStone_transponderReportDataWriter xrw = DDS_OBJECT_NIL;
static DDSTouchStone_errorReportDataWriter       erw = DDS_OBJECT_NIL;
static DDSTouchStone_discoveryReportDataWriter   drw = DDS_OBJECT_NIL;

typedef struct transmitter_t *transmitter;
typedef struct receiver_t    *receiver;
typedef struct transceiver_t *transceiver;
typedef struct transponder_t *transponder;
typedef struct participant_t *participant;

struct transmitter_t {
    DDSTouchStone_transmitterDef  config;
    int                           config_number;
    DDS_Topic                     topic;
    DDS_DataWriterQos            *wQos;
    DDS_DataWriter                writer;
    DDS_boolean                   active;
    touchstone_os_threadId        threadId;
    touchstone_os_timer           timer;
    DDS_QueryCondition            query;
    DDSTouchStone_timestamp       creation_time;
    DDSTouchStone_usec_duration   creation_duration;
    participant                   owner;
    transmitter                   next;
};

struct receiver_t {
    DDSTouchStone_receiverDef     config;
    int                           config_number;
    DDS_Topic                     topic;
    DDS_DataReaderQos            *rQos;
    DDS_DataReader                reader;
    DDS_boolean                   active;
    touchstone_os_threadId        threadId;
    touchstone_os_threadId        reportThreadId;
    DDS_WaitSet                   waitset;
    DDS_StatusCondition           condition;
    DDS_QueryCondition            query;
    alive_writer                  alive_writers;
    DDSTouchStone_timestamp       creation_time;
    DDSTouchStone_usec_duration   creation_duration;
    participant                   owner;
    int                           byte_count;
    int                           msg_count;
    receiver                      next;
};

struct transceiver_t {
    DDSTouchStone_transceiverDef  config;
    int                           config_number;
    DDS_Topic                     topic;
    DDS_Topic                     echo_topic;
    DDS_DataWriterQos            *wQos;
    DDS_DataReaderQos            *rQos;
    DDS_DataWriter                writer;
    DDS_DataReader                reader;
    DDS_boolean                   writerActive;
    DDS_boolean                   readerActive;
    DDS_boolean                   reportActive;
    touchstone_os_threadId        writerThreadId;
    touchstone_os_threadId        readerThreadId;
    touchstone_os_threadId        reportThreadId;
    touchstone_os_timer           timer;
    DDS_WaitSet                   waitset;
    DDS_StatusCondition           condition;
    DDS_QueryCondition            query;
    metrics                       send_latency;
    metrics                       echo_latency;
    metrics                       trip_latency;
    metrics                       send_source_latency;
    metrics                       send_arrival_latency;
    metrics                       send_trip_latency;
    metrics                       echo_source_latency;
    metrics                       echo_arrival_latency;
    metrics                       echo_trip_latency;
    metrics                       inter_arrival_time;
    participant                   owner;
    transceiver                   next;
};

struct transponder_t {
    DDSTouchStone_transponderDef  config;
    int                           config_number;
    DDS_Topic                     topic;
    DDS_Topic                     echo_topic;
    DDS_DataWriterQos            *wQos;
    DDS_DataReaderQos            *rQos;
    DDS_DataWriter                writer;
    DDS_DataReader                reader;
    DDS_boolean                   active;
    touchstone_os_threadId        threadId;
    DDS_WaitSet                   waitset;
    DDS_StatusCondition           condition;
    DDS_QueryCondition            query;
    participant                   owner;
    transponder                   next;
};

struct participant_t {
    long                  partition_id;
    DDS_DomainParticipant domain_participant;
    DDS_Publisher         publisher;
    DDS_Subscriber        subscriber;
    DDS_TypeSupport       throughput_typeSupport;
    DDS_TypeSupport       latency_typeSupport;
    transmitter           transmitters;
    receiver              receivers;
    transceiver           transceivers;
    transponder           transponders;
    participant           next;
};


/* ------------------------------------------------------------------- */


static void
receiver_report(
    receiver _this,
    double period,
    DDSTouchStone_receiverReport *report)
{
    double count;

    report->read_bytes_per_second = 1000.0*(_this->byte_count)/period;
    report->read_msgs_per_second = 1000.0*(_this->msg_count)/period;
    _this->byte_count = 0;
    _this->msg_count = 0;
}


static int
copy_touchstone_datawriter_qos(
    DDS_DataWriterQos *destQos,
    DDSTouchStone_DataWriterQos *srcQos)
{
    /* TODO: implement checking for changes */
    int changed = 0;

    destQos->transport_priority.value = srcQos->transport_priority.value;
    destQos->latency_budget.duration.sec = srcQos->latency_budget.duration.sec;
    destQos->latency_budget.duration.nanosec = srcQos->latency_budget.duration.nanosec;

    return changed;
}

static int
copy_touchstone_datareader_qos(
    DDS_DataReaderQos *destQos,
    DDSTouchStone_DataReaderQos *srcQos)
{
    /* TODO: implement checking for changes */
    int changed = 0;

    destQos->history.depth = srcQos->history.depth;
    destQos->latency_budget.duration.sec = srcQos->latency_budget.duration.sec;
    destQos->latency_budget.duration.nanosec = srcQos->latency_budget.duration.nanosec;

    return changed;
}

static void
set_touchstone_datawriter_qos_default(
    DDSTouchStone_DataWriterQos *qos)
{
    qos->latency_budget.duration.sec = 0;
    qos->latency_budget.duration.nanosec = 0;
    qos->transport_priority.value = 0;
}

static void
set_touchstone_datareader_qos_default(
    DDSTouchStone_DataReaderQos *qos)
{
    qos->latency_budget.duration.sec = 0;
    qos->latency_budget.duration.nanosec = 0;
    qos->history.depth = 1;
}

static participant Admin = NULL;

static DDS_boolean
process_command (
    DDS_StatusCondition condition);

/* Convenience function */
static touchstone_os_result
threadAttrInit(
    touchstone_os_threadAttr *_this,
    DDSTouchStone_SchedulingClass scheduling_class,
    touchstone_os_priority thread_priority)
{
    touchstone_os_result result;

    result = touchstone_os_threadAttrInit(_this);

    if (result == touchstone_os_resultSuccess) {
        if (scheduling_class == DDSTouchStone_REALTIME) {
            _this->schedClass = TOUCHSTONE_OS_SCHED_REALTIME;
        }
        if (thread_priority != 0) {
            _this->schedPriority = thread_priority;
        }
    }
    return result;
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

static DDSTouchStone_timestamp
get_timestamp(
    void)
{
    DDS_Time_t t;

    DDS_DomainParticipant_get_current_time(DDSTouchStoneParticipant,&t);
    return get_timestamp_from_dds_time(&t);
}

static unsigned long
get_random_value (
    void)
{
    DDS_Time_t t;

    DDS_DomainParticipant_get_current_time(DDSTouchStoneParticipant,&t);
    return t.nanosec;
}

static void
msec_sleep(
    DDSTouchStone_msec_t duration)
{
#if 1
    touchstone_os_time delay;

    delay.tv_sec = duration / 1000;
    delay.tv_nsec = (duration % 1000) * 1000000;
    touchstone_os_nanoSleep(delay);
#else
    if (duration > 0) {
        usleep(duration*1000);
    }
#endif
}

static void
set_topic_qos(
    DDS_TopicQos *tQos,
    DDSTouchStone_TopicKind kind)
{
    switch (kind) {
    case DDSTouchStone_RELIABLE:
        tQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    break;
    case DDSTouchStone_TRANSIENT:
        tQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
        tQos->durability.kind = DDS_TRANSIENT_DURABILITY_QOS;
    break;
    case DDSTouchStone_PERSISTENT:
        tQos->durability.kind = DDS_PERSISTENT_DURABILITY_QOS;
    break;
    case DDSTouchStone_BEST_EFFORT:
    break;
    }
}

static char *
set_topic_name (
    char *dst,
    char *name,
    unsigned long id,
    DDSTouchStone_TopicKind kind)
{
    switch (kind) {
    case DDSTouchStone_RELIABLE:
        sprintf(dst, "%s_%lu_R",name,id);
    break;
    case DDSTouchStone_TRANSIENT:
        sprintf(dst, "%s_%lu_T",name,id);
    break;
    case DDSTouchStone_PERSISTENT:
        sprintf(dst, "%s_%lu_P",name,id);
    break;
    case DDSTouchStone_BEST_EFFORT:
        sprintf(dst, "%s_%lu_B",name,id);
    break;
    }
    return dst;
}

static void
activate_transmitter_report()
{
    if (trw == DDS_OBJECT_NIL) {
        if (trs == DDS_OBJECT_NIL) {
            trs = DDSTouchStone_transmitterReportTypeSupport__alloc ();

            DDSTouchStone_transmitterReportTypeSupport_register_type (
                      trs,
                      DDSTouchStoneParticipant,
                      "DDSTouchStone::transmitterReport");
        }
        if (trt == DDS_OBJECT_NIL) {
            trt = DDS_DomainParticipant_create_topic (
                      DDSTouchStoneParticipant,
                      "transmitterReportTopic",
                      "DDSTouchStone::transmitterReport",
                      DDS_TOPIC_QOS_DEFAULT,
                      NULL,
                      DDS_ANY_STATUS);
        }
        trw = DDS_Publisher_create_datawriter(
                      DDSTouchStonePublisher,
                      trt,
                      DDS_DATAWRITER_QOS_DEFAULT,
                      NULL,
                      DDS_ANY_STATUS);
    }
}

static void
activate_receiver_report()
{
    if (rrw == DDS_OBJECT_NIL) {
        if (rrs == DDS_OBJECT_NIL) {
            rrs = DDSTouchStone_receiverReportTypeSupport__alloc ();

            DDSTouchStone_receiverReportTypeSupport_register_type (
                      rrs,
                      DDSTouchStoneParticipant,
                      "DDSTouchStone::receiverReport");
        }
        if (rrt == DDS_OBJECT_NIL) {
            rrt = DDS_DomainParticipant_create_topic (
                      DDSTouchStoneParticipant,
                      "receiverReportTopic",
                      "DDSTouchStone::receiverReport",
                      DDS_TOPIC_QOS_DEFAULT,
                      NULL,
                      DDS_ANY_STATUS);
        }
        rrw = DDS_Publisher_create_datawriter(
                      ReportPublisher,
                      rrt,
                      DDS_DATAWRITER_QOS_DEFAULT,
                      NULL,
                      DDS_ANY_STATUS);
    }
}

static void
activate_discovery_report()
{
    if (drw == DDS_OBJECT_NIL) {
        if (drs == DDS_OBJECT_NIL) {
            drs = DDSTouchStone_discoveryReportTypeSupport__alloc ();

            DDSTouchStone_discoveryReportTypeSupport_register_type (
                      drs,
                      DDSTouchStoneParticipant,
                      "DDSTouchStone::discoveryReport");
        }
        if (drt == DDS_OBJECT_NIL) {
            drt = DDS_DomainParticipant_create_topic (
                      DDSTouchStoneParticipant,
                      "discoveryReportTopic",
                      "DDSTouchStone::discoveryReport",
                      DDS_TOPIC_QOS_DEFAULT,
                      NULL,
                      DDS_ANY_STATUS);
        }
        drw = DDS_Publisher_create_datawriter(
                      ReportPublisher,
                      drt,
                      DDS_DATAWRITER_QOS_DEFAULT,
                      NULL,
                      DDS_ANY_STATUS);
    }
}

static void
activate_transceiver_report()
{
    if (srw == DDS_OBJECT_NIL) {
        if (srs == DDS_OBJECT_NIL) {
            srs = DDSTouchStone_transceiverReportTypeSupport__alloc ();

            DDSTouchStone_transceiverReportTypeSupport_register_type (
                      srs,
                      DDSTouchStoneParticipant,
                      "DDSTouchStone::transceiverReport");
        }
        if (srt == DDS_OBJECT_NIL) {
            srt = DDS_DomainParticipant_create_topic (
                      DDSTouchStoneParticipant,
                      "transceiverReportTopic",
                      "DDSTouchStone::transceiverReport",
                      DDS_TOPIC_QOS_DEFAULT,
                      NULL,
                      DDS_ANY_STATUS);
        }
        srw = DDS_Publisher_create_datawriter(
                      ReportPublisher,
                      srt,
                      DDS_DATAWRITER_QOS_DEFAULT,
                      NULL,
                      DDS_ANY_STATUS);
    }
}

static void
activate_transponder_report()
{
    if (xrw == DDS_OBJECT_NIL) {
        if (xrs == DDS_OBJECT_NIL) {
            xrs = DDSTouchStone_transponderReportTypeSupport__alloc ();

            DDSTouchStone_transponderReportTypeSupport_register_type (
                      xrs,
                      DDSTouchStoneParticipant,
                      "DDSTouchStone::transponderReport");
        }
        if (xrt == DDS_OBJECT_NIL) {
            xrt = DDS_DomainParticipant_create_topic (
                      DDSTouchStoneParticipant,
                      "transponderReportTopic",
                      "DDSTouchStone::transponderReport",
                      DDS_TOPIC_QOS_DEFAULT,
                      NULL,
                      DDS_ANY_STATUS);
        }
        xrw = DDS_Publisher_create_datawriter(
                      ReportPublisher,
                      xrt,
                      DDS_DATAWRITER_QOS_DEFAULT,
                      NULL,
                      DDS_ANY_STATUS);
    }
}

int
main (
    int argc,
    char *argv[])
{
    DDS_WaitSet DDSTouchStoneWaitset;
    DDS_Duration_t DDSTouchStoneTimeout = DDS_DURATION_INFINITE;
    DDS_ReturnCode_t retcode;
    DDS_ConditionSeq *conditionList;
    DDS_StringSeq groupId;
    DDS_TopicQos     *tQos;
    DDS_SubscriberQos *sQos;
    DDS_PublisherQos  *pQos;
    DDS_DataWriterQos *wQos;
    DDS_DataReaderQos *rQos;
    int length, i, proceed;

    if (argc == 2) {
        if (strcmp(argv[1], "-info") == 0) {
            printf("This is touchstone version %s, built on %s\n",
                DDS_TOUCHSTONE_VERSION, DDS_TOUCHSTONE_DATE);
            exit(0);
        }
    }

    if ((argc < 2) || (argc > 3)) {
        printf ("DDSTouchStone %s Invalid usage\n"
                "Usage: %s application_id [ group_id ]\n",
                argv[0], argv[0]);
        exit(1);
    }

    groupId._maximum   = 1;
    groupId._length    = 1;
    groupId._buffer    = DDS_StringSeq_allocbuf(1);

    application_name = argv[0];
    application_id = atoi(argv[1]);
    if (argc == 3) {
        group_id = atoi(argv[2]);
        groupId._buffer[0] = DDS_string_dup(argv[2]);
    } else {
        group_id = application_id;
        groupId._buffer[0] = DDS_string_dup(argv[1]);
    }
    /* random_id = get_random_value(); */

    /*
     * Create WaitSet
     */
    DDSTouchStoneWaitset = DDS_WaitSet__alloc ();

    /*
     * Create participant
     */
    DDSTouchStoneFactory = DDS_DomainParticipantFactory_get_instance ();

    if (DDSTouchStoneFactory == DDS_HANDLE_NIL) {
        printf ("DDSTouchStone %s %lu: ERROR - missing factoy instance\n",
                argv[0],application_id);
        exit (1);
    }

    DDSTouchStoneParticipant = DDS_DomainParticipantFactory_create_participant (
                             DDSTouchStoneFactory,
                             DDSTouchStoneDomain,
                             DDS_PARTICIPANT_QOS_DEFAULT,
                             NULL,
                             DDS_ANY_STATUS);

    if (DDSTouchStoneParticipant == DDS_HANDLE_NIL) {
        printf ("DDSTouchStone %s %lu: ERROR - OpenSplice not running\n",
                argv[0],application_id);
        exit (1);
    }
    pQos = DDS_PublisherQos__alloc();
    DDS_DomainParticipant_get_default_publisher_qos (DDSTouchStoneParticipant,
                                                     pQos);

    pQos->partition.name._length = 1;
    pQos->partition.name._maximum = 1;
    pQos->partition.name._buffer = DDS_StringSeq_allocbuf (1);
    pQos->partition.name._buffer[0] = DDS_string_dup(DDSTouchStoneReportPartition);

    ReportPublisher = DDS_DomainParticipant_create_publisher (
                             DDSTouchStoneParticipant,
                             pQos,
                             NULL,
                             DDS_ANY_STATUS);
    if (ReportPublisher == DDS_HANDLE_NIL) {
        printf ("DDSTouchStone %s %lu: ERROR - failed to create ReportPublisher\n",
                argv[0],application_id);
        exit (1);
    }

    DDS_free(pQos->partition.name._buffer[0]);
    pQos->partition.name._buffer[0] = DDS_string_dup(DDSTouchStoneCommandPartition);

    DDSTouchStonePublisher = DDS_DomainParticipant_create_publisher (
                             DDSTouchStoneParticipant,
                             pQos,
                             NULL,
                             DDS_ANY_STATUS);

    DDS_free(pQos);

    if (DDSTouchStonePublisher == DDS_HANDLE_NIL) {
        printf ("DDSTouchStone %s %lu: ERROR - failed to create DDSTouchStonePublisher\n",
                argv[0],application_id);
        exit (1);
    }

    ers = DDSTouchStone_errorReportTypeSupport__alloc ();

    DDSTouchStone_errorReportTypeSupport_register_type (
              ers,
              DDSTouchStoneParticipant,
              "DDSTouchStone::errorReport");

    tQos = DDS_TopicQos__alloc();

    DDS_DomainParticipant_get_default_topic_qos(DDSTouchStoneParticipant, tQos);
    tQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    tQos->durability.kind = DDS_TRANSIENT_DURABILITY_QOS;

    ert = DDS_DomainParticipant_create_topic (
              DDSTouchStoneParticipant,
              "errorReportTopic",
              "DDSTouchStone::errorReport",
              tQos,
              NULL,
              DDS_ANY_STATUS);

    if (ert == DDS_HANDLE_NIL) {
        printf ("DDSTouchStone %s %lu: ERROR - failed to create ErrorReport Topic\n",
                argv[0],application_id);
        exit (1);
    }

    wQos = DDS_DataWriterQos__alloc();
    DDS_Publisher_get_default_datawriter_qos(ReportPublisher, wQos);
    DDS_Publisher_copy_from_topic_qos(ReportPublisher, wQos, tQos);

    erw = DDS_Publisher_create_datawriter(
              ReportPublisher,
              ert,
              wQos,
              NULL,
              DDS_ANY_STATUS);

    if (erw == DDS_HANDLE_NIL) {
        printf ("DDSTouchStone %s %lu: ERROR - failed to create ErrorReport Writer\n",
                argv[0],application_id);
        exit (1);
    }

    error_msg.application_id = application_id;

    /* from now on error no longer are printed to stdout but are instead
       published by the ErrorReport Writer. */

    sQos = DDS_SubscriberQos__alloc();
    DDS_DomainParticipant_get_default_subscriber_qos (DDSTouchStoneParticipant,
                                                      sQos);

    sQos->partition.name._length = 1;
    sQos->partition.name._maximum = 1;
    sQos->partition.name._buffer = DDS_StringSeq_allocbuf (1);
    sQos->partition.name._buffer[0] = DDS_string_dup(DDSTouchStoneCommandPartition);

    DDSTouchStoneSubscriber = DDS_DomainParticipant_create_subscriber (
                             DDSTouchStoneParticipant,
                             sQos,
                             NULL,
                             DDS_ANY_STATUS);
    DDS_free(sQos);

    if (DDSTouchStoneSubscriber == NULL) {
        ENTITY_ERROR(0,0,"Failed to create DDSTouchStoneSubscriber");
        exit(1);
    }

    rQos = DDS_DataReaderQos__alloc();
    DDS_Subscriber_get_default_datareader_qos(DDSTouchStoneSubscriber, rQos);
    DDS_Subscriber_copy_from_topic_qos(DDSTouchStoneSubscriber, rQos, tQos);

    /*
     * TransmitterDef
     */

    /*  Create Topic */
    tds = DDSTouchStone_transmitterDefTypeSupport__alloc ();

    retcode = DDSTouchStone_transmitterDefTypeSupport_register_type (
              tds,
              DDSTouchStoneParticipant,
              "DDSTouchStone::transmitterDef");

    if (retcode != DDS_RETCODE_OK) {
        DDS_ERROR(0,0,
                  "failed to register type: "
                  "DDSTouchStone::transmitterDef",
                  retcode);
        exit(1);
    }
    tdt = DDS_DomainParticipant_create_topic (
              DDSTouchStoneParticipant,
              "transmitterDefTopic",
              "DDSTouchStone::transmitterDef",
              DDS_TOPIC_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    if (tdt == NULL) {
        ENTITY_ERROR(0,0,"Failed to create Topic: "
              "transmitterDefTopic");
        exit(1);
    }

    /*  Create Content Filtered Topic */
    tdf = DDS_DomainParticipant_create_contentfilteredtopic (
              DDSTouchStoneParticipant,
              "transmitterDefFilteredTopic",
              tdt,
              "group_id = %0",
              &groupId);

    if (tdf == NULL) {
        ENTITY_ERROR(0,0,"Failed to create Topic: "
              "transmitterDefFilteredTopic");
        exit(1);
    }

    /* Create datareader */
    tdr = DDS_Subscriber_create_datareader (
              DDSTouchStoneSubscriber,
              tdf,
              DDS_DATAREADER_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    if (tdr == NULL) {
        ENTITY_ERROR(0,0,
              "Failed to create a DataReader for: "
              "transmitterDefFilteredTopic");
        exit(1);
    }

    /* Add datareader readcondition to waitset */

    tdrc = DDS_DataReader_create_readcondition (
              tdr,
              DDS_NOT_READ_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    if (tdrc == NULL) {
        ENTITY_ERROR(0,0,
              "Failed to create a read condition for "
              "transmitterDefFilteredTopic DataReader");
        exit(1);
    }

    retcode = DDS_WaitSet_attach_condition (DDSTouchStoneWaitset, tdrc);

    if (retcode != DDS_RETCODE_OK) {
        DDS_ERROR(0,0,
                  "failed to attach transmitterDefFilteredTopic "
                  "read condition to waitset",
                  retcode);
        exit(1);
    }
    /*
     * TransmitterQos
     */

    /*  Create Topic */
    tqs = DDSTouchStone_transmitterQosTypeSupport__alloc ();

    DDSTouchStone_transmitterQosTypeSupport_register_type (
              tqs,
              DDSTouchStoneParticipant,
              "DDSTouchStone::transmitterQos");

    tqt = DDS_DomainParticipant_create_topic (
              DDSTouchStoneParticipant,
              "transmitterQosTopic",
              "DDSTouchStone::transmitterQos",
              tQos,
              NULL,
              DDS_ANY_STATUS);

    /*  Create Content Filtered Topic */
    tqf = DDS_DomainParticipant_create_contentfilteredtopic (
              DDSTouchStoneParticipant,
              "transmitterQosFilteredTopic",
              tqt,
              "group_id = %0",
              &groupId);

    /* Create datawriter */
    tqw = DDS_Publisher_create_datawriter (
              DDSTouchStonePublisher,
              tqt,
              wQos,
              NULL,
              DDS_ANY_STATUS );

    /* Create datareader */
    tqr = DDS_Subscriber_create_datareader (
              DDSTouchStoneSubscriber,
              tqf,
              rQos,
              NULL,
              DDS_ANY_STATUS);

    /* Add datareader readcondition to waitset */

    tqrc = DDS_DataReader_create_readcondition (
              tqr,
              DDS_NOT_READ_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    retcode = DDS_WaitSet_attach_condition (DDSTouchStoneWaitset, tqrc);

    /*
     * ReceiverDef
     */

    /*  Create Topic */
    rds = DDSTouchStone_receiverDefTypeSupport__alloc ();

    DDSTouchStone_receiverDefTypeSupport_register_type (
              rds,
              DDSTouchStoneParticipant,
              "DDSTouchStone::receiverDef");

    rdt = DDS_DomainParticipant_create_topic (
              DDSTouchStoneParticipant,
              "receiverDefTopic",
              "DDSTouchStone::receiverDef",
              DDS_TOPIC_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /*  Create Content Filtered Topic */
    rdf = DDS_DomainParticipant_create_contentfilteredtopic (
              DDSTouchStoneParticipant,
              "receiverDefFilteredTopic",
              rdt,
              "group_id = %0",
              &groupId);

    /* Create datareader */
    rdr = DDS_Subscriber_create_datareader (
              DDSTouchStoneSubscriber,
              rdf,
              DDS_DATAREADER_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Add datareader readcondition to waitset */

    rdrc = DDS_DataReader_create_readcondition (
              rdr,
              DDS_ANY_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    retcode = DDS_WaitSet_attach_condition (DDSTouchStoneWaitset, rdrc);
    /*
     * ReceiverQos
     */

    /*  Create Topic */
    rqs = DDSTouchStone_receiverQosTypeSupport__alloc ();

    DDSTouchStone_receiverQosTypeSupport_register_type (
        rqs, DDSTouchStoneParticipant,
        "DDSTouchStone::receiverQos");

    rqt = DDS_DomainParticipant_create_topic (
              DDSTouchStoneParticipant,
              "receiverQosTopic",
              "DDSTouchStone::receiverQos",
              tQos,
              NULL,
              DDS_ANY_STATUS);

    /*  Create Content Filtered Topic */
    rqf = DDS_DomainParticipant_create_contentfilteredtopic (
              DDSTouchStoneParticipant,
              "receiverQosFilteredTopic",
              rqt,
              "group_id = %0",
              &groupId);

    /* Create datawriter */
    rqw = DDS_Publisher_create_datawriter (
              DDSTouchStonePublisher,
              rqt,
              wQos,
              NULL,
              DDS_ANY_STATUS );

    /* Create datareader */
    rqr = DDS_Subscriber_create_datareader (
              DDSTouchStoneSubscriber,
              rqf,
              rQos,
              NULL,
              DDS_ANY_STATUS);

    /* Add datareader readcondition to waitset */

    rqrc = DDS_DataReader_create_readcondition (
              rqr,
              DDS_NOT_READ_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    retcode = DDS_WaitSet_attach_condition (DDSTouchStoneWaitset, rqrc);

    /*
     * transceiverDef
     */

    /*  Create Topic */
    sds = DDSTouchStone_transceiverDefTypeSupport__alloc ();

    DDSTouchStone_transceiverDefTypeSupport_register_type (
              sds, DDSTouchStoneParticipant,
              "DDSTouchStone::transceiverDef");

    sdt = DDS_DomainParticipant_create_topic (
              DDSTouchStoneParticipant,
              "transceiverDefTopic",
              "DDSTouchStone::transceiverDef",
              DDS_TOPIC_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /*  Create Content Filtered Topic */
    sdf = DDS_DomainParticipant_create_contentfilteredtopic (
              DDSTouchStoneParticipant,
              "transceiverDefFilteredTopic",
              sdt,
              "group_id = %0",
              &groupId);

    /* Create datareader */
    sdr = DDS_Subscriber_create_datareader (
              DDSTouchStoneSubscriber,
              sdf,
              DDS_DATAREADER_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Add datareader readcondition to waitset */

    sdrc = DDS_DataReader_create_readcondition (
              sdr,
              DDS_NOT_READ_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    retcode = DDS_WaitSet_attach_condition (DDSTouchStoneWaitset, sdrc);
    /*
     * transceiverQos
     */

    /*  Create Topic */
    sqs = DDSTouchStone_transceiverQosTypeSupport__alloc ();

    DDSTouchStone_transceiverQosTypeSupport_register_type (
              sqs, DDSTouchStoneParticipant,
              "DDSTouchStone::transceiverQos");

    sqt = DDS_DomainParticipant_create_topic (
              DDSTouchStoneParticipant,
              "transceiverQosTopic",
              "DDSTouchStone::transceiverQos",
              tQos,
              NULL,
              DDS_ANY_STATUS);

    /*  Create Content Filtered Topic */
    sqf = DDS_DomainParticipant_create_contentfilteredtopic (
              DDSTouchStoneParticipant,
              "transceiverQosFilteredTopic",
              sqt,
              "group_id = %0",
              &groupId);

    /* Create datawriter */
    sqw = DDS_Publisher_create_datawriter (
              DDSTouchStonePublisher,
              sqt,
              wQos,
              NULL,
              DDS_ANY_STATUS );

    /* Create datareader */
    sqr = DDS_Subscriber_create_datareader (
              DDSTouchStoneSubscriber,
              sqf,
              rQos,
              NULL,
              DDS_ANY_STATUS);

    /* Add datareader readcondition to waitset */

    sqrc = DDS_DataReader_create_readcondition (
              sqr,
              DDS_NOT_READ_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    retcode = DDS_WaitSet_attach_condition (DDSTouchStoneWaitset, sqrc);

    /*
     * transponderDef
     */

    /*  Create Topic */
    xds = DDSTouchStone_transponderDefTypeSupport__alloc ();

    DDSTouchStone_transponderDefTypeSupport_register_type (
              xds, DDSTouchStoneParticipant,
              "DDSTouchStone::transponderDef");

    xdt = DDS_DomainParticipant_create_topic (
              DDSTouchStoneParticipant,
              "transponderDefTopic",
              "DDSTouchStone::transponderDef",
              DDS_TOPIC_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /*  Create Content Filtered Topic */
    xdf = DDS_DomainParticipant_create_contentfilteredtopic (
              DDSTouchStoneParticipant,
              "transponderDefFilteredTopic",
              xdt,
              "group_id = %0",
              &groupId);

    /* Create datareader */
    xdr = DDS_Subscriber_create_datareader (
              DDSTouchStoneSubscriber,
              xdf,
              DDS_DATAREADER_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Add datareader readcondition to waitset */

    xdrc = DDS_DataReader_create_readcondition (
              xdr,
              DDS_NOT_READ_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    retcode = DDS_WaitSet_attach_condition (DDSTouchStoneWaitset, xdrc);
    /*
     * transponderQos
     */

    /*  Create Topic */
    xqs = DDSTouchStone_transponderQosTypeSupport__alloc ();

    DDSTouchStone_transponderQosTypeSupport_register_type (
        xqs, DDSTouchStoneParticipant,
        "DDSTouchStone::transponderQos");

    xqt = DDS_DomainParticipant_create_topic (
              DDSTouchStoneParticipant,
              "transponderQosTopic",
              "DDSTouchStone::transponderQos",
              tQos,
              NULL,
              DDS_ANY_STATUS);

    /*  Create Content Filtered Topic */
    xqf = DDS_DomainParticipant_create_contentfilteredtopic (
              DDSTouchStoneParticipant,
              "transponderQosFilteredTopic",
              xqt,
              "group_id = %0",
              &groupId);

    /* Create datawriter */
    xqw = DDS_Publisher_create_datawriter (
              DDSTouchStonePublisher,
              xqt,
              wQos,
              NULL,
              DDS_ANY_STATUS );

    /* Create datareader */
    xqr = DDS_Subscriber_create_datareader (
              DDSTouchStoneSubscriber,
              xqf,
              rQos,
              NULL,
              DDS_ANY_STATUS);

    /* Add datareader readcondition to waitset */

    xqrc = DDS_DataReader_create_readcondition (
              xqr,
              DDS_NOT_READ_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    retcode = DDS_WaitSet_attach_condition (DDSTouchStoneWaitset, xqrc);

    DDS_free(rQos);
    DDS_free(wQos);
    DDS_free(tQos);

    activate_discovery_report();
    /*
     * MainLoop processing
     */
    proceed = TRUE;
    conditionList = DDS_ConditionSeq__alloc();
    while (proceed) {
        retcode = DDS_WaitSet_wait (DDSTouchStoneWaitset,
                                   conditionList,
                                   &DDSTouchStoneTimeout);
        if (retcode == DDS_RETCODE_OK) {
            length = conditionList->_length;
            for (i = 0; (i < length) && proceed; i++) {
                proceed = process_command(conditionList->_buffer[i]);
            }
        } else {
            DDS_ERROR(0,0,
                      "main loop: "
                      "DDS_WaitSet_wait failed",
                      retcode);
        }
    }
    DDS_free(conditionList);
    return 0;
}

/*****************************************************************************
 * Transmitter Implementation
 *****************************************************************************/

static participant
lookup_participant (
    long partition_id)
{
    participant p;
    p = Admin;
    while ((p != NULL) &&
           (p->partition_id != partition_id)) {
        p = p->next;
    }
    return p;
}

static participant
create_participant (
    long partition_id)
{
    DDS_PublisherQos *pQos;
    DDS_SubscriberQos *sQos;
    DDS_ReturnCode_t retcode;
    participant p;
    char partitionName[64];

    p = (participant)malloc(sizeof(*p));
    p->partition_id = partition_id;
    p->transmitters = NULL;
    p->receivers = NULL;
    p->transceivers = NULL;
    p->transponders = NULL;
    p->next = Admin;

    sprintf(partitionName, "%ld", partition_id);

    p->domain_participant = DDS_DomainParticipantFactory_create_participant (
                             DDSTouchStoneFactory,
                             DDSTouchStoneDomain,
                             DDS_PARTICIPANT_QOS_DEFAULT,
                             NULL,
                             DDS_ANY_STATUS);

    if (p->domain_participant != DDS_HANDLE_NIL) {
        pQos = DDS_PublisherQos__alloc();

        retcode = DDS_DomainParticipant_get_default_publisher_qos (
                     p->domain_participant,
                     pQos);

        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(partition_id,
                      0,
                      "create_participant: "
                      "get_default_publisher_qos failed",
                      retcode);
            return NULL;
        }

        pQos->partition.name._length = 1;
        pQos->partition.name._maximum = 1;
        pQos->partition.name._buffer = DDS_StringSeq_allocbuf (1);
        pQos->partition.name._buffer[0] = DDS_string_alloc(strlen(partitionName));
        strcpy (pQos->partition.name._buffer[0], partitionName);

        p->publisher = DDS_DomainParticipant_create_publisher (
                             p->domain_participant,
                             pQos, NULL, DDS_ANY_STATUS);

        DDS_free(pQos);

        if (p->publisher == DDS_HANDLE_NIL) {
            ENTITY_ERROR(partition_id,
                  0,
                  "create_participant: "
                  "create_publisher failed");
            return NULL;
        }

        sQos = DDS_SubscriberQos__alloc();

        retcode = DDS_DomainParticipant_get_default_subscriber_qos (
                     p->domain_participant,
                     sQos);

        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(partition_id,
                      0,
                      "create_participant: "
                      "get_default_subscriber_qos failed",
                      retcode);
            return NULL;
        }

        sQos->partition.name._length = 1;
        sQos->partition.name._maximum = 1;
        sQos->partition.name._buffer = DDS_StringSeq_allocbuf (1);
        sQos->partition.name._buffer[0] = DDS_string_alloc(strlen(partitionName));
        strcpy (sQos->partition.name._buffer[0], partitionName);

        p->subscriber = DDS_DomainParticipant_create_subscriber (
                             p->domain_participant,
                             sQos, NULL, DDS_ANY_STATUS);

        DDS_free(sQos);

        if (p->subscriber == DDS_HANDLE_NIL) {
            ENTITY_ERROR(partition_id,
                  0,
                  "create_participant: "
                  "create_subscriber failed");
            return NULL;
        }

        p->throughput_typeSupport =
        DDSTouchStone_throughput_messageTypeSupport__alloc ();
        DDSTouchStone_throughput_messageTypeSupport_register_type (
                      p->throughput_typeSupport,
                      p->domain_participant,
                      "DDSTouchStone::throughput_message");

        if (p->throughput_typeSupport == DDS_HANDLE_NIL) {
            ENTITY_ERROR(partition_id,
                  0,
                  "create_participant:register_type "
                  "DDSTouchStone::throughput_message failed");
            return NULL;
        }

        p->latency_typeSupport =
        DDSTouchStone_latency_messageTypeSupport__alloc ();
        DDSTouchStone_latency_messageTypeSupport_register_type (
                      p->latency_typeSupport,
                      p->domain_participant,
                      "DDSTouchStone::latency_message");

        if (p->latency_typeSupport == DDS_HANDLE_NIL) {
            ENTITY_ERROR(partition_id,
                  0,
                  "create_participant:register_type "
                  "DDSTouchStone::latency_message failed");
            return NULL;
        }

        p->next = Admin;
        Admin = p;
    } else {
        ENTITY_ERROR(partition_id,
              0,
              "create_participant: "
              "create_participant failed");
        free(p);
        p = NULL;
    }
    return p;
}

static transmitter
lookup_transmitter (
    participant p,
    unsigned long transmitter_id)
{
    transmitter t;

    t = p->transmitters;
    while ((t != NULL) &&
           (t->config.transmitter_id != transmitter_id)) {
        t = t->next;
    }
    return t;
}

static void
transmitter_del_topic (
    transmitter _this)
{
    DDS_ReturnCode_t retcode;

    if (_this->topic != NULL) {
        retcode = DDS_DomainParticipant_delete_topic(
                      _this->owner, _this->topic);
        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.transmitter_id,
                      "transmitter_del_topic: "
                      "delete Topic failed",
                      retcode);
        } else {
            _this->topic = NULL;
        }
    }
}

static void
transmitter_set_topic (
    transmitter _this)
{
    DDS_TopicQos *tQos;
    DDS_ReturnCode_t retcode;
    char id[32];

    tQos = DDS_TopicQos__alloc();

    retcode = DDS_DomainParticipant_get_default_topic_qos(
                  _this->owner->domain_participant,
                  tQos);

    if (retcode == DDS_RETCODE_OK) {
        set_topic_qos(tQos, _this->config.topic_kind);
        set_topic_name(id,"ThroughputTopic",
                       _this->config.topic_id,
                       _this->config.topic_kind);
        transmitter_del_topic(_this);
        _this->topic =
            DDS_DomainParticipant_create_topic (
                      _this->owner->domain_participant,
                      id,
                      "DDSTouchStone::throughput_message",
                      tQos,
                      NULL,
                      DDS_ANY_STATUS);

        DDS_free(tQos);

        if (_this->topic == DDS_HANDLE_NIL) {
            ENTITY_ERROR(_this->config.partition_id,
                  _this->config.transmitter_id,
                  "transmitter_set_topic: "
                  "create_topic failed");
        }
    } else {
        DDS_ERROR(_this->config.partition_id,
                  _this->config.transmitter_id,
                  "transmitter_set_topic: "
                  "get_default_topic_qos failed",
                  retcode);
    }
}

static void
transmitter_del_writer (
    transmitter _this)
{
    DDS_ReturnCode_t retcode;

    if (_this->writer != NULL) {
        retcode = DDS_Publisher_delete_datawriter(
                      _this->owner->publisher, _this->writer);
        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                 _this->config.transmitter_id,
                 "transmitter_del_writer: "
                 "delete DataWriter failed",
                 retcode);
        } else {
            _this->writer = NULL;
        }
    }
}

static void
transmitter_set_writer (
    transmitter _this)
{
    transmitter_del_writer(_this);
    _this->writer = DDS_Publisher_create_datawriter (
                           _this->owner->publisher,
                           _this->topic,
                           _this->wQos,
                           NULL,
                           DDS_ANY_STATUS );
    if (_this->writer == NULL) {
        ENTITY_ERROR(_this->config.partition_id,
              _this->config.transmitter_id,
              "transmitter_set_writer: "
              "create DataWriter failed");
    }
    assert(_this->writer);
}

void *
transmitterThread(
    void *arg)
{
    transmitter _this = (transmitter)arg;
    DDSTouchStone_throughput_message *message;
    DDS_ReturnCode_t retcode;
    unsigned int i, index;
    unsigned int size, last_size;
    unsigned int last_burst_period;
    touchstone_os_time interval;

    _this->creation_time = get_timestamp();
    transmitter_set_writer(_this);

    message = DDSTouchStone_throughput_message__alloc();
    message->application_id = application_id;
    /* message->random_id = random_id; */
    message->random_id = get_random_value();
    message->transmitter_id = _this->config.transmitter_id;
    message->creation_time = _this->creation_time;
    message->creation_duration = get_timestamp() - _this->creation_time;
    message->sequence_number = 0;
    message->payload_data._buffer = DDSTouchStone_payload_allocbuf(0);
    message->payload_data._length = 0;
    message->payload_data._maximum = 0;
    last_size = 0;
    last_burst_period = _this->config.burst_period;
    touchstone_os_timerStart(_this->timer);

    retcode = DDS_RETCODE_OK;
    while (_this->active) {
        if (_this->config.burst_period != last_burst_period) {
            last_burst_period = _this->config.burst_period;
            interval.tv_sec = last_burst_period / 1000;
            interval.tv_nsec = (last_burst_period % 1000) * 1000000;
            touchstone_os_timerIntervalSet(
                _this->timer, interval);
        }
        if (_this->config.message_size != last_size) {
            size = sizeof(DDSTouchStone_throughput_message);
            if (_this->config.message_size > size) {
                size = _this->config.message_size - size;
            } else {
                size = 1;
            }
            last_size = _this->config.message_size;
            DDS_free(message->payload_data._buffer);
            message->payload_data._buffer = DDSTouchStone_payload_allocbuf(size);
            message->payload_data._length = size;
            message->payload_data._maximum = size;
            for (i=0; i<size; i++) {
                message->payload_data._buffer[i] = PAYLOAD_ITEM_FROM_UINT(i);
            }
        }
        message->sequence_number++;
        message->config_number = _this->config_number;
        message->write_timestamp = get_timestamp();
        for (index = 0; index < _this->config.messages_per_burst; index++) {
            message->instance_id = index;
            retcode = DDSTouchStone_throughput_messageDataWriter_write (
                     _this->writer,
                     message,
                     DDS_HANDLE_NIL);
            if (retcode != DDS_RETCODE_OK) {
                if (retcode == DDS_RETCODE_TIMEOUT) {
                    DDS_INFO(_this->config.partition_id,
                          _this->config.transmitter_id,
                          "transmitterThread: "
                          "DDSTouchStone_throughput_messageDataWriter_write",
                          "transmitter is writing too fast, slowing down");
                } else {
                    DDS_ERROR(_this->config.partition_id,
                          _this->config.transmitter_id,
                          "transmitterThread: "
                          "DDSTouchStone_throughput_messageDataWriter_write",
                          retcode);
                }
            }
        }
        touchstone_os_timerWait(_this->timer);
    }
    DDS_free (message);
    transmitter_del_writer(_this);
    return (void *)retcode;
}

static touchstone_os_result
transmitter_start_thread (
    transmitter _this)
{
    touchstone_os_threadAttr threadAttr;
    touchstone_os_result result;
    char name[32];

    sprintf(name,"transmitter_%d",_this->config.transmitter_id);

    if (_this->active) {
        _this->active = FALSE;
        result = touchstone_os_threadWaitExit(_this->threadId, NULL);
        if (result != touchstone_os_resultSuccess) {
            OS_ERROR(_this->config.partition_id,
                     _this->config.transmitter_id,
                     "transmitter_start_thread: "
                     "Thread wait exit failed",
                     result);
        }
    }
    _this->active = TRUE;

    result = threadAttrInit(&threadAttr,
                   _this->config.scheduling_class,
                   _this->config.thread_priority);

    if (result == touchstone_os_resultSuccess) {
        result = touchstone_os_threadCreate(
                       &_this->threadId,
                       name,
                       &threadAttr,
                       transmitterThread,
                       _this);
        if (result != touchstone_os_resultSuccess) {
            OS_ERROR(_this->config.partition_id,
                     _this->config.transmitter_id,
                     "transmitter_start_thread: "
                      "touchstone_os_threadCreate failed",
                     result);
        }
    } else {
        OS_ERROR(_this->config.partition_id,
                 _this->config.transmitter_id,
                 "transmitter_start_thread: "
                 "threadAttrInit failed",
                 result);
    }
    return result;
}

static void
transmitter_set_qos (
    transmitter _this)
{
    DDS_sequence_DDSTouchStone_transmitterQos tql;
    DDS_SampleInfoSeq infoList;
    DDS_ReturnCode_t retcode;
    DDS_TopicQos *tQos;
    DDSTouchStone_DataWriterQos *qos;
    DDSTouchStone_transmitterQos tqm;
    touchstone_os_result result;
    int changed;

    infoList._release = TRUE;
    infoList._maximum = 1;
    infoList._length  = 0;
    infoList._buffer  = DDS_SampleInfoSeq_allocbuf(1);

    tql._release = TRUE;
    tql._maximum = 1;
    tql._length  = 0;
    tql._buffer  = DDS_sequence_DDSTouchStone_transmitterQos_allocbuf(1);

    retcode = DDSTouchStone_transmitterQosDataReader_take_w_condition (
                   tqr, &tql, &infoList, 1, _this->query);

    if ((retcode != DDS_RETCODE_OK) && (retcode != DDS_RETCODE_NO_DATA)) {
        DDS_ERROR(_this->config.partition_id,
                  _this->config.transmitter_id,
                  "transmitter_set_qos: "
                  "read transmitter Qos policies failed",
                  retcode);
    }

    if (retcode == DDS_RETCODE_NO_DATA) {
         tqm.group_id = group_id;
         tqm.transmitter_id = _this->config.transmitter_id;
         tqm.partition_id   = _this->config.partition_id;

         set_touchstone_datawriter_qos_default(&tqm.qos);

         retcode = DDSTouchStone_transmitterQosDataWriter_write (
                       tqw,
                       &tqm,
                       DDS_HANDLE_NIL);

         if (retcode != DDS_RETCODE_OK) {
             DDS_ERROR(_this->config.partition_id,
                       _this->config.transmitter_id,
                       "transmitter_set_qos: "
                       "DDSTouchStone_transmitterQosDataWriter_write",
                       retcode);
         }
    }

    if (_this->wQos == NULL) {
        _this->wQos = DDS_DataWriterQos__alloc();
        retcode = DDS_Publisher_get_default_datawriter_qos(
                     _this->owner->publisher,
                     _this->wQos);
        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.transmitter_id,
                      "transmitter_set_qos: "
                      "get_default_datawriter_qos failed",
                      retcode);
        }
    }
    tQos = DDS_TopicQos__alloc();
    retcode = DDS_Topic_get_qos(_this->topic, tQos);
    if (retcode != DDS_RETCODE_OK) {
        DDS_ERROR(_this->config.partition_id,
                  _this->config.transmitter_id,
                  "transmitter_set_qos: "
                  "DDS_Topic_get_qos failed",
                  retcode);
    }

    retcode = DDS_Publisher_copy_from_topic_qos(
                  _this->owner->publisher, _this->wQos, tQos);
    if (retcode != DDS_RETCODE_OK) {
        DDS_ERROR(_this->config.partition_id,
                  _this->config.transmitter_id,
                  "transmitter_set_qos: "
                  "DDS_Publisher_copy_from_topic_qos failed",
                  retcode);
    }
    DDS_free(tQos);

    /* For reliable transport, the transmitter writer
     * needs to set extra policies for its resource
     * limits, which avoids excessive memory growth
     * and invokes flow control if needed */
    if (_this->wQos->reliability.kind == DDS_RELIABLE_RELIABILITY_QOS) {
        _this->wQos->history.kind = DDS_KEEP_ALL_HISTORY_QOS;
        _this->wQos->resource_limits.max_samples = 1;
    }

    if (tql._length == 1) {
        qos = &tql._buffer[0].qos;
	changed = copy_touchstone_datawriter_qos(_this->wQos, qos);
        if (changed) {
	    if (_this->active) {
                result = transmitter_start_thread (_this);
                if (result != touchstone_os_resultSuccess) {
                    OS_ERROR(_this->config.partition_id,
                              _this->config.transmitter_id,
                             "transmitter_set_qos: "
                             "Thread creation failed",
                             result);
                }
            }
        } else {
            if (_this->writer) {
                retcode = DDS_DataWriter_set_qos(_this->writer,_this->wQos);
                if (retcode != DDS_RETCODE_OK) {
                    DDS_ERROR(_this->config.partition_id,
                              _this->config.transmitter_id,
                              "transmitter_set_qos: "
                              "DataWriter_set_qos failed",
                              retcode);
                }
            }
        }
        _this->config_number++;
    } else if (tql._length > 1) {
        ENTITY_ERROR(_this->config.partition_id,
              _this->config.transmitter_id,
              "transmitter_set_qos: "
              "too many messages received");
    }
}

static void
dispose_transmitterDef (
    DDSTouchStone_transmitterDef *msg)
{
    participant p;
    transmitter _this;
    transmitter *ref;
    touchstone_os_result result;

    p = lookup_participant(msg->partition_id);
    if (p != NULL) {
        _this = lookup_transmitter(p,msg->transmitter_id);
        if (_this != NULL) {
            _this->active = FALSE;
            result = touchstone_os_threadWaitExit(_this->threadId, NULL);
            if (result != touchstone_os_resultSuccess) {
                OS_ERROR(msg->partition_id,
                         msg->transmitter_id,
                         "dispose_transmitterDef: "
                         "touchstone_os_threadWaitExit failed",
                         result);
            }
            DDS_DataReader_delete_readcondition(tqr, _this->query);
            DDS_free(_this->wQos);
            transmitter_del_topic(_this);
            ref = &p->transmitters;
            while ((*ref != NULL) && (*ref != _this)) {
                ref = &(*ref)->next;
            }
            assert(*ref == _this);
            *ref = _this->next;
            _this->next = NULL;
            free(_this);
        }
    }
}

static void
process_transmitterDef (
    DDSTouchStone_transmitterDef *msg)
{
    participant p;
    transmitter _this;
    DDS_StringSeq TransmitterId;
    touchstone_os_result result;
    char id[32];
    DDSTouchStone_timestamp timestamp;
    touchstone_os_time interval;

    p = lookup_participant(msg->partition_id);
    if (p == NULL) {
        p = create_participant(msg->partition_id);
        _this = NULL;
    } else {
        _this = lookup_transmitter(p,msg->transmitter_id);
    }
    if (p != NULL) {
        if (_this == NULL) {
            activate_transmitter_report();
            sprintf(id,"%d",msg->transmitter_id);

            TransmitterId._maximum   = 1;
            TransmitterId._length    = 1;
            TransmitterId._buffer    = DDS_StringSeq_allocbuf(1);
            TransmitterId._buffer[0] = DDS_string_dup(id);

            _this = (transmitter)malloc(sizeof(*_this));

            _this->config = *msg;
            _this->config_number = 0;
            _this->topic = NULL;
            _this->wQos = NULL;
            _this->writer = NULL;

            memset(&(_this->threadId), 0, sizeof(_this->threadId));

            interval.tv_sec = _this->config.burst_period / 1000;
            interval.tv_nsec = (_this->config.burst_period % 1000) * 1000000;
            _this->timer = touchstone_os_timerCreate(interval);
            _this->active = FALSE;
            _this->owner = p;
            _this->query  = DDS_DataReader_create_querycondition(
                               tqr,
                               DDS_ANY_SAMPLE_STATE,
                               DDS_ANY_VIEW_STATE,
                               DDS_ANY_INSTANCE_STATE,
                               "transmitter_id = %0",
                               &TransmitterId);

            transmitter_set_topic(_this);
            transmitter_set_qos(_this);

            _this->next = p->transmitters;
            p->transmitters = _this;

            result = transmitter_start_thread (_this);

            if (result != touchstone_os_resultSuccess) {
                p->transmitters = _this->next;
                _this->next = NULL;
                _this->active = FALSE;
            }
        } else {
            if ((_this->config.scheduling_class != msg->scheduling_class) ||
                (_this->config.thread_priority != msg->thread_priority) ||
                (_this->config.topic_kind      != msg->topic_kind) ||
                (_this->config.topic_id        != msg->topic_id)) {

                _this->active = FALSE;

                result = touchstone_os_threadWaitExit(_this->threadId, NULL);
                if (result != touchstone_os_resultSuccess) {
                    OS_ERROR(msg->partition_id,
                             msg->transmitter_id,
                             "process_transmitterDef: "
                             "touchstone_os_threadWaitExit failed",
                             result);
                }

                if ((_this->config.topic_kind != msg->topic_kind) ||
                    (_this->config.topic_id   != msg->topic_id)) {
                    _this->config = *msg;
                    transmitter_set_topic (_this);
                    transmitter_set_qos(_this);
                }

                _this->config = *msg;
                _this->config_number++;

                result = transmitter_start_thread (_this);

                if (result != touchstone_os_resultSuccess) {
                    p->transmitters = _this->next;
                    _this->next = NULL;
                    _this->active = FALSE;
                }
            } else {
                _this->config = *msg;
                _this->config_number++;
            }
        }
    }
}

static void
process_transmitterQos (
    DDSTouchStone_transmitterQos *msg)
{
    participant p;
    transmitter t;

    p = lookup_participant(msg->partition_id);
    if (p != NULL) {
        t = lookup_transmitter(p,msg->transmitter_id);
        if (t != NULL) {
            transmitter_set_qos(t);
        }
    }
}

/*****************************************************************************
 * Receiver Implementation
 *****************************************************************************/

static receiver
lookup_receiver (
    participant p,
    unsigned long receiver_id)
{
    receiver r;

    r = p->receivers;
    while ((r != NULL) &&
           (r->config.receiver_id != receiver_id)) {
        r = r->next;
    }
    return r;
}

static void
receiver_del_topic (
    receiver _this)
{
    DDS_ReturnCode_t retcode;

    if (_this->topic != NULL) {
        retcode = DDS_DomainParticipant_delete_topic(
                      _this->owner, _this->topic);
        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.receiver_id,
                      "receiver_del_topic: "
                      "delete Topic failed",
                      retcode);
        } else {
            _this->topic = NULL;
        }
    }
}

static void
receiver_set_topic (
    receiver _this)
{
    DDS_TopicQos *tQos;
    DDS_ReturnCode_t retcode;
    char id[32];

    tQos = DDS_TopicQos__alloc();

    retcode = DDS_DomainParticipant_get_default_topic_qos(
                  _this->owner->domain_participant,
                  tQos);

    if (retcode == DDS_RETCODE_OK) {
        set_topic_qos(tQos, _this->config.topic_kind);
        set_topic_name(id,"ThroughputTopic",
                       _this->config.topic_id,
                       _this->config.topic_kind);
        receiver_del_topic(_this);
        _this->topic =
            DDS_DomainParticipant_create_topic (
                      _this->owner->domain_participant,
                      id,
                      "DDSTouchStone::throughput_message",
                      tQos,
                      NULL,
                      DDS_ANY_STATUS);

        DDS_free(tQos);

        if (_this->topic == DDS_HANDLE_NIL) {
            ENTITY_ERROR(_this->config.partition_id,
                  _this->config.receiver_id,
                  "receiver_set_topic: "
                  "create_topic failed");
        }
    } else {
        DDS_ERROR(_this->config.partition_id,
                  _this->config.receiver_id,
                  "receiver_set_topic: "
                  "get_default_topic_qos failed",
                  retcode);
    }
}

static void
receiver_del_reader (
    receiver _this)
{
    DDS_ReturnCode_t retcode;

    if (_this->reader != NULL) {
        if (_this->condition) {
            retcode = DDS_WaitSet_detach_condition (
                         _this->waitset,
                         _this->condition);
            if (retcode != DDS_RETCODE_OK) {
                DDS_ERROR(_this->config.partition_id,
                          _this->config.receiver_id,
                          "receiver_del_reader: "
                          "Detach condition from waitset failed",
                          retcode);
            } else {
                _this->condition = NULL;
            }
        }

        retcode = DDS_Subscriber_delete_datareader(
                     _this->owner->subscriber,
                     _this->reader);
        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.receiver_id,
                      "receiver_del_reader: "
                      "delete DataReader failed",
                      retcode);
        } else {
            _this->reader = NULL;
        }
    }
}

static void
receiver_set_reader (
    receiver _this)
{
    DDS_ReturnCode_t retcode;

    receiver_del_reader(_this);
    _this->reader = DDS_Subscriber_create_datareader (
                           _this->owner->subscriber,
                           _this->topic,
                           _this->rQos,
                           NULL,
                           DDS_ANY_STATUS );
    if (_this->reader == NULL) {
        ENTITY_ERROR(_this->config.partition_id,
              _this->config.receiver_id,
              "receiver_set_reader: "
              "create DataReader failed");
    }
    _this->condition = DDS_DataReader_get_statuscondition (
                           _this->reader);

    if (_this->condition) {
        DDS_StatusCondition_set_enabled_statuses (
                 _this->condition,
                 DDS_DATA_AVAILABLE_STATUS);

        retcode = DDS_WaitSet_attach_condition (
                     _this->waitset,
                     _this->condition);
        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.receiver_id,
                      "receiver_set_reader: "
                      "Attach condition to waitset failed",
                      retcode);
        }
    } else {
        ENTITY_ERROR(_this->config.partition_id,
              _this->config.receiver_id,
              "receiver_set_reader: "
              "Get status condition failed");
    }
    assert(_this->reader);
}


static void
receiver_add_throughput (
    receiver _this,
    DDSTouchStone_throughput_message *msg)
{
   _this->byte_count += sizeof(DDSTouchStone_throughput_message) + msg->payload_data._length;
   _this->msg_count++;
}


/* TODO: Make this configurable */
#define MAX_INSTANCES (1000)

void *
receiverThread(
    void *arg)
{
    receiver _this = (receiver)arg;
    DDS_sequence_DDSTouchStone_throughput_message dataList;
    DDS_SampleInfoSeq infoList;
    DDS_ReturnCode_t retcode;
    DDS_ConditionSeq *conditionList;
    DDS_Duration_t wait_timeout = DELAY;
    DDSTouchStone_discoveryReport discovery;
    unsigned int i;
    double timestamp;
    int config_number;
    int hdepth = _this->rQos->history.depth;
    int maxNofSamples = hdepth*MAX_INSTANCES;
    DDS_boolean is_new;
    DDS_boolean receiver_discovery_done;
    DDS_boolean has_been_reported_before;

    dataList._buffer = DDS_sequence_DDSTouchStone_throughput_message_allocbuf(maxNofSamples);
    dataList._length = 0;
    dataList._maximum = maxNofSamples;
    dataList._release = TRUE;

    infoList._buffer = DDS_SampleInfoSeq_allocbuf(maxNofSamples);
    infoList._length = 0;
    infoList._maximum = maxNofSamples;
    infoList._release = TRUE;

    config_number = 0;

    /* This receiver has not been discovered by any writer yet... */
    receiver_discovery_done = FALSE;

    /* Freeze creation time now */
    _this->creation_time = get_timestamp();
    /* and create the reader */
    receiver_set_reader (_this);

    retcode = DDS_RETCODE_OK;
    conditionList = DDS_ConditionSeq__alloc();
    timestamp = get_timestamp();
    _this->creation_duration = timestamp - _this->creation_time;
    while (_this->active) {
        if (_this->config.polling_period == 0) {
            retcode = DDS_WaitSet_wait (_this->waitset,
                                   conditionList,
                                   &wait_timeout);
        } else {
            msec_sleep(_this->config.polling_period);
        }

        retcode = DDSTouchStone_throughput_messageDataReader_take (
                     _this->reader,
                     &dataList,
                     &infoList,
                     maxNofSamples,
                     DDS_ANY_SAMPLE_STATE,
                     DDS_ANY_VIEW_STATE,
                     DDS_ANY_INSTANCE_STATE);
        timestamp = get_timestamp();
        if (retcode == DDS_RETCODE_OK) {
            for (i=0; i<dataList._length; i++) {
                if (infoList._buffer[i].valid_data) {
                    receiver_add_throughput(_this,&dataList._buffer[i]);
                    if (infoList._buffer[i].view_state == DDS_NEW_VIEW_STATE) {
                        /* There is a chance that a new writer has been
                         * discovered, or that this is a new reader in the system */
                        alive_writer_find_or_add(&(dataList._buffer[i]),
                            &is_new, &_this->alive_writers);
                        if (is_new) {
                            /* As long as we are not sure, assume that this
                             * discovery event has not been reported before */
                            has_been_reported_before = FALSE;
                            /* This writer was not yet known to this reader, check
                             * who is the new kid in town */
                            if (dataList._buffer[i].creation_time <
                                _this->creation_time) {
                                /* writer is older than reader so
                                   report reader discovery time -- if needed */
                                if (!receiver_discovery_done) {
                                    discovery.report_kind = DDSTouchStone_DataReaderDiscovery;
                                    discovery.discovery_time =
                                              timestamp - _this->creation_time;
                                    discovery.creation_duration =
                                              _this->creation_duration;
                                    discovery.application_id = application_id;
                                    discovery.partition_id =
                                              _this->config.partition_id;
                                    discovery.entity_id =
                                              _this->config.receiver_id;
                                    /* This receiver has been discoverd now */
                                    receiver_discovery_done = TRUE;
                                } else {
                                    /* Now we are sure: this has event has been reported before */
                                    has_been_reported_before = TRUE;
                                }
                            } else {
                                /* reader is older than writer so
                                   report writer discovery time. */
                                discovery.report_kind = DDSTouchStone_DataWriterDiscovery;
                                discovery.discovery_time =
                                          dataList._buffer[i].write_timestamp -
                                          dataList._buffer[i].creation_time;
                                discovery.creation_duration =
                                          dataList._buffer[i].creation_duration;
                                discovery.application_id =
                                          dataList._buffer[i].application_id;
                                discovery.partition_id =
                                          _this->config.partition_id;
                                discovery.entity_id =
                                          dataList._buffer[i].transmitter_id;
                            }

                            if (!has_been_reported_before) {
                                discovery.samples_missed =
                                      dataList._buffer[i].sequence_number-1;
                                retcode = DDSTouchStone_discoveryReportDataWriter_write (
                                             drw,
                                             &discovery,
                                             DDS_HANDLE_NIL);

                                if (retcode != DDS_RETCODE_OK) {
                                    DDS_ERROR(_this->config.partition_id,
                                              _this->config.receiver_id,
                                              "receiverThread: "
                                              "DDSTouchStone_discoveryReportDataWriter_write failed",
                                              retcode);
                                }
                            }
                        }
                    }

#if 0 /* Needs to keep track of all publishers */
                    if (dataList._buffer[i].config_number != config_number) {
                        config_number = dataList._buffer[i].config_number;
                        _this->.config_number++;
                    }
#endif
                }
            }
        } else if (retcode != DDS_RETCODE_NO_DATA) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.receiver_id,
                      "receiverThread:"
                      "DDSTouchStone_throughput_messageDataReader_take",
                      retcode);
        }
    }
    DDS_free(conditionList);

    receiver_del_reader (_this);
    return (void *)retcode;
}

static touchstone_os_result
receiver_start_readerThread (
    receiver _this)
{
    touchstone_os_threadAttr threadAttr;
    touchstone_os_result result;
    char name[32];

    sprintf(name,"receiver_%d",_this->config.receiver_id);

    if (_this->active) {
        _this->active = FALSE;
        result = touchstone_os_threadWaitExit(_this->threadId, NULL);
        if (result != touchstone_os_resultSuccess) {
            OS_ERROR(_this->config.partition_id,
                     _this->config.receiver_id,
                     "receiver_start_readerThread: "
                     "Thread wait exit failed",
                     result);
        }
    }

    _this->active = TRUE;

    result = threadAttrInit(&threadAttr,
                   _this->config.scheduling_class,
                   _this->config.thread_priority);

    if (result == touchstone_os_resultSuccess) {
        result = touchstone_os_threadCreate(
                       &_this->threadId,
                       name,
                       &threadAttr,
                       receiverThread,
                       _this);
        if (result != touchstone_os_resultSuccess) {
            OS_ERROR(_this->config.partition_id,
                     _this->config.receiver_id,
                     "receiver_start_readerThread: "
                      "touchstone_os_threadCreate failed",
                     result);
        }
    } else {
        OS_ERROR(_this->config.partition_id,
                 _this->config.receiver_id,
                 "receiver_start_readerThread: "
                 "threadAttrInit failed",
                 result);
    }
    return result;
}

void *
receiverReportThread(
    void *arg)
{
    receiver _this = (receiver)arg;
    DDS_SampleLostStatus sl_status;
    DDS_SampleRejectedStatus sr_status;
    DDS_RequestedDeadlineMissedStatus rdm_status;
    DDSTouchStone_receiverReport report;
    DDS_ReturnCode_t retcode;

    report.application_id = application_id;
    report.partition_id = _this->config.partition_id;
    report.receiver_id    = _this->config.receiver_id;

    retcode = DDS_RETCODE_OK;
    while (_this->active) {
        report.reader_status.samples_lost = 0;
        report.reader_status.samples_rejected = 0;
        report.reader_status.deadlines_missed = 0;

        msec_sleep(_this->config.report_period);

        receiver_report(_this, (double)_this->config.report_period, &report);

        if (_this->reader) {
            retcode = DDS_DataReader_get_sample_lost_status(
                         _this->reader, &sl_status);
            if (retcode != DDS_RETCODE_OK) {
                DDS_ERROR(_this->config.partition_id,
                          _this->config.receiver_id,
                          "receiverReportThread: "
                          "get_sample_lost_status failed",
                          retcode);
            } else {
                report.reader_status.samples_lost =
                       sl_status.total_count_change;
            }
            retcode = DDS_DataReader_get_sample_rejected_status(
                         _this->reader, &sr_status);
            if (retcode != DDS_RETCODE_OK) {
                DDS_ERROR(_this->config.partition_id,
                          _this->config.receiver_id,
                          "receiverReportThread: "
                          "get_sample_rejected_status failed",
                          retcode);
            } else {
                report.reader_status.samples_rejected =
                       sr_status.total_count_change;
            }
            retcode = DDS_DataReader_get_requested_deadline_missed_status(
                         _this->reader, &rdm_status);
            if (retcode != DDS_RETCODE_OK) {
                DDS_ERROR(_this->config.partition_id,
                          _this->config.receiver_id,
                          "receiverReportThread: "
                          "get_requested_deadline_missed_status failed",
                          retcode);
            } else {
                report.reader_status.deadlines_missed =
                       rdm_status.total_count_change;
            }
        }
        report.config_number = _this->config_number;
        retcode = DDSTouchStone_receiverReportDataWriter_write (
                     rrw,
                     &report,
                     DDS_HANDLE_NIL);
        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.receiver_id,
                      "receiverReportThread: "
                      "DDSTouchStone_receiverReportDataWriter_write failed",
                      retcode);
        }
    }
    touchstone_os_threadExit((void *)retcode);
    return NULL;
}

static touchstone_os_result
receiver_start_reportThread (
    receiver _this)
{
    touchstone_os_threadAttr threadAttr;
    touchstone_os_result result;
    char name[32];

    sprintf(name,"receiver_%d",_this->config.receiver_id);

    _this->active = TRUE;

    result = threadAttrInit(&threadAttr,
                   _this->config.scheduling_class,
                   _this->config.thread_priority);

    if (result == touchstone_os_resultSuccess) {
        result = touchstone_os_threadCreate(
                           &_this->reportThreadId,
                           name,
                           &threadAttr,
                           receiverReportThread,
                           _this);
        if (result != touchstone_os_resultSuccess) {
            OS_ERROR(_this->config.partition_id,
                     _this->config.receiver_id,
                     "receiver_start_reportThread: "
                     "touchstone_os_threadCreate failed",
                     result);
        }
    } else {
        OS_ERROR(_this->config.partition_id,
                 _this->config.receiver_id,
                 "receiver_start_reportThread: "
                 "threadAttrInit failed",
                 result);
    }
    return result;
}

static void
receiver_set_qos (
    receiver _this)
{
    DDS_sequence_DDSTouchStone_receiverQos rql;
    DDS_SampleInfoSeq infoList;
    DDS_ReturnCode_t retcode;
    DDS_TopicQos *tQos;
    DDSTouchStone_DataReaderQos *qos;
    DDSTouchStone_receiverQos rqm;
    touchstone_os_result result;
    int changed;

    infoList._release = TRUE;
    infoList._maximum = 1;
    infoList._length  = 0;
    infoList._buffer  = DDS_SampleInfoSeq_allocbuf(1);

    rql._release = TRUE;
    rql._maximum = 1;
    rql._length  = 0;
    rql._buffer  = DDS_sequence_DDSTouchStone_receiverQos_allocbuf(1);

    retcode = DDSTouchStone_receiverQosDataReader_take_w_condition (
                   rqr, &rql, &infoList, 1, _this->query);

    if ((retcode != DDS_RETCODE_OK) && (retcode != DDS_RETCODE_NO_DATA)) {
        DDS_ERROR(_this->config.partition_id,
                  _this->config.receiver_id,
                  "receiver_set_qos: "
                  "read receiver Qos policies failed",
                  retcode);
    }

    if (retcode == DDS_RETCODE_NO_DATA) {
        rqm.group_id = group_id;
        rqm.receiver_id = _this->config.receiver_id;
        rqm.partition_id   = _this->config.partition_id;
        set_touchstone_datareader_qos_default(&rqm.qos);
        retcode = DDSTouchStone_receiverQosDataWriter_write (
                      rqw,
                      &rqm,
                      DDS_HANDLE_NIL);

        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.receiver_id,
                      "receiver_set_qos: "
                      "DDSTouchStone_receiverQosDataWriter_write",
                      retcode);
        }
    }


    if (_this->rQos == NULL) {
        _this->rQos = DDS_DataReaderQos__alloc();
        retcode = DDS_Subscriber_get_default_datareader_qos(
                     _this->owner->subscriber,
                     _this->rQos);
        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.receiver_id,
                      "receiver_set_qos: "
                      "get_default_datareader_qos failed",
                      retcode);
        }
    }
    tQos = DDS_TopicQos__alloc();
    retcode = DDS_Topic_get_qos(_this->topic, tQos);
    if (retcode != DDS_RETCODE_OK) {
        DDS_ERROR(_this->config.partition_id,
                  _this->config.receiver_id,
                  "receiver_set_qos: "
                  "DDS_Topic_get_qos failed",
                  retcode);
    }

    retcode = DDS_Subscriber_copy_from_topic_qos(
                  _this->owner->subscriber, _this->rQos, tQos);
    if (retcode != DDS_RETCODE_OK) {
        DDS_ERROR(_this->config.partition_id,
                  _this->config.receiver_id,
                  "receiver_set_qos: "
                  "DDS_Subscriber_copy_from_topic_qos failed",
                  retcode);
    }
    DDS_free(tQos);

    if (rql._length == 1) {
        qos = &rql._buffer[0].qos;
	changed = copy_touchstone_datareader_qos(_this->rQos, qos);
        if (changed) {
	    if (_this->active) {
                result = receiver_start_readerThread (_this);
                if (result != touchstone_os_resultSuccess) {
                    OS_ERROR(_this->config.partition_id,
                             _this->config.receiver_id,
                             "receiver_set_qos: "
                             "Thread creation failed",
                             result);
                }
            }
        } else {
            if (_this->reader) {
                retcode = DDS_DataReader_set_qos(_this->reader,_this->rQos);
                if (retcode != DDS_RETCODE_OK) {
                    DDS_ERROR(_this->config.partition_id,
                              _this->config.receiver_id,
                              "receiver_set_qos: "
                              "DataReader_set_qos failed",
                              retcode);
                }
            }
        }
        _this->config_number++;
    } else if (rql._length > 1) {
        ENTITY_ERROR(_this->config.partition_id,
              _this->config.receiver_id,
              "receiver_set_qos: "
              "too many messages received");
    }
}

static void
dispose_receiverDef (
    DDSTouchStone_receiverDef *msg)
{
    participant p;
    receiver _this;
    receiver *ref;
    touchstone_os_result result;

    p = lookup_participant(msg->partition_id);
    if (p != NULL) {
        _this = lookup_receiver(p,msg->receiver_id);
        if (_this != NULL) {
            _this->active = FALSE;
            result = touchstone_os_threadWaitExit(_this->reportThreadId, NULL);
            if (result != touchstone_os_resultSuccess) {
                OS_ERROR(msg->partition_id,
                         msg->receiver_id,
                         "dispose_receiverDef: "
                         "touchstone_os_threadWaitExit report thread failed",
                         result);
            }
            result = touchstone_os_threadWaitExit(_this->threadId, NULL);
            if (result != touchstone_os_resultSuccess) {
                OS_ERROR(msg->partition_id,
                         msg->receiver_id,
                         "dispose_receiverDef: "
                         "touchstone_os_threadWaitExit receiver thread failed",
                         result);
            }
            DDS_DataReader_delete_readcondition(rqr, _this->query);
            DDS_free(_this->rQos);
            DDS_free(_this->waitset);
            receiver_del_topic(_this);
            ref = &p->receivers;
            while ((*ref != NULL) && (*ref != _this)) {
                ref = &(*ref)->next;
            }
            assert(*ref == _this);
            *ref = _this->next;
            _this->next = NULL;
            free(_this);
        }
    }
}

static void
process_receiverDef (
    DDSTouchStone_receiverDef *msg)
{
    participant p;
    receiver _this;
    DDS_StringSeq ReceiverId;
    touchstone_os_result result;
    char id[32];
    DDSTouchStone_timestamp timestamp;

    p = lookup_participant(msg->partition_id);
    if (p == NULL) {
        p = create_participant(msg->partition_id);
        _this = NULL;
    } else {
        _this = lookup_receiver(p,msg->receiver_id);
    }
    if (p != NULL) {
        if (_this == NULL) {
            activate_receiver_report();
            sprintf(id,"%d",msg->receiver_id);

            ReceiverId._maximum   = 1;
            ReceiverId._length    = 1;
            ReceiverId._buffer    = DDS_StringSeq_allocbuf(1);
            ReceiverId._buffer[0] = DDS_string_dup(id);

            _this = (receiver)malloc(sizeof(*_this));

            _this->config = *msg;
            _this->config_number = 0;
            _this->topic = NULL;
            _this->rQos = NULL;
            _this->reader = NULL;

            memset(&(_this->threadId), 0, sizeof(_this->threadId));
            memset(&(_this->reportThreadId), 0, sizeof(_this->reportThreadId));

            _this->active = FALSE;
            _this->owner = p;
            _this->waitset = DDS_WaitSet__alloc ();
            _this->query  = DDS_DataReader_create_querycondition(
                                   rqr,
                                   DDS_ANY_SAMPLE_STATE,
                                   DDS_ANY_VIEW_STATE,
                                   DDS_ANY_INSTANCE_STATE,
                                   "receiver_id = %0",
                                   &ReceiverId);
            _this->alive_writers = NULL;
            _this->byte_count = 0;
            _this->msg_count = 0;
            receiver_set_topic(_this);
            receiver_set_qos (_this);

            result = receiver_start_readerThread(_this);
            if (result == touchstone_os_resultSuccess) {
                result = receiver_start_reportThread(_this);
            }
            if (result == touchstone_os_resultSuccess) {
                _this->next = p->receivers;
                p->receivers = _this;
            } else {
                _this->active = FALSE;
            }
        } else {
            if ((_this->config.scheduling_class != msg->scheduling_class) ||
                (_this->config.thread_priority != msg->thread_priority) ||
                (_this->config.topic_kind      != msg->topic_kind) ||
                (_this->config.topic_id        != msg->topic_id)) {

                _this->active = FALSE;

                result = touchstone_os_threadWaitExit(_this->reportThreadId, NULL);
                if (result != touchstone_os_resultSuccess) {
                    OS_ERROR(msg->partition_id,
                             msg->receiver_id,
                             "process_receiverDef: "
                             "touchstone_os_threadWaitExit report thread failed",
                             result);
                }
                result = touchstone_os_threadWaitExit(_this->threadId, NULL);
                if (result != touchstone_os_resultSuccess) {
                    OS_ERROR(msg->partition_id,
                             msg->receiver_id,
                             "process_receiverDef: "
                             "touchstone_os_threadWaitExit receiver thread failed",
                             result);
                }

                if ((_this->config.topic_kind != msg->topic_kind) ||
                    (_this->config.topic_id != msg->topic_id)) {
                    _this->config = *msg;
                    receiver_set_topic(_this);
                    receiver_set_qos (_this);
                }

                _this->config = *msg;
                _this->config_number++;

                result = receiver_start_readerThread(_this);
                if (result == touchstone_os_resultSuccess) {
                    result = receiver_start_reportThread(_this);
                }
                if (result != touchstone_os_resultSuccess) {
                    p->receivers = _this->next;
                    _this->next = NULL;
                    _this->active = FALSE;
                }
            } else {
                _this->config = *msg;
                _this->config_number++;
            }
        }
    }
}

static void
process_receiverQos (
    DDSTouchStone_receiverQos *msg)
{
    participant p;
    receiver t;

    p = lookup_participant(msg->partition_id);
    if (p != NULL) {
        t = lookup_receiver(p,msg->receiver_id);
        if (t != NULL) {
            receiver_set_qos(t);
        }
    }
}

/*****************************************************************************
 * Transceiver Implementation
 *****************************************************************************/

static transceiver
lookup_transceiver (
    participant p,
    unsigned long transceiver_id)
{
    transceiver t;

    t = p->transceivers;
    while ((t != NULL) &&
           (t->config.transceiver_id != transceiver_id)) {
        t = t->next;
    }
    return t;
}

static void
transceiver_del_writer (
    transceiver _this)
{
    DDS_ReturnCode_t retcode;

    if (_this->writer != NULL) {
        retcode = DDS_Publisher_delete_datawriter(
                      _this->owner->publisher,
                      _this->writer);
        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.transceiver_id,
                      "transceiver_del_writer: "
                      "delete DataWriter failed",
                      retcode);
        } else {
            _this->writer = NULL;
        }
    }
}

static void
transceiver_set_writer (
    transceiver _this)
{
    transceiver_del_writer(_this);
    _this->writer = DDS_Publisher_create_datawriter (
                           _this->owner->publisher,
                           _this->topic,
                           _this->wQos,
                           NULL,
                           DDS_ANY_STATUS );
    if (_this->writer == NULL) {
        ENTITY_ERROR(_this->config.partition_id,
              _this->config.transceiver_id,
              "transceiver_set_writer: "
              "create DataWriter failed");
    }
    assert(_this->writer);
}

static void
transceiver_del_reader (
    transceiver _this)
{
    DDS_ReturnCode_t retcode;

    if (_this->reader != NULL) {
        if (_this->condition) {
            retcode = DDS_WaitSet_detach_condition (
                             _this->waitset,
                             _this->condition);
            if (retcode != DDS_RETCODE_OK) {
                DDS_ERROR(_this->config.partition_id,
                          _this->config.transceiver_id,
                          "transceiver_del_reader: "
                          "Detach condition from waitset failed",
                          retcode);
            } else {
                _this->condition = NULL;
            }
        }
        retcode = DDS_Subscriber_delete_datareader(
                     _this->owner->subscriber,
                     _this->reader);

        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.transceiver_id,
                      "transceiver_del_reader: "
                      "delete DataReader failed",
                      retcode);
        } else {
            _this->reader = NULL;
        }
    }
}

static void
transceiver_set_reader (
    transceiver _this)
{
    DDS_ReturnCode_t retcode;

    transceiver_del_reader(_this);
    _this->reader = DDS_Subscriber_create_datareader (
                           _this->owner->subscriber,
                           _this->echo_topic,
                           _this->rQos,
                           NULL,
                           DDS_ANY_STATUS );
    if (_this->reader == NULL) {
        ENTITY_ERROR(_this->config.partition_id,
              _this->config.transceiver_id,
              "transceiver_set_reader:  "
              "create DataReader failed");
    }
    assert(_this->reader);

    _this->condition = DDS_DataReader_get_statuscondition (
                           _this->reader);

    if (_this->condition) {
        retcode = DDS_StatusCondition_set_enabled_statuses (
                     _this->condition,
                     DDS_DATA_AVAILABLE_STATUS);

        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.transceiver_id,
                      "transceiver_set_reader: "
                      "set_enabled_statuses failed",
                      retcode);
        }

        retcode = DDS_WaitSet_attach_condition (
                     _this->waitset,
                     _this->condition);

        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.transceiver_id,
                      "transceiver_set_reader: "
                      "attach_condition failed",
                      retcode);
        }
    }
}

void *
transceiverWriterThread(
    void *arg)
{
    transceiver _this = (transceiver)arg;
    DDSTouchStone_latency_message *message;
    DDS_ReturnCode_t retcode;
    touchstone_os_time interval;
    unsigned int i, size, last_size;
    unsigned int last_write_period;

    transceiver_set_writer(_this);

    message = DDSTouchStone_latency_message__alloc();
    message->application_id = application_id;
    /* message->random_id = random_id; */
    message->random_id = get_random_value();
    message->transceiver_id = _this->config.transceiver_id;
    message->sequence_number = 0;
    message->payload_data._buffer = DDSTouchStone_payload_allocbuf(0);
    message->payload_data._length = 0;
    message->payload_data._maximum = 0;
    last_size = 0;

    retcode = DDS_RETCODE_OK;
    last_write_period = _this->config.write_period;
    touchstone_os_timerStart(_this->timer);
    while (_this->writerActive) {
        if (_this->config.write_period != last_write_period) {
            last_write_period = _this->config.write_period;
            interval.tv_sec = last_write_period / 1000;
            interval.tv_nsec = (last_write_period % 1000) * 1000000;
            touchstone_os_timerIntervalSet(
                _this->timer, interval);
        }
        if (_this->config.message_size != last_size) {
            size = sizeof(DDSTouchStone_latency_message);
            if (_this->config.message_size > size) {
                size = _this->config.message_size - size;
            } else {
                size = 1;
            }
            last_size = _this->config.message_size;
            DDS_free(message->payload_data._buffer);
            message->payload_data._buffer = DDSTouchStone_payload_allocbuf(size);
            message->payload_data._length = size;
            message->payload_data._maximum = size;
            for (i=0; i<size; i++) {
                message->payload_data._buffer[i] = PAYLOAD_ITEM_FROM_UINT(i);
            }
        }
        message->sequence_number++;
        message->config_number = _this->config_number;
        message->write_timestamp = get_timestamp();
        retcode = DDSTouchStone_latency_messageDataWriter_write (
                     _this->writer,
                     message,
                     DDS_HANDLE_NIL);
        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.transceiver_id,
                      "transceiverThread:"
                      "DDSTouchStone_latency_messageDataWriter_write",
                      retcode);
        }
        /* msec_sleep(_this->config.write_period); */
        touchstone_os_timerWait(_this->timer);

    }
    DDS_free (message);
    transceiver_del_writer(_this);
    return (void *)retcode;
}

static touchstone_os_result
transceiver_start_writerThread (
    transceiver _this)
{
    touchstone_os_threadAttr threadAttr;
    touchstone_os_result result;
    char name[32];

    sprintf(name,"transceiver_%d",_this->config.transceiver_id);

    if (_this->writerActive) {
        _this->writerActive = FALSE;
        result = touchstone_os_threadWaitExit(_this->writerThreadId, NULL);
        if (result != touchstone_os_resultSuccess) {
            OS_ERROR(_this->config.partition_id,
                     _this->config.transceiver_id,
                     "transceiver_start_writerThread: "
                     "Thread wait exit failed",
                     result);
        }
    }

    _this->writerActive = TRUE;

    result = threadAttrInit(&threadAttr,
                   _this->config.scheduling_class,
                   _this->config.thread_priority);

    if (result == touchstone_os_resultSuccess) {
        result = touchstone_os_threadCreate(
                       &_this->writerThreadId,
                       name,
                       &threadAttr,
                       transceiverWriterThread,
                       _this);
        if (result != touchstone_os_resultSuccess) {
            OS_ERROR(_this->config.partition_id,
                     _this->config.transceiver_id,
                     "transceiver_start_writerThread: "
                     "touchstone_os_threadCreate failed",
                     result);
        }
    } else {
        OS_ERROR(_this->config.partition_id,
                 _this->config.transceiver_id,
                 "transceiver_start_writerThread: "
                 "threadAttrInit failed",
                 result);
    }
    return result;
}

void *
transceiverReaderThread(
    void *arg)
{
    transceiver _this = (transceiver)arg;
    DDS_sequence_DDSTouchStone_latency_message dataList;
    DDS_SampleInfoSeq infoList;
    DDS_ReturnCode_t retcode;
    DDS_ConditionSeq *conditionList;
    DDS_Duration_t wait_timeout = DELAY;
    unsigned int i;

    DDSTouchStone_timestamp readTime, writeTime, echoTime,
                     prevTime, sourceTime, arrivalTime;

    dataList._buffer = DDS_sequence_DDSTouchStone_latency_message_allocbuf(1);
    dataList._length = 0;
    dataList._maximum = 1;
    dataList._release = 1;

    infoList._buffer = DDS_SampleInfoSeq_allocbuf(1);
    infoList._length = 0;
    infoList._maximum = 1;
    infoList._release = 1;

    prevTime = 0;

    transceiver_set_reader(_this);

    retcode = DDS_RETCODE_OK;
    conditionList = DDS_ConditionSeq__alloc();
    while (_this->readerActive) {
        retcode = DDS_WaitSet_wait (_this->waitset,
                                   conditionList,
                                   &wait_timeout);
        retcode = DDSTouchStone_latency_messageDataReader_take (
                     _this->reader,
                     &dataList,
                     &infoList,
                     1,
                     DDS_ANY_SAMPLE_STATE,
                     DDS_ANY_VIEW_STATE,
                     DDS_ANY_INSTANCE_STATE);

        readTime = get_timestamp();

        if (retcode == DDS_RETCODE_OK) {
            for (i=0; i<dataList._length; i++) {
                writeTime = dataList._buffer[i].write_timestamp;
                echoTime = dataList._buffer[i].echo_timestamp;
                metrics_add_stamp(_this->send_latency,
                            dataList._buffer[i].send_latency);
                metrics_add_stamp(_this->echo_latency, (readTime-echoTime));
                metrics_add_stamp(_this->trip_latency, (readTime-writeTime));

                sourceTime = get_timestamp_from_dds_time(
                                 &infoList._buffer[i].source_timestamp);
                arrivalTime = get_timestamp_from_dds_time(
                                 &infoList._buffer[i].reception_timestamp);
                metrics_add_stamp(_this->send_source_latency,
                            dataList._buffer[i].source_latency);
                metrics_add_stamp(_this->send_arrival_latency,
                            dataList._buffer[i].arrival_latency);
                metrics_add_stamp(_this->send_trip_latency,
                            dataList._buffer[i].send_latency -
                            dataList._buffer[i].source_latency -
                            dataList._buffer[i].arrival_latency);
                metrics_add_stamp(_this->echo_source_latency,
                            (sourceTime-echoTime));
                metrics_add_stamp(_this->echo_arrival_latency,
                            (readTime-arrivalTime));
                metrics_add_stamp(_this->echo_trip_latency,
                            (arrivalTime-sourceTime));
#if 0 /* must keep track of all producers */
                if (dataList._buffer[i].config_number != config_number) {
                    config_number = dataList._buffer[i].config_number;
                    _this->config_number++;
                }
#endif
            }
            if (prevTime != 0) {
                metrics_add_stamp(_this->inter_arrival_time, (readTime-prevTime));
            }
            prevTime = readTime;
        } else if (retcode != DDS_RETCODE_NO_DATA) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.transceiver_id,
                      "transceiverThread:"
                      "DDSTouchStone_latency_messageDataReader_take",
                      retcode);
            prevTime = 0;
        }
    }
    DDS_free(conditionList);
    transceiver_del_reader(_this);

    return (void *)retcode;
}

static touchstone_os_result
transceiver_start_readerThread (
    transceiver _this)
{
    touchstone_os_threadAttr threadAttr;
    touchstone_os_result result;
    char name[32];

    sprintf(name,"transceiver_%d",_this->config.transceiver_id);

    if (_this->readerActive) {
        _this->readerActive = FALSE;
        result = touchstone_os_threadWaitExit(_this->readerThreadId, NULL);
        if (result != touchstone_os_resultSuccess) {
            OS_ERROR(_this->config.partition_id,
                     _this->config.transceiver_id,
                     "transceiver_start_readerThread: "
                     "Thread wait exit failed",
                     result);
        }
    }

    _this->readerActive = TRUE;

    result = threadAttrInit(&threadAttr,
                   _this->config.scheduling_class,
                   _this->config.thread_priority);

    if (result == touchstone_os_resultSuccess) {
        result = touchstone_os_threadCreate(
                       &_this->readerThreadId,
                       name,
                       &threadAttr,
                       transceiverReaderThread,
                       _this);
        if (result != touchstone_os_resultSuccess) {
            OS_ERROR(_this->config.partition_id,
                     _this->config.transceiver_id,
                     "transceiver_start_readerThread: "
                     "touchstone_os_threadCreate failed",
                     result);
        }
    } else {
        OS_ERROR(_this->config.partition_id,
                 _this->config.transceiver_id,
                 "transceiver_start_readerThread: "
                 "threadAttrInit failed",
                 result);
    }
    return result;
}

void *
transceiverReportThread(
    void *arg)
{
    transceiver _this = (transceiver)arg;
    DDS_SampleLostStatus sl_status;
    DDS_SampleRejectedStatus sr_status;
    DDS_RequestedDeadlineMissedStatus rdm_status;
    DDS_OfferedDeadlineMissedStatus odm_status;
    DDSTouchStone_transceiverReport report;
    DDS_ReturnCode_t retcode;

    report.application_id = application_id;
    report.partition_id = _this->config.partition_id;
    report.transceiver_id = _this->config.transceiver_id;

    retcode = DDS_RETCODE_OK;
    while (_this->reportActive) {
        report.reader_status.samples_lost = 0;
        report.reader_status.samples_rejected = 0;
        report.reader_status.deadlines_missed = 0;
        report.writer_status.deadlines_missed = 0;

        _this->send_latency = metrics_new();
        _this->echo_latency = metrics_new();
        _this->trip_latency = metrics_new();
        _this->send_source_latency = metrics_new();
        _this->send_arrival_latency = metrics_new();
        _this->send_trip_latency = metrics_new();
        _this->echo_source_latency = metrics_new();
        _this->echo_arrival_latency = metrics_new();
        _this->echo_trip_latency = metrics_new();
        _this->inter_arrival_time = metrics_new();

        msec_sleep(_this->config.report_period);

        metrics_analyse(_this->send_latency,&report.send_latency[0]);
        metrics_analyse(_this->echo_latency,&report.echo_latency[0]);
        metrics_analyse(_this->trip_latency,&report.trip_latency[0]);
        metrics_analyse(_this->send_source_latency,
                        &report.send_source_latency[0]);
        metrics_analyse(_this->send_arrival_latency,
                        &report.send_arrival_latency[0]);
        metrics_analyse(_this->send_trip_latency,
                        &report.send_trip_latency[0]);
        metrics_analyse(_this->echo_source_latency,
                        &report.echo_source_latency[0]);
        metrics_analyse(_this->echo_arrival_latency,
                        &report.echo_arrival_latency[0]);
        metrics_analyse(_this->echo_trip_latency,
                        &report.echo_trip_latency[0]);
        metrics_analyse(_this->inter_arrival_time,
                        &report.inter_arrival_time[0]);

        if (_this->reader) {
            retcode = DDS_DataReader_get_sample_lost_status(
                         _this->reader, &sl_status);
            if (retcode != DDS_RETCODE_OK) {
                DDS_ERROR(_this->config.partition_id,
                          _this->config.transceiver_id,
                          "transceiverReportThread: "
                          "get_sample_lost_status failed",
                          retcode);
            } else {
                report.reader_status.samples_lost =
                       sl_status.total_count_change;
            }
            retcode = DDS_DataReader_get_sample_rejected_status(
                         _this->reader, &sr_status);
            if (retcode != DDS_RETCODE_OK) {
                DDS_ERROR(_this->config.partition_id,
                          _this->config.transceiver_id,
                          "transceiverReportThread: "
                          "get_sample_rejected_status failed",
                          retcode);
            } else {
                report.reader_status.samples_rejected =
                       sr_status.total_count_change;
            }
            retcode = DDS_DataReader_get_requested_deadline_missed_status(
                         _this->reader, &rdm_status);
            if (retcode != DDS_RETCODE_OK) {
                DDS_ERROR(_this->config.partition_id,
                          _this->config.transceiver_id,
                          "transceiverReportThread: "
                          "get_requested_deadline_missed_status failed",
                          retcode);
            } else {
                report.reader_status.deadlines_missed =
                       rdm_status.total_count_change;
            }
        }
        if (_this->writer) {
            retcode = DDS_DataWriter_get_offered_deadline_missed_status(
                         _this->writer, &odm_status);
            if (retcode != DDS_RETCODE_OK) {
                DDS_ERROR(_this->config.partition_id,
                          _this->config.transceiver_id,
                          "transceiverReportThread: "
                          "get_offered_deadline_missed_status failed",
                          retcode);
            } else if (odm_status.total_count_change > 0) {
                report.writer_status.deadlines_missed =
                       odm_status.total_count_change;
            }
        }
        report.config_number = _this->config_number;
        retcode = DDSTouchStone_transceiverReportDataWriter_write (
                     srw,
                     &report,
                     DDS_HANDLE_NIL);
        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.transceiver_id,
                      "transceiverReportThread: "
                      "DDSTouchStone_transceiverReportDataWriter_write failed",
                      retcode);
        }
    }
    touchstone_os_threadExit((void *)retcode);
    return NULL;
}

static touchstone_os_result
transceiver_start_reportThread (
    transceiver _this)
{
    touchstone_os_threadAttr threadAttr;
    touchstone_os_result result;
    char name[32];

    sprintf(name,"transceiver_%d",_this->config.transceiver_id);

    _this->reportActive = TRUE;

    result = threadAttrInit(&threadAttr,
                   _this->config.scheduling_class,
                   _this->config.thread_priority);

    if (result == touchstone_os_resultSuccess) {
        result = touchstone_os_threadCreate(
                           &_this->reportThreadId,
                           name,
                           &threadAttr,
                           transceiverReportThread,
                           _this);
        if (result != touchstone_os_resultSuccess) {
            OS_ERROR(_this->config.partition_id,
                     _this->config.transceiver_id,
                     "transceiver_start_reportThread: "
                     "touchstone_os_threadCreate failed",
                     result);
        }
    } else {
        OS_ERROR(_this->config.partition_id,
                 _this->config.transceiver_id,
                 "transceiver_start_reportThread: "
                 "threadAttrInit failed",
                 result);
    }
    return result;
}

static void
transceiver_set_qos (
    transceiver _this)
{
    DDS_sequence_DDSTouchStone_transceiverQos sql;
    DDS_SampleInfoSeq infoList;
    DDS_ReturnCode_t retcode;
    DDS_TopicQos *tQos;
    DDSTouchStone_DataWriterQos *wQos;
    DDSTouchStone_DataReaderQos *rQos;
    DDSTouchStone_transceiverQos sqm;
    touchstone_os_result result;
    int changed;

    infoList._release = TRUE;
    infoList._maximum = 1;
    infoList._length  = 0;
    infoList._buffer  = DDS_SampleInfoSeq_allocbuf(1);

    sql._release = TRUE;
    sql._maximum = 1;
    sql._length  = 0;
    sql._buffer  = DDS_sequence_DDSTouchStone_transceiverQos_allocbuf(1);

    retcode = DDSTouchStone_transceiverQosDataReader_take_w_condition (
                   sqr, &sql, &infoList, 1, _this->query);

    if ((retcode != DDS_RETCODE_OK) && (retcode != DDS_RETCODE_NO_DATA)) {
        DDS_ERROR(_this->config.partition_id,
                  _this->config.transceiver_id,
                  "transceiver_set_qos: "
                  "read transceiver Qos policies failed",
                  retcode);
    }

    if (retcode == DDS_RETCODE_NO_DATA) {
        sqm.group_id = group_id;
        sqm.transceiver_id = _this->config.transceiver_id;
        sqm.partition_id   = _this->config.partition_id;
        set_touchstone_datareader_qos_default(&sqm.reader_qos);
        set_touchstone_datawriter_qos_default(&sqm.writer_qos);

        retcode = DDSTouchStone_transceiverQosDataWriter_write (
                      sqw,
                      &sqm,
                      DDS_HANDLE_NIL);

        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.transceiver_id,
                      "transceiver_set_qos: "
                      "DDSTouchStone_transceiverQosDataWriter_write",
                      retcode);
        }
    }

    tQos = DDS_TopicQos__alloc();
    retcode = DDS_Topic_get_qos(_this->topic, tQos);
    if (retcode != DDS_RETCODE_OK) {
        DDS_ERROR(_this->config.partition_id,
                  _this->config.transceiver_id,
                  "transceiver_set_qos: "
                  "DDS_Topic_get_qos failed",
                  retcode);
    }

    if (_this->wQos == NULL) {
        _this->wQos = DDS_DataWriterQos__alloc();
        retcode = DDS_Publisher_get_default_datawriter_qos(
                     _this->owner->publisher,
                     _this->wQos);
        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.transceiver_id,
                      "transceiver_set_qos: "
                      "get_default_datawriter_qos failed",
                      retcode);
        }
    }
    retcode = DDS_Publisher_copy_from_topic_qos(
                  _this->owner->publisher, _this->wQos, tQos);
    if (retcode != DDS_RETCODE_OK) {
        DDS_ERROR(_this->config.partition_id,
                  _this->config.transceiver_id,
                  "transceiver_set_qos: "
                  "DDS_Publisher_copy_from_topic_qos failed",
                  retcode);
    }

    if (_this->rQos == NULL) {
        _this->rQos = DDS_DataReaderQos__alloc();

        retcode = DDS_Subscriber_get_default_datareader_qos(
                     _this->owner->subscriber,
                     _this->rQos);
        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.transceiver_id,
                      "transceiver_set_qos: "
                      "get_default_datareader_qos failed",
                      retcode);
        }
    }
    retcode = DDS_Subscriber_copy_from_topic_qos(
                  _this->owner->subscriber, _this->rQos, tQos);
    if (retcode != DDS_RETCODE_OK) {
        DDS_ERROR(_this->config.partition_id,
                  _this->config.transceiver_id,
                  "transceiver_set_qos: "
                  "DDS_Subscriber_copy_from_topic_qos failed",
                  retcode);
    }
    DDS_free(tQos);

    if (sql._length == 1) {
        wQos = &sql._buffer[0].writer_qos;
	changed = copy_touchstone_datawriter_qos(_this->wQos, wQos);
        if (changed) {
	    if (_this->writerActive) {
                result = transceiver_start_writerThread (_this);
                if (result != touchstone_os_resultSuccess) {
                    OS_ERROR(_this->config.partition_id,
                             _this->config.transceiver_id,
                             "transceiver_set_qos: "
                             "Thread creation failed",
                             result);
                }
            }
        } else {
            if (_this->writer) {
                retcode = DDS_DataWriter_set_qos(_this->writer,_this->wQos);
                if (retcode != DDS_RETCODE_OK) {
                    DDS_ERROR(_this->config.partition_id,
                              _this->config.transceiver_id,
                              "transceiver_set_qos: "
                              "DataWriter_set_qos failed",
                              retcode);
                }
            }
        }


        rQos = &sql._buffer[0].reader_qos;
	changed = copy_touchstone_datareader_qos(_this->rQos, rQos);
        if (changed) {
	    if (_this->readerActive) {
                result = transceiver_start_readerThread (_this);
                if (result != touchstone_os_resultSuccess) {
                    OS_ERROR(_this->config.partition_id,
                             _this->config.transceiver_id,
                             "transceiver_set_qos: "
                             "Thread creation failed",
                             result);
                }
            }
        } else {
            if (_this->reader) {
                retcode = DDS_DataReader_set_qos(_this->reader,_this->rQos);
                if (retcode != DDS_RETCODE_OK) {
                    DDS_ERROR(_this->config.partition_id,
                              _this->config.transceiver_id,
                              "transceiver_set_qos: "
                              "DataReader_set_qos failed",
                              retcode);
                }
            }
        }
        _this->config_number++;
    } else if (sql._length > 1) {
        ENTITY_ERROR(_this->config.partition_id,
              _this->config.transceiver_id,
                  "transceiver_set_qos: "
                  "too many messages received");
    }
}

static void
transceiver_del_topics (
    transceiver _this)
{
    DDS_ReturnCode_t retcode;

    if (_this->topic != NULL) {
        retcode = DDS_DomainParticipant_delete_topic(
                      _this->owner, _this->topic);
        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.transceiver_id,
                      "transceiver_del_topic: "
                      "delete Topic failed",
                      retcode);
        } else {
            _this->topic = NULL;
        }
    }
    if (_this->echo_topic != NULL) {
        retcode = DDS_DomainParticipant_delete_topic(
                      _this->owner, _this->echo_topic);
        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.transceiver_id,
                      "transceiver_del_topic: "
                      "delete Topic failed",
                      retcode);
        } else {
            _this->echo_topic = NULL;
        }
    }
}

static void
transceiver_set_topics (
    transceiver _this)
{
    DDS_TopicQos *tQos;
    DDS_ReturnCode_t retcode;
    char id[32];

    tQos = DDS_TopicQos__alloc();

    retcode = DDS_DomainParticipant_get_default_topic_qos(
                  _this->owner->domain_participant,
                  tQos);

    if (retcode == DDS_RETCODE_OK) {
        set_topic_qos(tQos, _this->config.topic_kind);
        set_topic_name(id,"LatencyTopic",
                       _this->config.topic_id,
                       _this->config.topic_kind);
        transceiver_del_topics(_this);
        _this->topic =
            DDS_DomainParticipant_create_topic (
                      _this->owner->domain_participant,
                      id,
                      "DDSTouchStone::latency_message",
                      tQos,
                      NULL,
                      DDS_ANY_STATUS);

        set_topic_name(id,"LatencyEchoTopic",
                       _this->config.topic_id,
                       _this->config.topic_kind);
        _this->echo_topic =
            DDS_DomainParticipant_create_topic (
                      _this->owner->domain_participant,
                      id,
                      "DDSTouchStone::latency_message",
                      tQos,
                      NULL,
                      DDS_ANY_STATUS);

        DDS_free(tQos);

        if (_this->topic == DDS_HANDLE_NIL) {
            ENTITY_ERROR(_this->config.partition_id,
                  _this->config.transceiver_id,
                  "transceiver_set_topics: "
                  "create_topic failed");
        }
    } else {
        DDS_ERROR(_this->config.partition_id,
                  _this->config.transceiver_id,
                  "transceiver_set_topics: "
                  "get_default_topic_qos failed",
                  retcode);
    }
}

static void
dispose_transceiverDef (
    DDSTouchStone_transceiverDef *msg)
{
    participant p;
    transceiver _this;
    transceiver *ref;
    touchstone_os_result result;

    p = lookup_participant(msg->partition_id);
    if (p != NULL) {
        _this = lookup_transceiver(p,msg->transceiver_id);
        if (_this != NULL) {
            _this->reportActive = FALSE;
            result = touchstone_os_threadWaitExit(_this->reportThreadId, NULL);
            if (result != touchstone_os_resultSuccess) {
                OS_ERROR(msg->partition_id,
                         msg->transceiver_id,
                         "dispose_transceiverDef: "
                         "touchstone_os_threadWaitExit failed for reportThread",
                         result);
            }
            _this->readerActive = FALSE;
            result = touchstone_os_threadWaitExit(_this->readerThreadId, NULL);
            if (result != touchstone_os_resultSuccess) {
                OS_ERROR(msg->partition_id,
                         msg->transceiver_id,
                         "dispose_transceiverDef: "
                         "touchstone_os_threadWaitExit failed for readerThread",
                         result);
            }
            _this->writerActive = FALSE;
            result = touchstone_os_threadWaitExit(_this->writerThreadId, NULL);
            if (result != touchstone_os_resultSuccess) {
                OS_ERROR(msg->partition_id,
                         msg->transceiver_id,
                         "dispose_transceiverDef: "
                         "touchstone_os_threadWaitExit failed for writerThread",
                         result);
            }
            DDS_DataReader_delete_readcondition(sqr, _this->query);
            DDS_free(_this->wQos);
            DDS_free(_this->rQos);
            DDS_free(_this->waitset);
            transceiver_del_topics(_this);
            ref = &p->transceivers;
            while ((*ref != NULL) && (*ref != _this)) {
                ref = &(*ref)->next;
            }
            assert(*ref == _this);
            *ref = _this->next;
            _this->next = NULL;
            free(_this);
        }
    }
}

static void
process_transceiverDef (
    DDSTouchStone_transceiverDef *msg)
{
    participant p;
    transceiver _this;
    DDS_StringSeq TransceiverId;
    touchstone_os_result result;
    touchstone_os_time interval;
    char id[32];

    p = lookup_participant(msg->partition_id);
    if (p == NULL) {
        p = create_participant(msg->partition_id);
        _this = NULL;
    } else {
        _this = lookup_transceiver(p,msg->transceiver_id);
    }
    if (p != NULL) {
        if (_this == NULL) {
            activate_transceiver_report();
            sprintf(id,"%d",msg->transceiver_id);

            TransceiverId._maximum   = 1;
            TransceiverId._length    = 1;
            TransceiverId._buffer    = DDS_StringSeq_allocbuf(1);
            TransceiverId._buffer[0] = DDS_string_dup(id);

            _this = (transceiver)malloc(sizeof(*_this));

            _this->config = *msg;
            _this->config_number = 0;
            _this->topic = NULL;
            _this->echo_topic = NULL;
            _this->wQos = NULL;
            _this->rQos = NULL;
            _this->writer = NULL;
            _this->reader = NULL;

            memset(&(_this->writerThreadId), 0, sizeof(_this->writerThreadId));
            memset(&(_this->readerThreadId), 0, sizeof(_this->readerThreadId));
            memset(&(_this->reportThreadId), 0, sizeof(_this->reportThreadId));

            interval.tv_sec = _this->config.write_period / 1000;
            interval.tv_nsec = (_this->config.write_period % 1000) * 1000000;
            _this->timer = touchstone_os_timerCreate(interval);
            _this->writerActive = FALSE;
            _this->readerActive = FALSE;
            _this->reportActive = FALSE;
            _this->owner = p;
            _this->waitset = DDS_WaitSet__alloc ();
            _this->query  = DDS_DataReader_create_querycondition(
                               sqr,
                               DDS_ANY_SAMPLE_STATE,
                               DDS_ANY_VIEW_STATE,
                               DDS_ANY_INSTANCE_STATE,
                               "transceiver_id = %0",
                               &TransceiverId);

            transceiver_set_topics(_this);
            transceiver_set_qos (_this);

            _this->send_latency = metrics_new();
            _this->echo_latency = metrics_new();
            _this->trip_latency = metrics_new();
            _this->send_source_latency = metrics_new();
            _this->send_arrival_latency = metrics_new();
            _this->send_trip_latency = metrics_new();
            _this->echo_source_latency = metrics_new();
            _this->echo_arrival_latency = metrics_new();
            _this->echo_trip_latency = metrics_new();
            _this->inter_arrival_time = metrics_new();

            result = transceiver_start_writerThread (_this);
            if (result == touchstone_os_resultSuccess) {
                result = transceiver_start_readerThread (_this);
                if (result == touchstone_os_resultSuccess) {
                    result = transceiver_start_reportThread (_this);
                }
            }
            if (result == touchstone_os_resultSuccess) {
                _this->next = p->transceivers;
                p->transceivers = _this;
            } else {
                _this->writerActive = FALSE;
                _this->readerActive = FALSE;
                _this->reportActive = FALSE;
            }
        } else {
            if ((_this->config.scheduling_class != msg->scheduling_class) ||
                (_this->config.thread_priority != msg->thread_priority) ||
                (_this->config.topic_kind      != msg->topic_kind) ||
                (_this->config.topic_id        != msg->topic_id)) {

                _this->writerActive = FALSE;
                _this->readerActive = FALSE;
                _this->reportActive = FALSE;
                result = touchstone_os_threadWaitExit(_this->writerThreadId, NULL);
                if (result != touchstone_os_resultSuccess) {
                    OS_ERROR(msg->partition_id,
                             msg->transceiver_id,
                             "process_transceiverDef: "
                             "touchstone_os_threadWaitExit failed for reportThread",
                             result);
                }
                result = touchstone_os_threadWaitExit(_this->readerThreadId, NULL);
                if (result != touchstone_os_resultSuccess) {
                    OS_ERROR(msg->partition_id,
                             msg->transceiver_id,
                             "process_transceiverDef: "
                             "touchstone_os_threadWaitExit failed for readerThread",
                             result);
                }
                result = touchstone_os_threadWaitExit(_this->reportThreadId, NULL);
                if (result != touchstone_os_resultSuccess) {
                    OS_ERROR(msg->partition_id,
                             msg->transceiver_id,
                             "process_transceiverDef: "
                             "touchstone_os_threadWaitExit failed for reportThread",
                             result);
                }

                if ((_this->config.topic_kind != msg->topic_kind) ||
                    (_this->config.topic_id) != (msg->topic_id)) {
                    _this->config = *msg;
                    transceiver_set_topics(_this);
                    transceiver_set_qos (_this);
                }

                _this->config = *msg;
                _this->config_number++;

                result = transceiver_start_writerThread(_this);
                if (result == touchstone_os_resultSuccess) {
                    result = transceiver_start_readerThread(_this);
                    if (result == touchstone_os_resultSuccess) {
                        result = transceiver_start_reportThread(_this);
                    }
                }
                if (result != touchstone_os_resultSuccess) {
                    p->transceivers = _this->next;
                    _this->next = NULL;
                    _this->writerActive = FALSE;
                    _this->readerActive = FALSE;
                    _this->reportActive = FALSE;
                }
            } else {
                _this->config = *msg;
                _this->config_number++;
            }
        }
    }
}

static void
process_transceiverQos (
    DDSTouchStone_transceiverQos *msg)
{
    participant p;
    transceiver t;

    p = lookup_participant(msg->partition_id);
    if (p != NULL) {
        t = lookup_transceiver(p,msg->transceiver_id);
        if (t != NULL) {
            transceiver_set_qos(t);
        }
    }
}

/*****************************************************************************
 * Transponder Implementation
 *****************************************************************************/

static transponder
lookup_transponder (
    participant p,
    unsigned long transponder_id)
{
    transponder t;

    t = p->transponders;
    while ((t != NULL) &&
           (t->config.transponder_id != transponder_id)) {
        t = t->next;
    }
    return t;
}

static void
transponder_del_writer (
    transponder _this)
{
    DDS_ReturnCode_t retcode;

    if (_this->writer != NULL) {
        retcode = DDS_Publisher_delete_datawriter(
                      _this->owner->publisher,
                      _this->writer);
        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.transponder_id,
                      "transponder_del_writer: "
                      "delete DataWriter failed",
                      retcode);
        } else {
            _this->writer = NULL;
        }
    }
}

static void
transponder_set_writer (
    transponder _this)
{
    transponder_del_writer(_this);
    _this->writer = DDS_Publisher_create_datawriter (
                           _this->owner->publisher,
                           _this->echo_topic,
                           _this->wQos,
                           NULL,
                           DDS_ANY_STATUS );
    if (_this->writer == NULL) {
        ENTITY_ERROR(_this->config.partition_id,
              _this->config.transponder_id,
              "transponder_set_writer: "
              "create DataWriter failed");
    }
    assert(_this->writer);
}

static void
transponder_del_reader (
    transponder _this)
{
    DDS_ReturnCode_t retcode;

    if (_this->reader != NULL) {
        if (_this->condition) {
            retcode = DDS_WaitSet_detach_condition (
                             _this->waitset,
                             _this->condition);
            if (retcode != DDS_RETCODE_OK) {
                DDS_ERROR(_this->config.partition_id,
                          _this->config.transponder_id,
                          "transponder_del_reader: "
                          "Detach condition from waitset failed",
                          retcode);
            } else {
                _this->condition = NULL;
            }
        }
        retcode = DDS_Subscriber_delete_datareader(
                     _this->owner->subscriber,
                     _this->reader);

        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.transponder_id,
                      "transponder_set_reader: "
                      "delete DataReader failed",
                      retcode);
        } else {
            _this->reader = NULL;
        }
    }
}

static void
transponder_set_reader (
    transponder _this)
{
    DDS_ReturnCode_t retcode;

    transponder_del_reader(_this);
    _this->reader = DDS_Subscriber_create_datareader (
                           _this->owner->subscriber,
                           _this->topic,
                           _this->rQos,
                           NULL,
                           DDS_ANY_STATUS );
    if (_this->reader == NULL) {
        ENTITY_ERROR(_this->config.partition_id,
              _this->config.transponder_id,
              "transponder_set_reader: "
              "create DataReader failed");
    } else {
        _this->condition = DDS_DataReader_get_statuscondition (
                               _this->reader);

        retcode = DDS_StatusCondition_set_enabled_statuses (
                     _this->condition,
                     DDS_DATA_AVAILABLE_STATUS);

        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.transponder_id,
                      "transponder_set_reader: "
                      "set_enabled_statuses failed",
                      retcode);
        } else {
            retcode = DDS_WaitSet_attach_condition (
                         _this->waitset,
                         _this->condition);

            if (retcode != DDS_RETCODE_OK) {
                DDS_ERROR(_this->config.partition_id,
                          _this->config.transponder_id,
                          "transponder_set_reader: "
                          "attach_condition failed",
                          retcode);
            }
        }
    }
    assert(_this->reader);
}

void *
transponderThread(
    void *arg)
{
    transponder _this = (transponder)arg;
    DDS_sequence_DDSTouchStone_latency_message dataList;
    DDS_SampleInfoSeq infoList;
    DDS_ConditionSeq *conditionList;
    DDS_ReturnCode_t retcode;
    DDS_Duration_t wait_timeout = DELAY;
    unsigned int i, length;

    DDSTouchStone_timestamp readTime, writeTime, sourceTime, arrivalTime;

    dataList._buffer = DDS_sequence_DDSTouchStone_latency_message_allocbuf(1);
    dataList._length = 0;
    dataList._maximum = 1;
    dataList._release = 1;

    infoList._buffer = DDS_SampleInfoSeq_allocbuf(1);
    infoList._length = 0;
    infoList._maximum = 1;
    infoList._release = 1;

    retcode = DDS_RETCODE_OK;

    transponder_set_writer(_this);
    transponder_set_reader(_this);

    conditionList = DDS_ConditionSeq__alloc();

    while (_this->active) {
        retcode = DDS_WaitSet_wait (_this->waitset,
                                   conditionList,
                                   &wait_timeout);
        length = conditionList->_length;
        for (i = 0; i < length; i++) {
            if (conditionList->_buffer[i] == _this->condition) {
                retcode = DDSTouchStone_latency_messageDataReader_take (
                             _this->reader,
                             &dataList,
                             &infoList,
                             1,
                             DDS_ANY_SAMPLE_STATE,
                             DDS_ANY_VIEW_STATE,
                             DDS_ANY_INSTANCE_STATE);

                readTime = get_timestamp();

                if (retcode == DDS_RETCODE_OK) {
                    for (i=0; i<dataList._length; i++) {
                        writeTime = dataList._buffer[i].write_timestamp;
                        sourceTime = get_timestamp_from_dds_time(
                                         &infoList._buffer[i].source_timestamp);
                        arrivalTime = get_timestamp_from_dds_time(
                                         &infoList._buffer[i].reception_timestamp);
                        dataList._buffer[i].send_latency = readTime - writeTime;
                        dataList._buffer[i].echo_timestamp = get_timestamp();
                        dataList._buffer[i].source_latency = sourceTime - writeTime;
                        dataList._buffer[i].arrival_latency = readTime - arrivalTime;
                        dataList._buffer[i].config_number = _this->config_number;
                        retcode = DDSTouchStone_latency_messageDataWriter_write (
                                     _this->writer,
                                     &dataList._buffer[i],
                                     DDS_HANDLE_NIL);
                        if (retcode != DDS_RETCODE_OK) {
                            DDS_ERROR(_this->config.partition_id,
                                      _this->config.transponder_id,
                                      "transponderThread: "
                                      "DDSTouchStone_latency_messageDataWriter_write",
                                      retcode);
                        }
                    }
                } else if (retcode != DDS_RETCODE_NO_DATA) {
                    DDS_ERROR(_this->config.partition_id,
                              _this->config.transponder_id,
                              "transceiverThread: "
                              "DDSTouchStone_latency_messageDataReader_take",
                              retcode);
                }
            }
        }
    }
    DDS_free(conditionList);

    transponder_del_reader(_this);
    transponder_del_writer(_this);

    return NULL;
}

static touchstone_os_result
transponder_start_thread (
    transponder _this)
{
    touchstone_os_threadAttr threadAttr;
    touchstone_os_result result;
    char name[32];

    sprintf(name,"transponder_%d",_this->config.transponder_id);

    if (_this->active) {
        _this->active = FALSE;
        result = touchstone_os_threadWaitExit(_this->threadId, NULL);
        if (result != touchstone_os_resultSuccess) {
            OS_ERROR(_this->config.partition_id,
                     _this->config.transponder_id,
                     "transponder_start_thread: "
                     "Thread wait exit failed",
                     result);
        }
    }

    _this->active = TRUE;

    result = threadAttrInit(&threadAttr,
                   _this->config.scheduling_class,
                   _this->config.thread_priority);

    if (result == touchstone_os_resultSuccess) {
        result = touchstone_os_threadCreate(
                           &_this->threadId,
                           name,
                           &threadAttr,
                           transponderThread,
                           _this);
        if (result != touchstone_os_resultSuccess) {
            OS_ERROR(_this->config.partition_id,
                     _this->config.transponder_id,
                     "transponder_start_thread: "
                     "touchstone_os_threadCreate failed",
                     result);
        }
    } else {
        OS_ERROR(_this->config.partition_id,
                 _this->config.transponder_id,
                 "transponder_start_thread: "
                 "threadAttrInit failed",
                 result);
    }
    return result;
}

static void
transponder_set_qos (
    transponder _this)
{
    DDS_sequence_DDSTouchStone_transponderQos xql;
    DDS_SampleInfoSeq infoList;
    DDS_ReturnCode_t retcode;
    DDS_TopicQos *tQos;
    DDSTouchStone_DataWriterQos *wQos;
    DDSTouchStone_DataReaderQos *rQos;
    DDSTouchStone_transponderQos xqm;
    touchstone_os_result result;
    int changed;

    infoList._release = TRUE;
    infoList._maximum = 1;
    infoList._length  = 0;
    infoList._buffer  = DDS_SampleInfoSeq_allocbuf(1);

    xql._release = TRUE;
    xql._maximum = 1;
    xql._length  = 0;
    xql._buffer  = DDS_sequence_DDSTouchStone_transponderQos_allocbuf(1);

    retcode = DDSTouchStone_transponderQosDataReader_take_w_condition (
                   xqr, &xql, &infoList, 1, _this->query);

    if ((retcode != DDS_RETCODE_OK) && (retcode != DDS_RETCODE_NO_DATA)) {
        DDS_ERROR(_this->config.partition_id,
                  _this->config.transponder_id,
                  "transponder_set_qos: "
                  "read transponder Qos policies failed",
                  retcode);
    }

    if (retcode == DDS_RETCODE_NO_DATA) {
        xqm.group_id = group_id;
        xqm.transponder_id = _this->config.transponder_id;
        xqm.partition_id   = _this->config.partition_id;
        set_touchstone_datareader_qos_default(&xqm.reader_qos);
        set_touchstone_datawriter_qos_default(&xqm.writer_qos);

        retcode = DDSTouchStone_transponderQosDataWriter_write (
                      xqw,
                      &xqm,
                      DDS_HANDLE_NIL);

        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.transponder_id,
                      "transponder_set_qos: "
                      "DDSTouchStone_transponderQosDataWriter_write",
                      retcode);
        }
    }

    tQos = DDS_TopicQos__alloc();
    retcode = DDS_Topic_get_qos(_this->topic, tQos);
    if (retcode != DDS_RETCODE_OK) {
        DDS_ERROR(_this->config.partition_id,
                  _this->config.transponder_id,
                  "transponder_set_qos: "
                  "DDS_Topic_get_qos failed",
                  retcode);
    }

    if (_this->wQos == NULL) {
        _this->wQos = DDS_DataWriterQos__alloc();
        retcode = DDS_Publisher_get_default_datawriter_qos(
                     _this->owner->publisher,
                     _this->wQos);
        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.transponder_id,
                      "transponder_set_qos: "
                      "get_default_datawriter_qos failed",
                      retcode);
        }
    }
    retcode = DDS_Publisher_copy_from_topic_qos(
                  _this->owner->publisher, _this->wQos, tQos);
    if (retcode != DDS_RETCODE_OK) {
        DDS_ERROR(_this->config.partition_id,
                  _this->config.transponder_id,
                  "transponder_set_qos: "
                  "DDS_Publisher_copy_from_topic_qos failed",
                  retcode);
    }
    if (_this->rQos == NULL) {
        _this->rQos = DDS_DataReaderQos__alloc();

        retcode = DDS_Subscriber_get_default_datareader_qos(
                     _this->owner->subscriber,
                     _this->rQos);
        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.transponder_id,
                      "transponder_set_qos: "
                      "get_default_datareader_qos failed",
                      retcode);
        }
    }
    retcode = DDS_Subscriber_copy_from_topic_qos(
                  _this->owner->subscriber, _this->rQos, tQos);
    if (retcode != DDS_RETCODE_OK) {
        DDS_ERROR(_this->config.partition_id,
                  _this->config.transponder_id,
                  "transponder_set_qos: "
                  "DDS_Publisher_copy_from_topic_qos failed",
                  retcode);
    }
    DDS_free(tQos);

    if (xql._length == 1) {
        wQos = &xql._buffer[0].writer_qos;
        rQos = &xql._buffer[0].reader_qos;
	changed = copy_touchstone_datawriter_qos(_this->wQos, wQos);
        changed = changed || copy_touchstone_datareader_qos(_this->rQos, rQos);
        if (changed) {
	    if (_this->active) {
                result = transponder_start_thread (_this);
                if (result != touchstone_os_resultSuccess) {
                    OS_ERROR(_this->config.partition_id,
                             _this->config.transponder_id,
                             "transponder_set_qos: "
                             "Thread creation failed",
                             result);
                }
            }
        } else {
            if (_this->writer) {
                retcode = DDS_DataWriter_set_qos(_this->writer,_this->wQos);
                if (retcode != DDS_RETCODE_OK) {
                    DDS_ERROR(_this->config.partition_id,
                              _this->config.transponder_id,
                              "transponder_set_qos: "
                              "DataWriter_set_qos failed",
                              retcode);
                }
            }
            if (_this->reader) {
                retcode = DDS_DataReader_set_qos(_this->reader,_this->rQos);
                if (retcode != DDS_RETCODE_OK) {
                    DDS_ERROR(_this->config.partition_id,
                              _this->config.transponder_id,
                              "transponder_set_qos: "
                              "DataReader_set_qos failed",
                              retcode);
                }
            }
        }
        _this->config_number++;
    } else if (xql._length > 1) {
        ENTITY_ERROR(_this->config.partition_id,
              _this->config.transponder_id,
              "transponder_set_qos: "
              "too many messages received");
    }
}

static void
transponder_del_topics (
    transponder _this)
{
    DDS_ReturnCode_t retcode;

    if (_this->topic != NULL) {
        retcode = DDS_DomainParticipant_delete_topic(
                      _this->owner->domain_participant,
                      _this->topic);
        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.transponder_id,
                      "transponder_del_topic: "
                      "delete Topic failed",
                      retcode);
        } else {
            _this->topic = NULL;
        }
    }
    if (_this->echo_topic != NULL) {
        retcode = DDS_DomainParticipant_delete_topic(
                      _this->owner->domain_participant,
                      _this->echo_topic);
        if (retcode != DDS_RETCODE_OK) {
            DDS_ERROR(_this->config.partition_id,
                      _this->config.transponder_id,
                      "transponder_del_topic: "
                      "delete Topic failed",
                      retcode);
        } else {
            _this->echo_topic = NULL;
        }
    }
}

static void
transponder_set_topics (
    transponder _this)
{
    DDS_TopicQos *tQos;
    DDS_ReturnCode_t retcode;
    char id[32];

    tQos = DDS_TopicQos__alloc();

    retcode = DDS_DomainParticipant_get_default_topic_qos(
                  _this->owner->domain_participant,
                  tQos);

    if (retcode == DDS_RETCODE_OK) {
        transponder_del_topics(_this);
        set_topic_qos(tQos, _this->config.topic_kind);
        set_topic_name(id,"LatencyTopic",
                       _this->config.topic_id,
                       _this->config.topic_kind);
        _this->topic =
            DDS_DomainParticipant_create_topic (
                      _this->owner->domain_participant,
                      id,
                      "DDSTouchStone::latency_message",
                      tQos,
                      NULL,
                      DDS_ANY_STATUS);
        set_topic_name(id,"LatencyEchoTopic",
                       _this->config.topic_id,
                       _this->config.topic_kind);
        _this->echo_topic =
            DDS_DomainParticipant_create_topic (
                      _this->owner->domain_participant,
                      id,
                      "DDSTouchStone::latency_message",
                      tQos,
                      NULL,
                      DDS_ANY_STATUS);

        DDS_free(tQos);

        if (_this->topic == DDS_HANDLE_NIL) {
            ENTITY_ERROR(_this->config.partition_id,
                  _this->config.transponder_id,
                  "transponder_set_topic: "
                  "create_topic failed");
        }
    } else {
        DDS_ERROR(_this->config.partition_id,
                  _this->config.transponder_id,
                  "transponder_set_topic: "
                  "get_default_topic_qos failed",
                  retcode);
    }
}

static void
dispose_transponderDef (
    DDSTouchStone_transponderDef *msg)
{
    participant p;
    transponder _this;
    transponder *ref;
    touchstone_os_result result;

    p = lookup_participant(msg->partition_id);
    if (p != NULL) {
        _this = lookup_transponder(p,msg->transponder_id);
        if (_this != NULL) {
            _this->active = FALSE;
            result = touchstone_os_threadWaitExit(_this->threadId, NULL);
            if (result != touchstone_os_resultSuccess) {
                OS_ERROR(msg->partition_id,
                         msg->transponder_id,
                         "dispose_transponderDef: "
                         "touchstone_os_threadWaitExit failed",
                         result);
            }
            DDS_DataReader_delete_readcondition(xqr, _this->query);
            DDS_free(_this->wQos);
            DDS_free(_this->rQos);
            transponder_del_topics(_this);
            ref = &p->transponders;
            while ((*ref != NULL) && (*ref != _this)) {
                ref = &(*ref)->next;
            }
            assert(*ref == _this);
            *ref = _this->next;
            _this->next = NULL;
            free(_this);
        }
    }
}

static void
process_transponderDef (
    DDSTouchStone_transponderDef *msg)
{
    participant p;
    transponder _this;
    DDS_StringSeq TransponderId;
    char id[32];
    touchstone_os_result result;

    p = lookup_participant(msg->partition_id);
    if (p == NULL) {
        p = create_participant(msg->partition_id);
        _this = NULL;
    } else {
        _this = lookup_transponder(p,msg->transponder_id);
    }
    if (p != NULL) {
        if (_this == NULL) {
            activate_transponder_report();
            sprintf(id,"%d",msg->transponder_id);

            TransponderId._maximum   = 1;
            TransponderId._length    = 1;
            TransponderId._buffer    = DDS_StringSeq_allocbuf(1);
            TransponderId._buffer[0] = DDS_string_dup(id);

            _this = (transponder)malloc(sizeof(*_this));

            _this->config = *msg;
            _this->config_number = 0;
            _this->topic = NULL;
            _this->echo_topic = NULL;
            _this->wQos = NULL;
            _this->rQos = NULL;
            _this->writer = NULL;
            _this->reader = NULL;

            memset(&(_this->threadId), 0, sizeof(_this->threadId));

            _this->active = FALSE;
            _this->owner = p;
            _this->waitset = DDS_WaitSet__alloc ();
            _this->query  = DDS_DataReader_create_querycondition(
                               xqr,
                               DDS_ANY_SAMPLE_STATE,
                               DDS_ANY_VIEW_STATE,
                               DDS_ANY_INSTANCE_STATE,
                               "transponder_id = %0",
                               &TransponderId);

            transponder_set_topics(_this);
            transponder_set_qos(_this);


            result = transponder_start_thread(_this);

            if (result == touchstone_os_resultSuccess) {
                _this->next = p->transponders;
                p->transponders = _this;
            } else {
                _this->active = FALSE;
            }
        } else {
            if ((_this->config.scheduling_class != msg->scheduling_class) ||
                (_this->config.thread_priority != msg->thread_priority) ||
                (_this->config.topic_kind      != msg->topic_kind)||
                (_this->config.topic_id        != msg->topic_id)) {

                _this->active = FALSE;

                result = touchstone_os_threadWaitExit(_this->threadId, NULL);
                if (result != touchstone_os_resultSuccess) {
                    OS_ERROR(msg->partition_id,
                             msg->transponder_id,
                             "process_transponderDef: "
                             "touchstone_os_threadWaitExit failed",
                             result);
                }
                if ((_this->config.topic_kind != msg->topic_kind) ||
                    (_this->config.topic_id   != msg->topic_id)) {
                    _this->config = *msg;
                    transponder_set_topics(_this);
                    transponder_set_qos(_this);
                }

                _this->config = *msg;
                _this->config_number++;

                result = transponder_start_thread(_this);
                if (result != touchstone_os_resultSuccess) {
                    p->transponders = _this->next;
                    _this->next = NULL;
                    _this->active = FALSE;
                }
            } else {
                _this->config = *msg;
                _this->config_number++;
            }
        }
    }
}

static void
process_transponderQos (
    DDSTouchStone_transponderQos *msg)
{
    participant p;
    transponder t;

    p = lookup_participant(msg->partition_id);
    if (p != NULL) {
        t = lookup_transponder(p,msg->transponder_id);
        if (t != NULL) {
            transponder_set_qos(t);
        }
    }
}

static DDS_boolean
process_command (
    DDS_StatusCondition condition)
{
    DDS_sequence_DDSTouchStone_transmitterDef tdl = {0,0,DDS_OBJECT_NIL,FALSE};
    DDS_sequence_DDSTouchStone_transmitterQos tql = {0,0,DDS_OBJECT_NIL,FALSE};
    DDS_sequence_DDSTouchStone_receiverDef    rdl = {0,0,DDS_OBJECT_NIL,FALSE};
    DDS_sequence_DDSTouchStone_receiverQos    rql = {0,0,DDS_OBJECT_NIL,FALSE};
    DDS_sequence_DDSTouchStone_transceiverDef sdl = {0,0,DDS_OBJECT_NIL,FALSE};
    DDS_sequence_DDSTouchStone_transceiverQos sql = {0,0,DDS_OBJECT_NIL,FALSE};
    DDS_sequence_DDSTouchStone_transponderDef xdl = {0,0,DDS_OBJECT_NIL,FALSE};
    DDS_sequence_DDSTouchStone_transponderQos xql = {0,0,DDS_OBJECT_NIL,FALSE};
    DDS_SampleInfoSeq infoList = { 0, 0, NULL, FALSE };
    DDS_ReturnCode_t  retcode;
    int length, i;

    if (condition == tdrc) {
        retcode = DDSTouchStone_transmitterDefDataReader_take(
                     tdr, &tdl, &infoList, 1,
                     DDS_ANY_SAMPLE_STATE,
                     DDS_ANY_VIEW_STATE,
                     DDS_ANY_INSTANCE_STATE);
        if (retcode == DDS_RETCODE_OK) {
            length = tdl._length;
            if (length != 0) {
                for (i=0; i<length; i++) {
                    if (infoList._buffer[i].instance_state ==
                               DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE) {
                        dispose_transmitterDef(&tdl._buffer[i]);
                    } else {
                        if (infoList._buffer[i].valid_data) {
                            process_transmitterDef(&tdl._buffer[i]);
                        } else {
                            dispose_transmitterDef(&tdl._buffer[i]);
                        }
                    }
                }
                retcode = DDSTouchStone_transmitterDefDataReader_return_loan (
                             tdr, &tdl, &infoList);
            }
        }
    } else if (condition == tqrc) {
        retcode = DDSTouchStone_transmitterQosDataReader_read_w_condition (
                     tqr, &tql, &infoList, 1, tqrc);
        if (retcode == DDS_RETCODE_OK) {
            length = tql._length;
            if (length != 0) {
                for (i=0; i<length; i++) {
                    process_transmitterQos(&tql._buffer[i]);
                }
                retcode = DDSTouchStone_transmitterQosDataReader_return_loan (
                             tqr, &tql, &infoList);
            }
        }
    } else if (condition == rdrc) {
        retcode = DDSTouchStone_receiverDefDataReader_take(
                     rdr, &rdl, &infoList, 1,
                     DDS_ANY_SAMPLE_STATE,
                     DDS_ANY_VIEW_STATE,
                     DDS_ANY_INSTANCE_STATE);
        if (retcode == DDS_RETCODE_OK) {
            length = rdl._length;
            if (length != 0) {
                for (i=0; i<length; i++) {
                    if (infoList._buffer[i].instance_state ==
                               DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE) {
                        dispose_receiverDef(&rdl._buffer[i]);
                    } else {
                        if (infoList._buffer[i].valid_data) {
                            process_receiverDef(&rdl._buffer[i]);
                        } else {
                            dispose_receiverDef(&rdl._buffer[i]);
                        }
                    }
                }
                retcode = DDSTouchStone_receiverDefDataReader_return_loan (
                             rdr, &rdl, &infoList);
            }
        }
    } else if (condition == rqrc) {
        retcode = DDSTouchStone_receiverQosDataReader_read_w_condition (
                     rqr, &rql, &infoList, 1, rqrc);
        if (retcode == DDS_RETCODE_OK) {
            length = rql._length;
            if (length != 0) {
                for (i=0; i<length; i++) {
                    process_receiverQos(&rql._buffer[i]);
                }
                retcode = DDSTouchStone_receiverQosDataReader_return_loan (
                             rqr, &rql, &infoList);
            }
        }
    } else if (condition == sdrc) {
        retcode = DDSTouchStone_transceiverDefDataReader_take(
                     sdr, &sdl, &infoList, 1,
                     DDS_ANY_SAMPLE_STATE,
                     DDS_ANY_VIEW_STATE,
                     DDS_ANY_INSTANCE_STATE);
        if (retcode == DDS_RETCODE_OK) {
            length = sdl._length;
            if (length != 0) {
                for (i=0; i<length; i++) {
                    if (infoList._buffer[i].instance_state ==
                               DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE) {
                        dispose_transceiverDef(&sdl._buffer[i]);
                    } else {
                        if (infoList._buffer[i].valid_data) {
                            process_transceiverDef(&sdl._buffer[i]);
                        } else {
                            dispose_transceiverDef(&sdl._buffer[i]);
                        }
                    }
                }
                retcode = DDSTouchStone_transceiverDefDataReader_return_loan (
                             sdr, &sdl, &infoList);
            }
        }
    } else if (condition == sqrc) {
        retcode = DDSTouchStone_transceiverQosDataReader_read_w_condition (
                     sqr, &sql, &infoList, 1, sqrc);
        if (retcode == DDS_RETCODE_OK) {
            length = sql._length;
            if (length != 0) {
                for (i=0; i<length; i++) {
                    process_transceiverQos(&sql._buffer[i]);
                }
                retcode = DDSTouchStone_transceiverQosDataReader_return_loan (
                             sqr, &sql, &infoList);
            }
        }
    } else if (condition == xdrc) {
        retcode = DDSTouchStone_transponderDefDataReader_take(
                     xdr, &xdl, &infoList, 1,
                     DDS_ANY_SAMPLE_STATE,
                     DDS_ANY_VIEW_STATE,
                     DDS_ANY_INSTANCE_STATE);
        if (retcode == DDS_RETCODE_OK) {
            length = xdl._length;
            if (length != 0) {
                for (i=0; i<length; i++) {
                    if (infoList._buffer[i].instance_state ==
                               DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE) {
                        dispose_transponderDef(&xdl._buffer[i]);
                    } else {
                        if (infoList._buffer[i].valid_data) {
                            process_transponderDef(&xdl._buffer[i]);
                        } else {
                            dispose_transponderDef(&xdl._buffer[i]);
                        }
                    }
                }
                retcode = DDSTouchStone_transponderDefDataReader_return_loan (
                             xdr, &xdl, &infoList);
            }
        }
    } else if (condition == xqrc) {
        retcode = DDSTouchStone_transponderQosDataReader_read_w_condition (
                     xqr, &xql, &infoList, 1, xqrc);
        if (retcode == DDS_RETCODE_OK) {
            length = xql._length;
            if (length != 0) {
                for (i=0; i<length; i++) {
                    process_transponderQos(&xql._buffer[i]);
                }
                retcode = DDSTouchStone_transponderQosDataReader_return_loan (
                             xqr, &xql, &infoList);
            }
        }
    } else {
        ENTITY_ERROR(0,0,"Waitset Returned an unknown condition");
    }
    return TRUE;
}

