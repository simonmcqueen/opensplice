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


/* Interface */
#include "commandHelpers.h"
/* Implementation */
#include <string.h> /* for strcmp */

#define COMMANDHELPERS_ERROR(message) \
        printf("Recorder line %d: %s\n", \
               __LINE__, message)


static const char *
topicKindEnumToStr(
    DDSTouchStone_TopicKind topicKind)
{
    static const char *topicKindStrings[] = {
        "BEST_EFFORT", "RELIABLE", "TRANSIENT", "PERSISTENT"
    };

    return topicKindStrings[topicKind];
}

static DDSTouchStone_TopicKind
topicKindStrToEnum(
    const char *str)
{
    DDSTouchStone_TopicKind currentKind;
    DDSTouchStone_TopicKind result = DDSTouchStone_BEST_EFFORT;
    int found = 0;

    for (currentKind = result; (currentKind <= DDSTouchStone_PERSISTENT) && !found; currentKind++) {
        found = (strcmp(topicKindEnumToStr(currentKind), str) == 0);
        if (found) {
            result = currentKind;
        }
    }
    if (!found) {
        COMMANDHELPERS_ERROR("Error in converting TopicKind from string to enum");
    }

    return result;
}

static const char *
schedulingClassEnumToStr(
    DDSTouchStone_SchedulingClass schedulingClass)
{
    static const char *schedulingClassStrings[] = {
       "TIMESHARING", "REALTIME" 
    };

    return schedulingClassStrings[schedulingClass];
}

static DDSTouchStone_SchedulingClass
schedulingClassStrToEnum(
    const char *str)
{
    DDSTouchStone_SchedulingClass currentKind;
    DDSTouchStone_SchedulingClass result = DDSTouchStone_TIMESHARING;
    int found = 0;

    for (currentKind = result; (currentKind <= DDSTouchStone_REALTIME) && !found; currentKind++) {
        found = (strcmp(schedulingClassEnumToStr(currentKind), str) == 0);
        if (found) {
            result = currentKind;
        }
    }
    if (!found) {
        COMMANDHELPERS_ERROR("Error in converting SchedulingClass from string to enum");
    }

    return result;
}


static void
DDSTouchStone_DataWriterQosPrint(
    char *buffer,
    unsigned int bufferSize,
    DDSTouchStone_DataWriterQos *dwq)
{
    int size;

#if _MSC_VER
#define snprintf _snprintf
#endif

    size = snprintf(buffer, bufferSize,
        "latency_budget= { sec=%d nanosec=%d } "
        "transport_priority=%d",
        dwq->latency_budget.duration.sec, dwq->latency_budget.duration.nanosec,
        dwq->transport_priority.value);
    if ((size == bufferSize) && (buffer[bufferSize-1] != '\0')) {
        COMMANDHELPERS_ERROR("bufferSize too small to fit command");
    }
}


static void
DDSTouchStone_DataReaderQosPrint(
    char *buffer,
    unsigned int bufferSize,
    DDSTouchStone_DataReaderQos *drq)
{
    int size;

    size = snprintf(buffer, bufferSize,
        "latency_budget= { sec=%d nanosec=%d } "
        "history_depth=%d",
        drq->latency_budget.duration.sec, drq->latency_budget.duration.nanosec,
        drq->history.depth);
    if ((size == bufferSize) && (buffer[bufferSize-1] != '\0')) {
        COMMANDHELPERS_ERROR("bufferSize too small to fit command");
    }
}


#define MAX_QOS_SIZE  (200)
#define MAX_ENUM_SIZE (50)

void
DDSTouchStone_transmitterDefPrint(
    char *buffer, 
    unsigned int bufferSize,
    DDSTouchStone_transmitterDef *td)
{
    int size;

    size = snprintf(buffer, bufferSize,
        "group_id=%u "
        "transmitter_id=%u "
        "partition_id=%u "
        "topic_id=%u "
        "topic_kind=%s "
        "scheduling_class=%s "
        "thread_priority=%d "
        "message_size=%u "
        "messages_per_burst=%u "
        "burst_period=%u",
        td->group_id, td->transmitter_id, td->partition_id, td->topic_id,
        topicKindEnumToStr(td->topic_kind), schedulingClassEnumToStr(td->scheduling_class),
        td->thread_priority, td->message_size, td->messages_per_burst, td->burst_period);
    if ((size == bufferSize) && (buffer[bufferSize-1] != '\0')) {
        COMMANDHELPERS_ERROR("bufferSize too small to fit command");
    }
}

void
DDSTouchStone_transmitterDefScan(
    FILE *stream,
    DDSTouchStone_transmitterDef *td,
    int *success)
{
    static char topicKindString[MAX_ENUM_SIZE];
    static char schedulingClassString[MAX_ENUM_SIZE];
    int result;

    result = fscanf(stream,
        "group_id=%u "
        "transmitter_id=%u "
        "partition_id=%u "
        "topic_id=%u "
        "topic_kind=%s "
        "scheduling_class=%s "
        "thread_priority=%d "
        "message_size=%u "
        "messages_per_burst=%u "
        "burst_period=%u",
        &(td->group_id), &(td->transmitter_id), &(td->partition_id), &(td->topic_id),
        topicKindString, schedulingClassString,
        &(td->thread_priority), &(td->message_size), &(td->messages_per_burst), &(td->burst_period));
    *success = (result == 10);
    if (!*success) {
        COMMANDHELPERS_ERROR("error in scanning transmitterDef");
    }
    td->topic_kind = topicKindStrToEnum(topicKindString);
    td->scheduling_class = schedulingClassStrToEnum(schedulingClassString);
}

void
DDSTouchStone_transmitterQosPrint(
    char *buffer, 
    unsigned int bufferSize,
    DDSTouchStone_transmitterQos *tq)
{
    int size;
    static char qosBuffer[MAX_QOS_SIZE];

    DDSTouchStone_DataWriterQosPrint(qosBuffer, MAX_QOS_SIZE, &tq->qos);
    size = snprintf(buffer, bufferSize,
        "group_id=%u "
        "transmitter_id=%u "
        "partition_id=%u "
        "qos= { %s }",
        tq->group_id, tq->transmitter_id, tq->partition_id, qosBuffer);
    if ((size == bufferSize) && (buffer[bufferSize-1] != '\0')) {
        COMMANDHELPERS_ERROR("bufferSize too small to fit command");
    }
}

void
DDSTouchStone_transmitterQosScan(
    FILE *stream,
    DDSTouchStone_transmitterQos *tq,
    int *success)
{
    int result;

    result = fscanf(stream,
        "group_id=%u "
        "transmitter_id=%u "
        "partition_id=%u "
        "qos= { "
            "latency_budget= { sec=%d nanosec=%d } "
            "transport_priority=%d"
        " }",
        &(tq->group_id), &(tq->transmitter_id), &(tq->partition_id),
        &(tq->qos.latency_budget.duration.sec), &(tq->qos.latency_budget.duration.nanosec),
        &(tq->qos.transport_priority.value));
    *success = (result == 6);
    if (!*success) {
        COMMANDHELPERS_ERROR("error in scanning transmitterQos");
    }
}

void
DDSTouchStone_receiverDefPrint(
    char *buffer, 
    unsigned int bufferSize,
    DDSTouchStone_receiverDef *rd)
{
    int size;

    size = snprintf(buffer, bufferSize,
        "group_id=%u "
        "receiver_id=%u "
        "partition_id=%u "
        "topic_id=%u "
        "topic_kind=%s "
        "scheduling_class=%s "
        "thread_priority=%d "
        "report_period=%u "
        "polling_period=%u",
        rd->group_id, rd->receiver_id, rd->partition_id, rd->topic_id,
        topicKindEnumToStr(rd->topic_kind), schedulingClassEnumToStr(rd->scheduling_class),
        rd->thread_priority, rd->report_period, rd->polling_period);
    if ((size == bufferSize) && (buffer[bufferSize-1] != '\0')) {
        COMMANDHELPERS_ERROR("bufferSize too small to fit command");
    }
}

void
DDSTouchStone_receiverDefScan(
    FILE *stream,
    DDSTouchStone_receiverDef *rd,
    int *success)
{
    static char topicKindString[MAX_ENUM_SIZE];
    static char schedulingClassString[MAX_ENUM_SIZE];
    int result;

    result = fscanf(stream,
        "group_id=%u "
        "receiver_id=%u "
        "partition_id=%u "
        "topic_id=%u "
        "topic_kind=%s "
        "scheduling_class=%s "
        "thread_priority=%d "
        "report_period=%u "
        "polling_period=%u",
        &(rd->group_id), &(rd->receiver_id), &(rd->partition_id), &(rd->topic_id),
        topicKindString, schedulingClassString,
        &(rd->thread_priority), &(rd->report_period), &(rd->polling_period));
    *success = (result == 9);
    if (!*success) {
        COMMANDHELPERS_ERROR("error in scanning receiverDef");
    }
    rd->topic_kind = topicKindStrToEnum(topicKindString);
    rd->scheduling_class = schedulingClassStrToEnum(schedulingClassString);
}

void
DDSTouchStone_receiverQosPrint(
    char *buffer, 
    unsigned int bufferSize,
    DDSTouchStone_receiverQos *rq)
{
    int size;
    static char qosBuffer[MAX_QOS_SIZE];

    DDSTouchStone_DataReaderQosPrint(qosBuffer, MAX_QOS_SIZE, &rq->qos);
    size = snprintf(buffer, bufferSize,
        "group_id=%u "
        "receiver_id=%u "
        "partition_id=%u "
        "qos= { %s }",
        rq->group_id, rq->receiver_id, rq->partition_id, qosBuffer);
    if ((size == bufferSize) && (buffer[bufferSize-1] != '\0')) {
        COMMANDHELPERS_ERROR("bufferSize too small to fit command");
    }
}

void
DDSTouchStone_receiverQosScan(
    FILE *stream,
    DDSTouchStone_receiverQos *rq,
    int *success)
{
    int result;

    result = fscanf(stream,
        "group_id=%u "
        "receiver_id=%u "
        "partition_id=%u "
        "qos= { "
            "latency_budget= { sec=%d nanosec=%d } "
            "history_depth=%d"
        " }",
        &(rq->group_id), &(rq->receiver_id), &(rq->partition_id),
        &(rq->qos.latency_budget.duration.sec), &(rq->qos.latency_budget.duration.nanosec),
        &(rq->qos.history.depth));
    *success = (result == 6);
    if (!*success) {
        COMMANDHELPERS_ERROR("error in scanning receiverQos");
    }
}

void
DDSTouchStone_transceiverDefPrint(
    char *buffer, 
    unsigned int bufferSize,
    DDSTouchStone_transceiverDef *sd)
{
    int size;

    size = snprintf(buffer, bufferSize,
        "group_id=%u "
        "transceiver_id=%u "
        "partition_id=%u "
        "topic_id=%u "
        "topic_kind=%s "
        "scheduling_class=%s "
        "thread_priority=%d "
        "message_size=%u "
        "write_period=%u "
        "report_period=%u",
        sd->group_id, sd->transceiver_id, sd->partition_id, sd->topic_id,
        topicKindEnumToStr(sd->topic_kind), schedulingClassEnumToStr(sd->scheduling_class),
        sd->thread_priority, sd->message_size, sd->write_period, sd->report_period);
    if ((size == bufferSize) && (buffer[bufferSize-1] != '\0')) {
        COMMANDHELPERS_ERROR("bufferSize too small to fit command");
    }
}

void
DDSTouchStone_transceiverDefScan(
    FILE *stream,
    DDSTouchStone_transceiverDef *sd,
    int *success)
{
    static char topicKindString[MAX_ENUM_SIZE];
    static char schedulingClassString[MAX_ENUM_SIZE];
    int result;

    result = fscanf(stream,
        "group_id=%u "
        "transceiver_id=%u "
        "partition_id=%u "
        "topic_id=%u "
        "topic_kind=%s "
        "scheduling_class=%s "
        "thread_priority=%d "
        "message_size=%u "
        "write_period=%u "
        "report_period=%u",
        &(sd->group_id), &(sd->transceiver_id), &(sd->partition_id), &(sd->topic_id),
        topicKindString, schedulingClassString,
        &(sd->thread_priority), &(sd->message_size), &(sd->write_period), &(sd->report_period));
    *success = (result == 10);
    if (!*success) {
        COMMANDHELPERS_ERROR("error in scanning transceiverDef");
    }
    sd->topic_kind = topicKindStrToEnum(topicKindString);
    sd->scheduling_class = schedulingClassStrToEnum(schedulingClassString);
}

void
DDSTouchStone_transceiverQosPrint(
    char *buffer, 
    unsigned int bufferSize,
    DDSTouchStone_transceiverQos *sq)
{
    int size;
    static char wqosBuffer[MAX_QOS_SIZE];
    static char rqosBuffer[MAX_QOS_SIZE];

    DDSTouchStone_DataWriterQosPrint(wqosBuffer, MAX_QOS_SIZE, &sq->writer_qos);
    DDSTouchStone_DataReaderQosPrint(rqosBuffer, MAX_QOS_SIZE, &sq->reader_qos);
    size = snprintf(buffer, bufferSize,
        "group_id=%u "
        "transceiver_id=%u "
        "partition_id=%u "
        "writer_qos={ %s } "
        "reader_qos={ %s }",
        sq->group_id, sq->transceiver_id, sq->partition_id, wqosBuffer, rqosBuffer);

    if ((size == bufferSize) && (buffer[bufferSize-1] != '\0')) {
        COMMANDHELPERS_ERROR("bufferSize too small to fit command");
    }
}

void
DDSTouchStone_transceiverQosScan(
    FILE *stream,
    DDSTouchStone_transceiverQos *sq,
    int *success)
{
    int result;

    result = fscanf(stream,
        "group_id=%u "
        "transceiver_id=%u "
        "partition_id=%u "
        "writer_qos= { "
            "latency_budget= { sec=%d nanosec=%d } "
            "transport_priority=%d"
        " } "
        "reader_qos= { "
            "latency_budget= { sec=%d nanosec=%d } "
            "history_depth=%d"
        " }",
        &(sq->group_id), &(sq->transceiver_id), &(sq->partition_id),
        &(sq->writer_qos.latency_budget.duration.sec), &(sq->writer_qos.latency_budget.duration.nanosec),
        &(sq->writer_qos.transport_priority.value),
        &(sq->reader_qos.latency_budget.duration.sec), &(sq->reader_qos.latency_budget.duration.nanosec),
        &(sq->reader_qos.history.depth));
    *success = (result == 9);
    if (!*success) {
        COMMANDHELPERS_ERROR("error in scanning transceiverQos");
    }
}

void
DDSTouchStone_transponderDefPrint(
    char *buffer, 
    unsigned int bufferSize,
    DDSTouchStone_transponderDef *xd)
{
    int size;

    size = snprintf(buffer, bufferSize,
        "group_id=%u "
        "transponder_id=%u "
        "partition_id=%u "
        "topic_id=%u "
        "topic_kind=%s "
        "scheduling_class=%s "
        "thread_priority=%d",
        xd->group_id, xd->transponder_id, xd->partition_id, xd->topic_id,
        topicKindEnumToStr(xd->topic_kind), schedulingClassEnumToStr(xd->scheduling_class),
        xd->thread_priority);
    if ((size == bufferSize) && (buffer[bufferSize-1] != '\0')) {
        COMMANDHELPERS_ERROR("bufferSize too small to fit command");
    }
}

void
DDSTouchStone_transponderDefScan(
    FILE *stream,
    DDSTouchStone_transponderDef *xd,
    int *success)
{
    static char topicKindString[MAX_ENUM_SIZE];
    static char schedulingClassString[MAX_ENUM_SIZE];
    int result;

    result = fscanf(stream,
        "group_id=%u "
        "transponder_id=%u "
        "partition_id=%u "
        "topic_id=%u "
        "topic_kind=%s "
        "scheduling_class=%s "
        "thread_priority=%d",
        &(xd->group_id), &(xd->transponder_id), &(xd->partition_id), &(xd->topic_id),
        topicKindString, schedulingClassString,
        &(xd->thread_priority));
    *success = (result == 7);
    if (!*success) {
        COMMANDHELPERS_ERROR("error in scanning transponderDef");
    }
    xd->topic_kind = topicKindStrToEnum(topicKindString);
    xd->scheduling_class = schedulingClassStrToEnum(schedulingClassString);
}

void
DDSTouchStone_transponderQosPrint(
    char *buffer, 
    unsigned int bufferSize,
    DDSTouchStone_transponderQos *xq)
{
    int size;
    static char wqosBuffer[MAX_QOS_SIZE];
    static char rqosBuffer[MAX_QOS_SIZE];

    DDSTouchStone_DataWriterQosPrint(wqosBuffer, MAX_QOS_SIZE, &xq->writer_qos);
    DDSTouchStone_DataReaderQosPrint(rqosBuffer, MAX_QOS_SIZE, &xq->reader_qos);
    size = snprintf(buffer, bufferSize,
        "group_id=%u "
        "transponder_id=%u "
        "partition_id=%u "
        "writer_qos= { %s } "
        "reader_qos= { %s }",
        xq->group_id, xq->transponder_id, xq->partition_id, wqosBuffer, rqosBuffer);
    if ((size == bufferSize) && (buffer[bufferSize-1] != '\0')) {
        COMMANDHELPERS_ERROR("bufferSize too small to fit command");
    }
}

void
DDSTouchStone_transponderQosScan(
    FILE *stream,
    DDSTouchStone_transponderQos *xq,
    int *success)
{
    int result;

    result = fscanf(stream,
        "group_id=%u "
        "transponder_id=%u "
        "partition_id=%u "
        "writer_qos= { "
            "latency_budget= { sec=%d nanosec=%d } "
            "transport_priority=%d"
        " } "
        "reader_qos= { "
            "latency_budget= { sec=%d nanosec=%d } "
            "history_depth=%d"
        " }",
        &(xq->group_id), &(xq->transponder_id), &(xq->partition_id),
        &(xq->writer_qos.latency_budget.duration.sec), &(xq->writer_qos.latency_budget.duration.nanosec),
        &(xq->writer_qos.transport_priority.value),
        &(xq->reader_qos.latency_budget.duration.sec), &(xq->reader_qos.latency_budget.duration.nanosec),
        &(xq->reader_qos.history.depth));
    *success = (result == 9);
    if (!*success) {
        COMMANDHELPERS_ERROR("error in scanning transponderQos");
    }
}

#undef MAX_QOS_SIZE
