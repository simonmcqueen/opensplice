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

#include "commandHelpers.h"

#define RECORDER_ERROR(message) \
        printf("Recorder line %d: %s\n", \
               __LINE__, message)

#define RECORDER_REPORT(message) \
        if (verbose_mode) { \
            printf("Recorder message: " message "\n"); \
        }

#define RECORDER_REPORT_1(message, a1) \
        if (verbose_mode) { \
            printf("Recorder message: " message "\n", a1); \
        }

static char *DDSTouchStoneCommandPartition = "DDSTouchStoneCommands";
/* static char *DDSTouchStoneReportPartition =  "DDSTouchStoneReports"; */ /* Not used */

static DDS_DomainId_t               RecorderDomain      = DDS_DOMAIN_ID_DEFAULT;
static DDS_DomainParticipantFactory RecorderFactory     = DDS_OBJECT_NIL;
static DDS_DomainParticipant        RecorderParticipant = DDS_OBJECT_NIL;
static DDS_Subscriber               RecorderSubscriber  = DDS_OBJECT_NIL;
static DDS_Publisher                RecorderPublisher   = DDS_OBJECT_NIL;

/* Command TypeSupport */
static DDSTouchStone_transmitterDefTypeSupport  tds = DDS_OBJECT_NIL;
static DDSTouchStone_transmitterQosTypeSupport  tqs = DDS_OBJECT_NIL;
static DDSTouchStone_receiverDefTypeSupport     rds = DDS_OBJECT_NIL;
static DDSTouchStone_receiverQosTypeSupport     rqs = DDS_OBJECT_NIL;
static DDSTouchStone_transceiverDefTypeSupport  sds = DDS_OBJECT_NIL;
static DDSTouchStone_transceiverQosTypeSupport  sqs = DDS_OBJECT_NIL;
static DDSTouchStone_transponderDefTypeSupport  xds = DDS_OBJECT_NIL;
static DDSTouchStone_transponderQosTypeSupport  xqs = DDS_OBJECT_NIL;
static DDSTouchStone_recorderCommandTypeSupport rcs = DDS_OBJECT_NIL;
static DDSTouchStone_recorderStateTypeSupport   rss = DDS_OBJECT_NIL;

/* Command Topics */
static DDS_Topic tdt = DDS_OBJECT_NIL;
static DDS_Topic tqt = DDS_OBJECT_NIL;
static DDS_Topic rdt = DDS_OBJECT_NIL;
static DDS_Topic rqt = DDS_OBJECT_NIL;
static DDS_Topic sdt = DDS_OBJECT_NIL;
static DDS_Topic sqt = DDS_OBJECT_NIL;
static DDS_Topic xdt = DDS_OBJECT_NIL;
static DDS_Topic xqt = DDS_OBJECT_NIL;
static DDS_Topic rct = DDS_OBJECT_NIL;
static DDS_Topic rst = DDS_OBJECT_NIL;

/* Content Filtered Command Topic */
static DDS_ContentFilteredTopic rcf = DDS_OBJECT_NIL;

/* Command Writers */
static DDSTouchStone_transmitterDefDataWriter tdw = DDS_OBJECT_NIL;
static DDSTouchStone_transmitterQosDataWriter tqw = DDS_OBJECT_NIL;
static DDSTouchStone_receiverDefDataWriter    rdw = DDS_OBJECT_NIL;
static DDSTouchStone_receiverQosDataWriter    rqw = DDS_OBJECT_NIL;
static DDSTouchStone_transceiverDefDataWriter sdw = DDS_OBJECT_NIL;
static DDSTouchStone_transceiverQosDataWriter sqw = DDS_OBJECT_NIL;
static DDSTouchStone_transponderDefDataWriter xdw = DDS_OBJECT_NIL;
static DDSTouchStone_transponderQosDataWriter xqw = DDS_OBJECT_NIL;
static DDSTouchStone_recorderStateDataWriter  rsw = DDS_OBJECT_NIL;

/* Command Readers */
static DDSTouchStone_transmitterDefDataReader  tdr = DDS_OBJECT_NIL;
static DDSTouchStone_transmitterQosDataReader  tqr = DDS_OBJECT_NIL;
static DDSTouchStone_receiverDefDataReader     rdr = DDS_OBJECT_NIL;
static DDSTouchStone_receiverQosDataReader     rqr = DDS_OBJECT_NIL;
static DDSTouchStone_transceiverDefDataReader  sdr = DDS_OBJECT_NIL;
static DDSTouchStone_transceiverQosDataReader  sqr = DDS_OBJECT_NIL;
static DDSTouchStone_transponderDefDataReader  xdr = DDS_OBJECT_NIL;
static DDSTouchStone_transponderQosDataReader  xqr = DDS_OBJECT_NIL;
static DDSTouchStone_recorderCommandDataReader rcr = DDS_OBJECT_NIL;

/* Command Reader Status */
static DDS_StatusCondition tdrc = DDS_OBJECT_NIL;
static DDS_StatusCondition tqrc = DDS_OBJECT_NIL;
static DDS_StatusCondition rdrc = DDS_OBJECT_NIL;
static DDS_StatusCondition rqrc = DDS_OBJECT_NIL;
static DDS_StatusCondition sdrc = DDS_OBJECT_NIL;
static DDS_StatusCondition sqrc = DDS_OBJECT_NIL;
static DDS_StatusCondition xdrc = DDS_OBJECT_NIL;
static DDS_StatusCondition xqrc = DDS_OBJECT_NIL;
static DDS_StatusCondition rcrc = DDS_OBJECT_NIL;

static char *application_name = NULL;
static char *logfile_name = NULL;
static FILE *logfile = NULL;
static touchstone_os_threadId PlayThreadId;
static DDSTouchStone_recorderState msg = {0,DDSTouchStone_STOPPED};
static int repeat_mode = FALSE;
static int binary_mode = FALSE;
static int verbose_mode = FALSE;
static int autoplay_mode = FALSE;
static int line_number = 0;

static const char *
boolToStr(
    DDS_boolean val)
{
    const char *result;

    if (val) {
        result = "TRUE";
    } else {
        result = "FALSE";
    }
    return result;
}

static DDS_boolean
strToBool(
    const char *str)
{
    return (strcasecmp("TRUE", str) == 0);
}


typedef enum {
    TD, TQ, RD, RQ, SD, SQ, XD, XQ, STOP
} DDSTouchStoneCommandKind;

static const char *
touchstoneCommandKindEnumToStr(
    DDSTouchStoneCommandKind kind)
{
    static const char *commandKindStrings[] = {
        "TransmitterDef", "TransmitterQos",
        "ReceiverDef",    "ReceiverQos",
        "TransceiverDef", "TransceiverQos",
        "TransponderDef", "TransponderQos",
        "Stopped"
    };

    return commandKindStrings[kind];
}

static DDSTouchStoneCommandKind
touchstoneCommandKindStrToEnum(
    const char *str)
{
    DDSTouchStoneCommandKind currentKind;
    DDSTouchStoneCommandKind result = TD;
    int found = 0;

    for (currentKind = result; (currentKind <= STOP) && !found; currentKind++) {
        found = (strcmp(touchstoneCommandKindEnumToStr(currentKind), str) == 0);
        if (found) {
            result = currentKind;
        }
    }
    if (!found) {
        RECORDER_ERROR("Error in converting DDSTouchStoneCommand from string to enum");
    }

    return result;
}

typedef struct data {
    DDSTouchStoneCommandKind kind;
    DDS_Duration_t delay;
    DDS_boolean disposed;
    union {
        DDSTouchStone_transmitterDef td;
        DDSTouchStone_transmitterQos tq;
        DDSTouchStone_receiverDef    rd;
        DDSTouchStone_receiverQos    rq;
        DDSTouchStone_transceiverDef sd;
        DDSTouchStone_transceiverQos sq;
        DDSTouchStone_transponderDef xd;
        DDSTouchStone_transponderQos xq;
    } command;
} fileData;


#define MAX_STRING_SIZE (1000)

static const char *
fileDataToString(
    fileData *fdata)
{
    static char result[MAX_STRING_SIZE];
    static char commandString[MAX_STRING_SIZE];
    int size;

    switch (fdata->kind) {
        case TD: DDSTouchStone_transmitterDefPrint(commandString, MAX_STRING_SIZE, &(fdata->command.td)); break;
        case TQ: DDSTouchStone_transmitterQosPrint(commandString, MAX_STRING_SIZE, &(fdata->command.tq)); break;
        case RD: DDSTouchStone_receiverDefPrint   (commandString, MAX_STRING_SIZE, &(fdata->command.rd)); break;
        case RQ: DDSTouchStone_receiverQosPrint   (commandString, MAX_STRING_SIZE, &(fdata->command.rq)); break;
        case SD: DDSTouchStone_transceiverDefPrint(commandString, MAX_STRING_SIZE, &(fdata->command.sd)); break;
        case SQ: DDSTouchStone_transceiverQosPrint(commandString, MAX_STRING_SIZE, &(fdata->command.sq)); break;
        case XD: DDSTouchStone_transponderDefPrint(commandString, MAX_STRING_SIZE, &(fdata->command.xd)); break;
        case XQ: DDSTouchStone_transponderQosPrint(commandString, MAX_STRING_SIZE, &(fdata->command.xq)); break;
        case STOP: assert(fdata->kind != STOP); /* Should not happen... */ break;
    }
    size = snprintf(result, MAX_STRING_SIZE,
        "kind=%s "
        "delay= { sec=%d nanosec=%d } "
        "disposed=%s "
        "command= { %s }",
        touchstoneCommandKindEnumToStr(fdata->kind), fdata->delay.sec, fdata->delay.nanosec,
        boolToStr(fdata->disposed), commandString);

    if ((size == MAX_STRING_SIZE) && (result[MAX_STRING_SIZE-1] != '\0')) {
        RECORDER_ERROR("bufferSize too small to fit fileData");
    }

    return result;
}

#undef MAX_STRING_SIZE

static void
fprintDDSTouchStoneCommand(
    FILE *stream,
    fileData *fdata)
{
    int status;
    if (!binary_mode) {
        if (fdata->kind != STOP) {
            fprintf(logfile, "DDSTouchStoneCommand= { %s }\n", fileDataToString(fdata));
        }
    } else {
        status = fwrite(fdata,sizeof(*fdata),1,logfile);
        assert(status == 1);
    }
}

#define MAX_COMMAND_SIZE (50)

static void
scanFileData(
    FILE *stream,
    fileData *fdata,
    int *success)
{
    static char commandKindString[MAX_COMMAND_SIZE];
    static char booleanString[MAX_COMMAND_SIZE];
    int result;

    result = fscanf(stream,
        "kind=%s "
        "delay= { sec=%d nanosec=%d } "
        "disposed=%s ",
        commandKindString, &(fdata->delay.sec), &(fdata->delay.nanosec), booleanString);
    *success = (result == 4);
    if (*success) {
        fdata->kind = touchstoneCommandKindStrToEnum(commandKindString);
        fdata->disposed = strToBool(booleanString);
        fscanf(stream, "command= { ");
        switch(fdata->kind) {
            case TD: DDSTouchStone_transmitterDefScan(stream, &(fdata->command.td), success); break;
            case TQ: DDSTouchStone_transmitterQosScan(stream, &(fdata->command.tq), success); break;
            case RD: DDSTouchStone_receiverDefScan   (stream, &(fdata->command.rd), success); break;
            case RQ: DDSTouchStone_receiverQosScan   (stream, &(fdata->command.rq), success); break;
            case SD: DDSTouchStone_transceiverDefScan(stream, &(fdata->command.sd), success); break;
            case SQ: DDSTouchStone_transceiverQosScan(stream, &(fdata->command.sq), success); break;
            case XD: DDSTouchStone_transponderDefScan(stream, &(fdata->command.xd), success); break;
            case XQ: DDSTouchStone_transponderQosScan(stream, &(fdata->command.xq), success); break;
            case STOP: /* Do nothing */ break;
        }
        fscanf(stream, " }");
    } else {
        RECORDER_ERROR("error scanning DDSTouchStoneCommand from file");
    }
}

static void
reportScanError(
    FILE *stream,
    int line_number)
{
#define LOOKAHEAD_SIZE (30)
    char lookAhead[LOOKAHEAD_SIZE];

    fgets(lookAhead, LOOKAHEAD_SIZE, stream);
    printf("Error in inputfile \"%s\", line %d\n", logfile_name, line_number);
    printf("Violation starts with: %s\n", lookAhead);
#undef LOOKAHEAD_SIZE
}

static void
fscanDDSTouchStoneCommand(
    FILE *stream,
    fileData *fdata,
    int *success)
{
    int length;

    *success = !feof(stream);
    if (*success) {
        if (!binary_mode) {
            fscanf(stream, "DDSTouchStoneCommand= { ");
            scanFileData(stream, fdata, success);
            fscanf(stream, " }\n");
            if (!*success) {
                reportScanError(stream, line_number);
            }
            line_number++;
        } else {
            length = fread(fdata, sizeof(*fdata), 1, logfile);
            *success = (length == 1);
        }
    }
}


static DDS_Time_t start_time = {0,0};

static DDS_Duration_t
get_delay(
    DDSTouchStone_stateKind kind)
{
    DDS_Time_t time;
    DDS_Duration_t delay;

    DDS_DomainParticipant_get_current_time(RecorderParticipant,&time);
    if (kind == DDSTouchStone_COMPOSING) {
        delay.sec = 0;
        delay.nanosec = 0;
    } else {
        if ((start_time.sec == 0) && (start_time.nanosec == 0)) {
            start_time = time;
        }
        delay.sec = time.sec - start_time.sec;
        if (start_time.nanosec <= time.nanosec) {
            delay.nanosec = time.nanosec - start_time.nanosec;
        } else {
            delay.nanosec = time.nanosec + 1000000000 - start_time.nanosec;
            delay.sec -= 1;
            assert(delay.sec >= 0);
        }
    }
    start_time = time;

    return delay;
}

static void
sleep_delay(
    DDS_Duration_t d)
{
    touchstone_os_time delay;

    delay.tv_sec = d.sec;
    delay.tv_nsec = d.nanosec;

    touchstone_os_nanoSleep(delay);
}


static void *
play_thread(
    void *arg)
{
    fileData data;
    int success;
    DDS_ReturnCode_t result;

#define _REPORT_(category) \
    RECORDER_REPORT("Successfully read " #category " touchstone command from file")

    if (logfile == NULL) {
        msg.state = DDSTouchStone_STOPPED;
    } else {
        while (msg.state == DDSTouchStone_PLAYING) {
            fscanDDSTouchStoneCommand(logfile, &data, &success);
            if (success) {
                sleep_delay(data.delay);
                switch (data.kind) {
                case TD:
                    if (data.disposed) {
                        result = DDSTouchStone_transmitterDefDataWriter_dispose (
                                     tdw, &data.command.td, DDS_HANDLE_NIL);
                    } else {
                        result = DDSTouchStone_transmitterDefDataWriter_write (
                                     tdw, &data.command.td, DDS_HANDLE_NIL);
                    }
                    _REPORT_(transmitterDef);
                break;
                case TQ:
                    if (data.disposed) {
                        result = DDSTouchStone_transmitterQosDataWriter_dispose (
                                     tqw, &data.command.tq, DDS_HANDLE_NIL);
                    } else {
                        result = DDSTouchStone_transmitterQosDataWriter_write (
                                     tqw, &data.command.tq, DDS_HANDLE_NIL);
                    }
                    _REPORT_(transmitterQos);
                break;
                case RD:
                    if (data.disposed) {
                        result = DDSTouchStone_receiverDefDataWriter_dispose (
                                     rdw, &data.command.rd, DDS_HANDLE_NIL);
                    } else {
                        result = DDSTouchStone_receiverDefDataWriter_write (
                                     rdw, &data.command.rd, DDS_HANDLE_NIL);
                    }
                    _REPORT_(receiverDef);
                break;
                case RQ:
                    if (data.disposed) {
                        result = DDSTouchStone_receiverQosDataWriter_dispose (
                                     rqw, &data.command.rq, DDS_HANDLE_NIL);
                    } else {
                        result = DDSTouchStone_receiverQosDataWriter_write (
                                     rqw, &data.command.rq, DDS_HANDLE_NIL);
                    }
                    _REPORT_(receiverQos);
                break;
                case SD:
                    if (data.disposed) {
                        result = DDSTouchStone_transceiverDefDataWriter_dispose (
                                     sdw, &data.command.sd, DDS_HANDLE_NIL);
                    } else {
                        result = DDSTouchStone_transceiverDefDataWriter_write (
                                     sdw, &data.command.sd, DDS_HANDLE_NIL);
                    }
                    _REPORT_(transceiverDef);
                break;
                case SQ:
                    if (data.disposed) {
                        result = DDSTouchStone_transceiverQosDataWriter_dispose (
                                     sqw, &data.command.sq, DDS_HANDLE_NIL);
                    } else {
                        result = DDSTouchStone_transceiverQosDataWriter_write (
                                     sqw, &data.command.sq, DDS_HANDLE_NIL);
                    }
                    _REPORT_(transceiverQos);
                break;
                case XD:
                    if (data.disposed) {
                        result = DDSTouchStone_transponderDefDataWriter_dispose (
                                     xdw, &data.command.xd, DDS_HANDLE_NIL);
                    } else {
                        result = DDSTouchStone_transponderDefDataWriter_write (
                                     xdw, &data.command.xd, DDS_HANDLE_NIL);
                    }
                    _REPORT_(transponderDef);
                break;
                case XQ:
                    if (data.disposed) {
                        result = DDSTouchStone_transponderQosDataWriter_dispose (
                                     xqw, &data.command.xq, DDS_HANDLE_NIL);
                    } else {
                        result = DDSTouchStone_transponderQosDataWriter_write (
                                     xqw, &data.command.xq, DDS_HANDLE_NIL);
                    }
                    _REPORT_(transponderQos);
                break;
                case STOP:
//                    msg.state = DDSTouchStone_STOPPED;
                break;
                }
            } else {
                if (repeat_mode) {
                    fclose(logfile);
                    logfile = fopen(logfile_name, "r");
                } else {
                    msg.state = DDSTouchStone_STOPPED;
                }
            }
        }
        fclose(logfile);
    }
    return NULL;

#undef _REPORT_
}

static void
start_play_thread ()
{
    touchstone_os_threadAttr threadAttr;
    touchstone_os_result result;

    result = touchstone_os_threadAttrInit(&threadAttr);

    if (result == touchstone_os_resultSuccess) {
        result = touchstone_os_threadCreate(
                       &PlayThreadId,
                       "RecorderPlayThread",
                       &threadAttr,
                       play_thread,
                       NULL);
        if (result != touchstone_os_resultSuccess) {
        }
    } else {
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
    DDS_sequence_DDSTouchStone_recorderCommand rcl = {0,0,DDS_OBJECT_NIL,FALSE};
    DDS_SampleInfoSeq infoList = { 0, 0, NULL, FALSE };
    DDS_ReturnCode_t  result;
    int length, i, status;
    int proceed = TRUE;
    fileData data;

#define _REPORT_(category) \
    RECORDER_REPORT("Successfully recorded " #category " touchstone command to file")

    if (condition == tdrc) {
#if 0 /* sample count bug */
        result = DDSTouchStone_transmitterDefDataReader_take_w_condition (
                     tdr, &tdl, &infoList, 1, tdrc);
#else
        result = DDSTouchStone_transmitterDefDataReader_take (
                     tdr, &tdl, &infoList, 1,
                     DDS_ANY_SAMPLE_STATE,
                     DDS_ANY_VIEW_STATE,
                     DDS_ANY_INSTANCE_STATE);
#endif
        if (result == DDS_RETCODE_OK) {
            length = tdl._length;
            if (length != 0) {
                for (i=0; ((i<length) &&
                           ((msg.state == DDSTouchStone_RECORDING) ||
                            (msg.state == DDSTouchStone_COMPOSING))); i++) {
                    data.kind = TD;
                    data.delay = get_delay(msg.state);
                    data.command.td = tdl._buffer[i];
                    data.disposed = (infoList._buffer[i].instance_state ==
                                     DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE);
                    fprintDDSTouchStoneCommand(logfile, &data);
                    status = fflush(logfile);
                    assert(status == 0);
                    _REPORT_(transmitterDef);
                }
                result = DDSTouchStone_transmitterDefDataReader_return_loan (
                             tdr, &tdl, &infoList);
            }
        }
    } else if (condition == tqrc) {
#if 0 /* sample count bug */
        result = DDSTouchStone_transmitterQosDataReader_take_w_condition (
                     tqr, &tql, &infoList, 1, tqrc);
#else
        result = DDSTouchStone_transmitterQosDataReader_take (
                     tqr, &tql, &infoList, 1,
                     DDS_ANY_SAMPLE_STATE,
                     DDS_ANY_VIEW_STATE,
                     DDS_ANY_INSTANCE_STATE);
#endif
        if (result == DDS_RETCODE_OK) {
            length = tql._length;
            if (length != 0) {
                for (i=0; ((i<length) &&
                           ((msg.state == DDSTouchStone_RECORDING) ||
                            (msg.state == DDSTouchStone_COMPOSING))); i++) {
                    data.kind = TQ;
                    data.delay = get_delay(msg.state);
                    data.command.tq = tql._buffer[i];
                    data.disposed = (infoList._buffer[i].instance_state ==
                                     DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE);
                    fprintDDSTouchStoneCommand(logfile, &data);
                    status = fflush(logfile);
                    assert(status == 0);
                    _REPORT_(transmitterQos);
                }
                result = DDSTouchStone_transmitterQosDataReader_return_loan (
                             tqr, &tql, &infoList);
            }
        }
    } else if (condition == rdrc) {
#if 0 /* sample count bug */
        result = DDSTouchStone_receiverDefDataReader_take_w_condition (
                     rdr, &rdl, &infoList, 1, rdrc);
#else
        result = DDSTouchStone_receiverDefDataReader_take (
                     rdr, &rdl, &infoList, 1,
                     DDS_ANY_SAMPLE_STATE,
                     DDS_ANY_VIEW_STATE,
                     DDS_ANY_INSTANCE_STATE);
#endif
        if (result == DDS_RETCODE_OK) {
            length = rdl._length;
            if (length != 0) {
                for (i=0; ((i<length) &&
                           ((msg.state == DDSTouchStone_RECORDING) ||
                            (msg.state == DDSTouchStone_COMPOSING))); i++) {
                    data.kind = RD;
                    data.delay = get_delay(msg.state);
                    data.command.rd = rdl._buffer[i];
                    data.disposed = (infoList._buffer[i].instance_state ==
                                     DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE);
                    fprintDDSTouchStoneCommand(logfile, &data);
                    status = fflush(logfile);
                    assert(status == 0);
                    _REPORT_(receiverDef);
                }
                result = DDSTouchStone_receiverDefDataReader_return_loan (
                             rdr, &rdl, &infoList);
            }
        }
    } else if (condition == rqrc) {
#if 0 /* sample count bug */
        result = DDSTouchStone_receiverQosDataReader_take_w_condition (
                     rqr, &rql, &infoList, 1, rqrc);
#else
        result = DDSTouchStone_receiverQosDataReader_take(
                     rqr, &rql, &infoList, 1,
                     DDS_ANY_SAMPLE_STATE,
                     DDS_ANY_VIEW_STATE,
                     DDS_ANY_INSTANCE_STATE);
#endif
        if (result == DDS_RETCODE_OK) {
            length = rql._length;
            if (length != 0) {
                for (i=0; ((i<length) &&
                           ((msg.state == DDSTouchStone_RECORDING) ||
                            (msg.state == DDSTouchStone_COMPOSING))); i++) {
                    data.kind = RQ;
                    data.delay = get_delay(msg.state);
                    data.command.rq = rql._buffer[i];
                    data.disposed = (infoList._buffer[i].instance_state ==
                                     DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE);
                    fprintDDSTouchStoneCommand(logfile, &data);
                    status = fflush(logfile);
                    assert(status == 0);
                    _REPORT_(receiverQos);
                }
                result = DDSTouchStone_receiverQosDataReader_return_loan (
                             rqr, &rql, &infoList);
            }
        }
    } else if (condition == sdrc) {
#if 0 /* sample count bug */
        result = DDSTouchStone_transceiverDefDataReader_take_w_condition (
                     sdr, &sdl, &infoList, 1, sdrc);
#else
        result = DDSTouchStone_transceiverDefDataReader_take(
                     sdr, &sdl, &infoList, 1,
                     DDS_ANY_SAMPLE_STATE,
                     DDS_ANY_VIEW_STATE,
                     DDS_ANY_INSTANCE_STATE);
#endif
        if (result == DDS_RETCODE_OK) {
            length = sdl._length;
            if (length != 0) {
                for (i=0; ((i<length) &&
                           ((msg.state == DDSTouchStone_RECORDING) ||
                            (msg.state == DDSTouchStone_COMPOSING))); i++) {
                    data.kind = SD;
                    data.delay = get_delay(msg.state);
                    data.command.sd = sdl._buffer[i];
                    data.disposed = (infoList._buffer[i].instance_state ==
                                     DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE);

                    fprintDDSTouchStoneCommand(logfile, &data);
                    status = fflush(logfile);
                    assert(status == 0);
                    _REPORT_(transceiverDef);
                }
                result = DDSTouchStone_transceiverDefDataReader_return_loan (
                             sdr, &sdl, &infoList);
            }
        }
    } else if (condition == sqrc) {
#if 0 /* sample count bug */
        result = DDSTouchStone_transceiverQosDataReader_take_w_condition (
                     sqr, &sql, &infoList, 1, sqrc);
#else
        result = DDSTouchStone_transceiverQosDataReader_take(
                     sqr, &sql, &infoList, 1,
                     DDS_ANY_SAMPLE_STATE,
                     DDS_ANY_VIEW_STATE,
                     DDS_ANY_INSTANCE_STATE);
#endif
        if (result == DDS_RETCODE_OK) {
            length = sql._length;
            if (length != 0) {
                for (i=0; ((i<length) &&
                           ((msg.state == DDSTouchStone_RECORDING) ||
                            (msg.state == DDSTouchStone_COMPOSING))); i++) {
                    data.kind = SQ;
                    data.delay = get_delay(msg.state);
                    data.command.sq = sql._buffer[i];
                    data.disposed = (infoList._buffer[i].instance_state ==
                                     DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE);
                    fprintDDSTouchStoneCommand(logfile, &data);
                    status = fflush(logfile);
                    assert(status == 0);
                    _REPORT_(transceiverQos);
                }
                result = DDSTouchStone_transceiverQosDataReader_return_loan (
                             sqr, &sql, &infoList);
            }
        }
    } else if (condition == xdrc) {
#if 0 /* sample count bug */
        result = DDSTouchStone_transponderDefDataReader_take_w_condition (
                     xdr, &xdl, &infoList, 1, xdrc);
#else
        result = DDSTouchStone_transponderDefDataReader_take(
                     xdr, &xdl, &infoList, 1,
                     DDS_ANY_SAMPLE_STATE,
                     DDS_ANY_VIEW_STATE,
                     DDS_ANY_INSTANCE_STATE);
#endif
        if (result == DDS_RETCODE_OK) {
            length = xdl._length;
            if (length != 0) {
                for (i=0; ((i<length) &&
                           ((msg.state == DDSTouchStone_RECORDING) ||
                            (msg.state == DDSTouchStone_COMPOSING))); i++) {
                    data.kind = XD;
                    data.delay = get_delay(msg.state);
                    data.command.xd = xdl._buffer[i];
                    data.disposed = (infoList._buffer[i].instance_state ==
                                     DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE);
                    fprintDDSTouchStoneCommand(logfile, &data);
                    status = fflush(logfile);
                    assert(status == 0);
                    _REPORT_(transponderDef);
                }
                result = DDSTouchStone_transponderDefDataReader_return_loan (
                             xdr, &xdl, &infoList);
            }
        }
    } else if (condition == xqrc) {
#if 0 /* sample count bug */
        result = DDSTouchStone_transponderQosDataReader_take_w_condition (
                     xqr, &xql, &infoList, 1, xqrc);
#else
        result = DDSTouchStone_transponderQosDataReader_take(
                     xqr, &xql, &infoList, 1,
                     DDS_ANY_SAMPLE_STATE,
                     DDS_ANY_VIEW_STATE,
                     DDS_ANY_INSTANCE_STATE);
#endif
        if (result == DDS_RETCODE_OK) {
            length = xql._length;
            if (length != 0) {
                for (i=0; ((i<length) &&
                           ((msg.state == DDSTouchStone_RECORDING) ||
                            (msg.state == DDSTouchStone_COMPOSING))); i++) {
                    data.kind = XQ;
                    data.delay = get_delay(msg.state);
                    data.command.xq = xql._buffer[i];
                    data.disposed = (infoList._buffer[i].instance_state ==
                                     DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE);
                    fprintDDSTouchStoneCommand(logfile, &data);
                    status = fflush(logfile);
                    assert(status == 0);
                    _REPORT_(transponderQos);
                }
                result = DDSTouchStone_transponderQosDataReader_return_loan (
                             xqr, &xql, &infoList);
            }
        }
#undef _REPORT_
    } else if (condition == rcrc) {
#define _REPORT_(category) \
    RECORDER_REPORT("Received recorder command " #category)
#if 0 /* sample count bug */
        result = DDSTouchStone_recorderCommandDataReader_take_w_condition (
                     rcr, &rcl, &infoList, 1, rcrc);
#else
        result = DDSTouchStone_recorderCommandDataReader_take(
                     rcr, &rcl, &infoList, 1,
                     DDS_ANY_SAMPLE_STATE,
                     DDS_ANY_VIEW_STATE,
                     DDS_ANY_INSTANCE_STATE);
#endif
        if (result == DDS_RETCODE_OK) {
            length = rcl._length;
            if (length != 0) {
                for (i=0; i<length; i++) {
                    repeat_mode = FALSE;
                    switch (rcl._buffer[i].command) {
                    case DDSTouchStone_STOP:
                         if (msg.state == DDSTouchStone_PLAYING) {
                             msg.state = DDSTouchStone_STOPPED;
                             result = touchstone_os_threadWaitExit(PlayThreadId, NULL);
                             if (result != touchstone_os_resultSuccess) {
                             }
                             fclose(logfile);
                         }
                         if (msg.state == DDSTouchStone_COMPOSING) {
                             fclose(logfile);
                             msg.state = DDSTouchStone_STOPPED;
                         }
                         if (msg.state == DDSTouchStone_RECORDING) {
                             data.kind = STOP;
                             data.delay = get_delay(msg.state);
                             fprintDDSTouchStoneCommand(logfile, &data);
                             status = fflush(logfile);
                             assert(status == 0);
                             fclose(logfile);
                             msg.state = DDSTouchStone_STOPPED;
                         }
                         _REPORT_(STOP);
                    break;
                    case DDSTouchStone_RECORD:
                         if (msg.state == DDSTouchStone_PLAYING) {
                             msg.state = DDSTouchStone_STOPPED;
                             result = touchstone_os_threadWaitExit(PlayThreadId, NULL);
                             if (result != touchstone_os_resultSuccess) {
                             }
                             fclose(logfile);
                         }
                         if (msg.state == DDSTouchStone_COMPOSING) {
                             fclose(logfile);
                             msg.state = DDSTouchStone_STOPPED;
                         }
                         if (msg.state == DDSTouchStone_STOPPED) {
                             logfile = fopen(logfile_name, "a");
                             msg.state = DDSTouchStone_RECORDING;
                         }
                         _REPORT_(RECORD);
                    break;
                    case DDSTouchStone_COMPOSE:
                         if (msg.state == DDSTouchStone_PLAYING) {
                             msg.state = DDSTouchStone_STOPPED;
                             result = touchstone_os_threadWaitExit(PlayThreadId, NULL);
                             if (result != touchstone_os_resultSuccess) {
                             }
                             fclose(logfile);
                         }
                         if (msg.state == DDSTouchStone_RECORDING) {
                             data.kind = STOP;
                             data.delay = get_delay(msg.state);
                             fprintDDSTouchStoneCommand(logfile, &data);
                             status = fflush(logfile);
                             assert(status == 0);
                             fclose(logfile);
                             msg.state = DDSTouchStone_STOPPED;
                         }
                         if (msg.state == DDSTouchStone_STOPPED) {
                             logfile = fopen(logfile_name, "a");
                             msg.state = DDSTouchStone_COMPOSING;
                         }
                         _REPORT_(COMPOSE);
                    break;
                    case DDSTouchStone_REPEAT:
                         repeat_mode = TRUE;
                    case DDSTouchStone_PLAY:
                         if (msg.state == DDSTouchStone_RECORDING) {
                             data.kind = STOP;
                             data.delay = get_delay(msg.state);
                             fprintDDSTouchStoneCommand(logfile, &data);
                             status = fflush(logfile);
                             assert(status == 0);
                             fclose(logfile);
                             msg.state = DDSTouchStone_STOPPED;
                         }
                         if (msg.state == DDSTouchStone_COMPOSING) {
                             fclose(logfile);
                             msg.state = DDSTouchStone_STOPPED;
                         }
                         if (msg.state == DDSTouchStone_STOPPED) {
                             msg.state = DDSTouchStone_PLAYING;
                             logfile = fopen(logfile_name, "r");
                             start_play_thread();
                         }
                         _REPORT_(PLAY);
                    break;
                    case DDSTouchStone_QUIT:
                         if (msg.state == DDSTouchStone_RECORDING) {
                             data.kind = STOP;
                             data.delay = get_delay(msg.state);
                             fprintDDSTouchStoneCommand(logfile, &data);
                             status = fflush(logfile);
                             assert(status == 0);
                             fclose(logfile);
                         } else if (msg.state == DDSTouchStone_COMPOSING) {
                             fclose(logfile);
                         } else if (msg.state == DDSTouchStone_PLAYING) {
                             msg.state = DDSTouchStone_STOPPED;
                             result = touchstone_os_threadWaitExit(PlayThreadId, NULL);
                             if (result != touchstone_os_resultSuccess) {
                             }
                             fclose(logfile);
                         }
                         msg.state = DDSTouchStone_TERMINATED;
                         proceed = FALSE;
                         _REPORT_(QUIT);
                    break;
                    }
                    result = DDSTouchStone_recorderStateDataWriter_write (
                                 rsw, &msg, DDS_HANDLE_NIL);
                }
                result = DDSTouchStone_recorderCommandDataReader_return_loan (
                             rcr, &rcl, &infoList);
            }
        }
    } else {
        RECORDER_ERROR("Waitset Returned an unknown condition");
    }
    return proceed;
}
#undef _REPORT_


static void
printUsage(
    char *name)
{
    printf("Usage: %s [-b] [-v] <recorder_id> [ <file_name> ]\n",name);
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
        if _PEQUALS_("-b") {
            binary_mode = TRUE;
            currArg++;
        } else if _PEQUALS_("-h") {
            printUsage(application_name);
            currArg++;
        } else if _PEQUALS_("-v") {
            verbose_mode = TRUE;
            currArg++;
        } else if _PEQUALS_("--autoplay") {
            autoplay_mode = TRUE;
            currArg++;
        }
    }
#undef _PEQUALS_

    /* Then read arguments */
    if (currArg < argc) {
       msg.recorder_id = atoi(argv[currArg]);
       currArg++;
    } else {
        printUsage(application_name);
        printf("recorder_id missing, exiting now.\n");
        exit(-1);
    }

    if (currArg < argc) {
        logfile_name = DDS_string_dup(argv[currArg]);
        currArg++;
    } else {
        static char scratch[80];
        snprintf(scratch, 80, "%s_%d.dat",application_name, msg.recorder_id);
        logfile_name = DDS_string_dup(scratch);
    }


    if (currArg < argc) {
        printUsage(application_name);
        printf("Ignored extra parameters.\n");
    }

    if (binary_mode) {
        RECORDER_REPORT("Switched on binary backwards compatibility mode");
    }
    if (verbose_mode) {
        RECORDER_REPORT("Switched on verbose mode");
    }
    RECORDER_REPORT_1("Using recorder_id %d", msg.recorder_id);
    RECORDER_REPORT_1("Using logfile \"%s\"", logfile_name);
}

int
main (
    int argc,
    char *argv[])
{
    DDS_WaitSet RecorderWaitset;
    DDS_Duration_t RecorderTimeout = DDS_DURATION_INFINITE;
    DDS_ReturnCode_t result;
    DDS_ConditionSeq *conditionList;
    DDS_TopicQos     *tQos;
    DDS_PublisherQos *pQos;
    DDS_SubscriberQos *sQos;
    DDS_DataWriterQos *wQos;
    DDS_DataReaderQos *rQos;
    DDS_StringSeq recorder_id;
    char scratch[20];

    int length, i, proceed;

    readCommandLineParams(argc, argv);

    recorder_id._maximum   = 1;
    recorder_id._length    = 1;
    recorder_id._buffer    = DDS_StringSeq_allocbuf(1);
    snprintf(scratch, 20, "%d", msg.recorder_id);
    recorder_id._buffer[0] = DDS_string_dup(scratch);

    logfile = fopen(logfile_name, "a");
    if (logfile == NULL) {
        printf ("Recorder %s: ERROR - failed to open logfile %s\n",
                application_name, logfile_name);
        exit(1);
    }

    /*
     * Create WaitSet
     */
    RecorderWaitset = DDS_WaitSet__alloc ();

    /*
     * Create participant
     */
    RecorderFactory = DDS_DomainParticipantFactory_get_instance ();

    if (RecorderFactory == DDS_HANDLE_NIL) {
        printf ("Recorder %s: ERROR - missing factory instance\n",
                application_name);
        exit (1);
    }

    RecorderParticipant = DDS_DomainParticipantFactory_create_participant (
                             RecorderFactory,
                             RecorderDomain,
                             DDS_PARTICIPANT_QOS_DEFAULT,
                             NULL,
                             DDS_ANY_STATUS);

    if (RecorderParticipant == DDS_HANDLE_NIL) {
        printf ("Recorder %s: ERROR - OpenSplice not running\n",
                application_name);
        exit (1);
    }

    pQos = DDS_PublisherQos__alloc();

    DDS_DomainParticipant_get_default_publisher_qos (RecorderParticipant,
                                                     pQos);

    pQos->partition.name._length = 1;
    pQos->partition.name._maximum = 1;
    pQos->partition.name._buffer = DDS_StringSeq_allocbuf (1);
    pQos->partition.name._buffer[0] = DDS_string_dup(DDSTouchStoneCommandPartition);

    RecorderPublisher = DDS_DomainParticipant_create_publisher (
                             RecorderParticipant,
                             pQos,
                             NULL,
                             DDS_ANY_STATUS);

    DDS_free(pQos);

    sQos = DDS_SubscriberQos__alloc();

    DDS_DomainParticipant_get_default_subscriber_qos (RecorderParticipant,
                                                      sQos);

    sQos->partition.name._length = 1;
    sQos->partition.name._maximum = 1;
    sQos->partition.name._buffer = DDS_StringSeq_allocbuf (1);
    sQos->partition.name._buffer[0] = DDS_string_dup(DDSTouchStoneCommandPartition);

    RecorderSubscriber = DDS_DomainParticipant_create_subscriber (
                             RecorderParticipant,
                             sQos,
                             NULL,
                             DDS_ANY_STATUS);

    DDS_free(sQos);

    tQos = DDS_TopicQos__alloc();

    DDS_DomainParticipant_get_default_topic_qos(RecorderParticipant, tQos);
    tQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    tQos->durability.kind = DDS_TRANSIENT_DURABILITY_QOS;

    wQos = DDS_DataWriterQos__alloc();
    DDS_Publisher_get_default_datawriter_qos(RecorderPublisher, wQos);
    DDS_Publisher_copy_from_topic_qos(RecorderPublisher, wQos, tQos);

    rQos = DDS_DataReaderQos__alloc();
    DDS_Subscriber_get_default_datareader_qos(RecorderSubscriber, rQos);
    DDS_Subscriber_copy_from_topic_qos(RecorderSubscriber, rQos, tQos);

    /*
     * Transmitter
     */

    /*  Create Definition Topic */
    tds = DDSTouchStone_transmitterDefTypeSupport__alloc ();

    DDSTouchStone_transmitterDefTypeSupport_register_type (
              tds,
              RecorderParticipant,
              "DDSTouchStone::transmitterDef");

    tdt = DDS_DomainParticipant_create_topic (
              RecorderParticipant,
              "transmitterDefTopic",
              "DDSTouchStone::transmitterDef",
              DDS_TOPIC_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Create Definition datawriter */
    tdw = DDS_Publisher_create_datawriter (
              RecorderPublisher,
              tdt,
              DDS_DATAWRITER_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Create Definition datareader */
    tdr = DDS_Subscriber_create_datareader (
              RecorderSubscriber,
              tdt,
              DDS_DATAREADER_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Add datareader readcondition to waitset */

    tdrc = DDS_DataReader_create_readcondition (
              tdr,
              DDS_ANY_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    result = DDS_WaitSet_attach_condition (RecorderWaitset, tdrc);

    /*  Create Qos Topic */
    tqs = DDSTouchStone_transmitterQosTypeSupport__alloc ();

    DDSTouchStone_transmitterQosTypeSupport_register_type (
              tqs,
              RecorderParticipant,
              "DDSTouchStone::transmitterQos");

    tqt = DDS_DomainParticipant_create_topic (
              RecorderParticipant,
              "transmitterQosTopic",
              "DDSTouchStone::transmitterQos",
              tQos,
              NULL,
              DDS_ANY_STATUS);

    /* Create Qos datawriter */
    tqw = DDS_Publisher_create_datawriter (
              RecorderPublisher,
              tqt,
              wQos,
              NULL,
              DDS_ANY_STATUS);

    /* Create Qos datareader */
    tqr = DDS_Subscriber_create_datareader (
              RecorderSubscriber,
              tqt,
              rQos,
              NULL,
              DDS_ANY_STATUS);

    /* Add datareader readcondition to waitset */

    tqrc = DDS_DataReader_create_readcondition (
              tqr,
              DDS_ANY_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    result = DDS_WaitSet_attach_condition (RecorderWaitset, tqrc);

    /*
     * Receiver
     */

    /*  Create Definition Topic */
    rds = DDSTouchStone_receiverDefTypeSupport__alloc ();

    DDSTouchStone_receiverDefTypeSupport_register_type (
              rds,
              RecorderParticipant,
              "DDSTouchStone::receiverDef");

    rdt = DDS_DomainParticipant_create_topic (
              RecorderParticipant,
              "receiverDefTopic",
              "DDSTouchStone::receiverDef",
              DDS_TOPIC_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Create datawriter */
    rdw = DDS_Publisher_create_datawriter (
              RecorderPublisher,
              rdt,
              DDS_DATAWRITER_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Create datareader */
    rdr = DDS_Subscriber_create_datareader (
              RecorderSubscriber,
              rdt,
              DDS_DATAREADER_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Add datareader readcondition to waitset */

    rdrc = DDS_DataReader_create_readcondition (
              rdr,
              DDS_ANY_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    result = DDS_WaitSet_attach_condition (RecorderWaitset, rdrc);

    /*  Create Qos Policy Topic */
    rqs = DDSTouchStone_receiverQosTypeSupport__alloc ();

    DDSTouchStone_receiverQosTypeSupport_register_type (
              rqs,
              RecorderParticipant,
              "DDSTouchStone::receiverQos");

    rqt = DDS_DomainParticipant_create_topic (
              RecorderParticipant,
              "receiverQosTopic",
              "DDSTouchStone::receiverQos",
              tQos,
              NULL,
              DDS_ANY_STATUS);

    /* Create datawriter */
    rqw = DDS_Publisher_create_datawriter (
              RecorderPublisher,
              rqt,
              wQos,
              NULL,
              DDS_ANY_STATUS);

    /* Create datareader */
    rqr = DDS_Subscriber_create_datareader (
              RecorderSubscriber,
              rqt,
              rQos,
              NULL,
              DDS_ANY_STATUS);

    /* Add datareader readcondition to waitset */

    rqrc = DDS_DataReader_create_readcondition (
              rqr,
              DDS_ANY_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    result = DDS_WaitSet_attach_condition (RecorderWaitset, rqrc);

    /*
     * transceiver
     */

    /*  Create Definition Topic */
    sds = DDSTouchStone_transceiverDefTypeSupport__alloc ();

    DDSTouchStone_transceiverDefTypeSupport_register_type (
              sds, RecorderParticipant,
              "DDSTouchStone::transceiverDef");

    sdt = DDS_DomainParticipant_create_topic (
              RecorderParticipant,
              "transceiverDefTopic",
              "DDSTouchStone::transceiverDef",
              DDS_TOPIC_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Create datawriter */
    sdw = DDS_Publisher_create_datawriter (
              RecorderPublisher,
              sdt,
              DDS_DATAWRITER_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Create datareader */
    sdr = DDS_Subscriber_create_datareader (
              RecorderSubscriber,
              sdt,
              DDS_DATAREADER_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Add datareader readcondition to waitset */

    sdrc = DDS_DataReader_create_readcondition (
              sdr,
              DDS_ANY_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    result = DDS_WaitSet_attach_condition (RecorderWaitset, sdrc);

    /*  Create Qos Policy Topic */
    sqs = DDSTouchStone_transceiverQosTypeSupport__alloc ();

    DDSTouchStone_transceiverQosTypeSupport_register_type (
              sqs, RecorderParticipant,
              "DDSTouchStone::transceiverQos");

    sqt = DDS_DomainParticipant_create_topic (
              RecorderParticipant,
              "transceiverQosTopic",
              "DDSTouchStone::transceiverQos",
              tQos,
              NULL,
              DDS_ANY_STATUS);

    /* Create datawriter */
    sqw = DDS_Publisher_create_datawriter (
              RecorderPublisher,
              sqt,
              wQos,
              NULL,
              DDS_ANY_STATUS);

    /* Create datareader */
    sqr = DDS_Subscriber_create_datareader (
              RecorderSubscriber,
              sqt,
              rQos,
              NULL,
              DDS_ANY_STATUS);

    /* Add datareader readcondition to waitset */

    sqrc = DDS_DataReader_create_readcondition (
              sqr,
              DDS_ANY_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    result = DDS_WaitSet_attach_condition (RecorderWaitset, sqrc);

    /*
     * transponder
     */

    /*  Create Definition Topic */
    xds = DDSTouchStone_transponderDefTypeSupport__alloc ();

    DDSTouchStone_transponderDefTypeSupport_register_type (
              xds, RecorderParticipant,
              "DDSTouchStone::transponderDef");

    xdt = DDS_DomainParticipant_create_topic (
              RecorderParticipant,
              "transponderDefTopic",
              "DDSTouchStone::transponderDef",
              DDS_TOPIC_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Create datawriter */
    xdw = DDS_Publisher_create_datawriter (
              RecorderPublisher,
              xdt,
              DDS_DATAWRITER_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Create datareader */
    xdr = DDS_Subscriber_create_datareader (
              RecorderSubscriber,
              xdt,
              DDS_DATAREADER_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Add datareader readcondition to waitset */

    xdrc = DDS_DataReader_create_readcondition (
              xdr,
              DDS_ANY_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    result = DDS_WaitSet_attach_condition (RecorderWaitset, xdrc);

    /*  Create Qos Policy Topic */
    xqs = DDSTouchStone_transponderQosTypeSupport__alloc ();

    DDSTouchStone_transponderQosTypeSupport_register_type (
              xqs, RecorderParticipant,
              "DDSTouchStone::transponderQos");

    xqt = DDS_DomainParticipant_create_topic (
              RecorderParticipant,
              "transponderQosTopic",
              "DDSTouchStone::transponderQos",
              tQos,
              NULL,
              DDS_ANY_STATUS);

    /* Create datawriter */
    xqw = DDS_Publisher_create_datawriter (
              RecorderPublisher,
              xqt,
              wQos,
              NULL,
              DDS_ANY_STATUS);

    /* Create datareader */
    xqr = DDS_Subscriber_create_datareader (
              RecorderSubscriber,
              xqt,
              rQos,
              NULL,
              DDS_ANY_STATUS);

    /* Add datareader readcondition to waitset */

    xqrc = DDS_DataReader_create_readcondition (
              xqr,
              DDS_ANY_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    result = DDS_WaitSet_attach_condition (RecorderWaitset, xqrc);

    /*  Create Command Topic */
    rcs = DDSTouchStone_recorderCommandTypeSupport__alloc ();

    DDSTouchStone_recorderCommandTypeSupport_register_type (
              rcs, RecorderParticipant,
              "DDSTouchStone::recorderCommand");

    rct = DDS_DomainParticipant_create_topic (
              RecorderParticipant,
              "recorderCommandTopic",
              "DDSTouchStone::recorderCommand",
              DDS_TOPIC_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /*  Create Content Filtered Topic */
    rcf = DDS_DomainParticipant_create_contentfilteredtopic (
              RecorderParticipant,
              "recorderCommandFilteredTopic",
              rct,
              "recorder_id = %0",
              &recorder_id);

    /* Create datareader */
    rcr = DDS_Subscriber_create_datareader (
              RecorderSubscriber,
              rcf,
              DDS_DATAREADER_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Add datareader readcondition to waitset */

    rcrc = DDS_DataReader_create_readcondition (
              rcr,
              DDS_ANY_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    result = DDS_WaitSet_attach_condition (RecorderWaitset, rcrc);

    /*  Create Command Topic */

    /*  Create Command Topic */
    rss = DDSTouchStone_recorderStateTypeSupport__alloc ();

    DDSTouchStone_recorderStateTypeSupport_register_type (
              rss, RecorderParticipant,
              "DDSTouchStone::recorderState");

    rst = DDS_DomainParticipant_create_topic (
              RecorderParticipant,
              "recorderStateTopic",
              "DDSTouchStone::recorderState",
              tQos,
              NULL,
              DDS_ANY_STATUS);

    /* Create datawriter */
    rsw = DDS_Publisher_create_datawriter (
              RecorderPublisher,
              rst,
              wQos,
              NULL,
              DDS_ANY_STATUS);

    /* publish initial state */
    result = DDSTouchStone_recorderStateDataWriter_write (
             rsw, &msg, DDS_HANDLE_NIL);

    DDS_free(rQos);
    DDS_free(wQos);
    DDS_free(tQos);

    /* Create datareader */
    /*
     * MainLoop processing
     */
    /* Start playing record file first, if requested */
    if (autoplay_mode) {
        msg.state = DDSTouchStone_PLAYING;
        logfile = fopen(logfile_name, "r");
        RECORDER_REPORT_1("Autoplaying recorder file \"%s\"", logfile_name);
        start_play_thread();
    }
    /* Continue with waiting for commands */
    proceed = TRUE;
    while (proceed) {
        conditionList = DDS_ConditionSeq__alloc();
        result = DDS_WaitSet_wait (RecorderWaitset,
                                   conditionList,
                                   &RecorderTimeout);
        length = conditionList->_length;
        for (i = 0; (i < length) && proceed; i++) {
            proceed = process_command(conditionList->_buffer[i]);
        }
        DDS_free(conditionList);
    }
    return 0;
}

