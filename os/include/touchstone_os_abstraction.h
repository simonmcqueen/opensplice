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

#ifndef __TOUCHSTONE_OS_ABSTRACTION_H
#define __TOUCHSTONE_OS_ABSTRACTION_H

#include "_touchstone_os_abstraction.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* ------------------- os dependent types ------------------- */

/* The actual types _touchstone_os_xxx are defined in the
   os-specific subdirectories, in the files called
   _touchstone_os_abstraction.h */
typedef _touchstone_os_threadId   touchstone_os_threadId;
typedef _touchstone_os_timeSec    touchstone_os_timeSec;


/* --------------- simple enumeration types ----------------- */

typedef enum touchstone_os_result {
    /** The call is successfuly completed */
    touchstone_os_resultSuccess,
    /** The operating system returned a failure */
    touchstone_os_resultFail
} touchstone_os_result;

typedef enum touchstone_os_schedClass {
    /** Schedule processes and threads according a platform default.
     *  OS_SCHED_REALTIME for timesharing platforms and
     *  OS_SCHED_TIMESHARE for realtime platforms
     */
    TOUCHSTONE_OS_SCHED_DEFAULT,
    /** Schedule processes and threads on realtime basis,
     *  on most platforms implying:
     *  - Fixed Priority
     *  - Preemption
     *  - Either "First In First Out" or "Round Robin"
     */
    TOUCHSTONE_OS_SCHED_REALTIME,
    /** Schedule processes and threads on timesharing basis,
     *  on most platforms implying:
     *  - Dynamic Priority to guarantee fair share
     *  - Preemption
     */
    TOUCHSTONE_OS_SCHED_TIMESHARE
} touchstone_os_schedClass;


/* ------------------- structured types -------------------- */

typedef int touchstone_os_priority;
typedef struct touchstone_os_threadAttr_s {
    /** Specifies the scheduling class */
    touchstone_os_schedClass  schedClass;
    /** Specifies the thread priority */
    touchstone_os_priority	  schedPriority;
    /** Specifies the thread stack size */
    unsigned int	  stackSize;
} touchstone_os_threadAttr;


typedef struct touchstone_os_time_s {
    /** Seconds since 1-jan-1970 00:00 */
    touchstone_os_timeSec tv_sec;
    /** Count of nanoseconds within the second */
    int               tv_nsec;
} touchstone_os_time;



/* -------------------- Threading functions ----------------- */

touchstone_os_result
touchstone_os_threadAttrInit(
    touchstone_os_threadAttr *attr);

touchstone_os_result
touchstone_os_threadWaitExit(
    touchstone_os_threadId threadId,
    void **result_value);

touchstone_os_result
touchstone_os_threadCreate (
    touchstone_os_threadId *threadId,
    const char *name,
    const touchstone_os_threadAttr *threadAttr,
    void *(* start_routine)(void *),
    void *arg);

void
touchstone_os_threadExit (
    void *thread_result);


/* ---------------------- Sleep functions ------------------- */

touchstone_os_result
touchstone_os_nanoSleep (
    touchstone_os_time delay);


/* ----------------- File and dir functions ----------------- */

int
touchstone_os_mkdir(
    const char *path);

/* ------------------------ Timer class --------------------- */

/* The actual type _touchstone_os_timer_s is defined in the
   os-specific subdirectories, in the files called
   _touchstone_os_abstraction.h */
typedef struct _touchstone_os_timer_s *touchstone_os_timer;


/* Constructor/destructor */

touchstone_os_timer
touchstone_os_timerCreate(
    touchstone_os_time interval);

void
touchstone_os_timerDestroy(
    touchstone_os_timer _this);



/* Public methods */

void
touchstone_os_timerStart(
    touchstone_os_timer _this);

void
touchstone_os_timerWait(
    touchstone_os_timer _this);


/* Getters/setters */

void
touchstone_os_timerIntervalSet(
    touchstone_os_timer _this,
    touchstone_os_time  interval);

touchstone_os_time
touchstone_os_timerIntervalGet(
    touchstone_os_timer _this);


#if defined (__cplusplus)
}
#endif

#endif /* __POSIX_TOUCHSTONE_OS_ABSTRACTION_H */
