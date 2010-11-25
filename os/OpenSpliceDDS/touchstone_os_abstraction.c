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
#include "_touchstone_os_abstraction.h"
#include "touchstone_os_abstraction.h"
/* Implementation */

#define MIN_SCHED_PRIO  (0)
#define MAX_SCHED_PRIO (99)

static touchstone_os_result
resultOsToTouchStone(
    os_result result)
{
    touchstone_os_result rv;

    if (result == os_resultSuccess) {
        rv = touchstone_os_resultSuccess;
    } else {
        rv = touchstone_os_resultFail;
    }
    return rv;
}

static touchstone_os_schedClass
schedClassOsToTouchStone(
    os_schedClass osSchedClass)
{
    touchstone_os_schedClass result;

    switch(osSchedClass) {
        case OS_SCHED_DEFAULT:
            result = TOUCHSTONE_OS_SCHED_DEFAULT;
        break;
        case OS_SCHED_REALTIME:
            result = TOUCHSTONE_OS_SCHED_REALTIME;
        break;
        case OS_SCHED_TIMESHARE:
            result = TOUCHSTONE_OS_SCHED_TIMESHARE;
        break;
    }
    return result;
}

static os_schedClass
schedClassTouchStoneToOS(
    touchstone_os_schedClass schedClass)
{
    os_schedClass result;

    switch (schedClass) {
        case TOUCHSTONE_OS_SCHED_DEFAULT:
            result = OS_SCHED_DEFAULT;
        break;
        case TOUCHSTONE_OS_SCHED_REALTIME:
            result = OS_SCHED_REALTIME;
        break;
        case TOUCHSTONE_OS_SCHED_TIMESHARE:
            result = OS_SCHED_TIMESHARE;
        break;
    }
    return result;
}


/* ----------------- Threading functions -------------------- */


touchstone_os_result
touchstone_os_threadAttrInit(
    touchstone_os_threadAttr *attr)
{
    os_threadAttr osAttr;
    os_result result;
    touchstone_os_result rv;


    osAttr.schedClass = schedClassTouchStoneToOS(attr->schedClass);
    osAttr.schedPriority = attr->schedPriority;
    osAttr.stackSize = attr->stackSize;

    result = os_threadAttrInit(&osAttr);
    rv = resultOsToTouchStone(result);
    if (result == os_resultSuccess) {
        attr->schedClass = schedClassOsToTouchStone(osAttr.schedClass);
        attr->schedPriority = osAttr.schedPriority;
        attr->stackSize = osAttr.stackSize;
    }
    return rv;
}


touchstone_os_result
touchstone_os_threadWaitExit(
    touchstone_os_threadId threadId,
    void **result_value)
{
    touchstone_os_result rv;
    os_result result;

    result = os_threadWaitExit(threadId, result_value);
    rv = resultOsToTouchStone(result);

    return rv;
}


touchstone_os_result
touchstone_os_threadCreate (
    touchstone_os_threadId *threadId,
    const char *name,
    const touchstone_os_threadAttr *threadAttr,
    void *(* start_routine)(void *),
    void *arg)
{
    os_threadAttr osThreadAttr;
    os_result result;
    touchstone_os_result rv;


    osThreadAttr.schedClass = schedClassTouchStoneToOS(threadAttr->schedClass);
    osThreadAttr.schedPriority = threadAttr->schedPriority;
    osThreadAttr.stackSize = threadAttr->stackSize;

    result = os_threadCreate(threadId, name, &osThreadAttr, start_routine, arg);
    rv = resultOsToTouchStone(result);

    return rv;
}

void
touchstone_os_threadExit (
    void *thread_result)
{
    os_threadExit(thread_result);
    return;
}


/* ------------------- Sleep functions ---------------------- */

touchstone_os_result
touchstone_os_nanoSleep (
    touchstone_os_time delay)
{
    os_time t;
    os_result result;
    touchstone_os_result rv;

    t.tv_sec = delay.tv_sec;
    t.tv_nsec = delay.tv_nsec;

    result = os_nanoSleep(t);
    rv = resultOsToTouchStone(result);

    return rv;
}

/* ----------------- File and dir functions ----------------- */

int
touchstone_os_mkdir(
    const char *path)
{
    int result;

    result = os_mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);

    return result;
}


/* --------------------- Timer class ------------------------ */

touchstone_os_timer
touchstone_os_timerCreate(
    touchstone_os_time interval)
{
    touchstone_os_timer result;

    result = (touchstone_os_timer)malloc(sizeof(*result));
    if (result != NULL) {
        result->interval_sec = interval.tv_sec;
        result->interval_nsec = interval.tv_nsec;
    }
    return result;
}

void
touchstone_os_timerDestroy(
    touchstone_os_timer _this)
{
    if (_this != NULL) {
        free(_this);
    }
}

void
touchstone_os_timerStart(
    touchstone_os_timer _this)
{
    /* Nothing for this abstraction layer */
}

void
touchstone_os_timerWait(
    touchstone_os_timer _this)
{
    touchstone_os_time interval;
    if (_this != NULL) {
        interval.tv_sec = _this->interval_sec;
        interval.tv_nsec = _this->interval_nsec;
        touchstone_os_nanoSleep(interval);
    }
}


void
touchstone_os_timerIntervalSet(
    touchstone_os_timer _this,
    touchstone_os_time  interval)
{
    if (_this != NULL) {
        _this->interval_sec = interval.tv_sec;
        _this->interval_nsec = interval.tv_nsec;
    }
}


touchstone_os_time
touchstone_os_timerIntervalGet(
    touchstone_os_timer _this)
{
    touchstone_os_time result = {0,0};

    if (_this != NULL) {
        result.tv_sec = _this->interval_sec;
        result.tv_nsec = _this->interval_nsec;
    }

    return result;
}

