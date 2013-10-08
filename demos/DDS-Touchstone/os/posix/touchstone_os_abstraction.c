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
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MIN_SCHED_PRIO  (0)
#define MAX_SCHED_PRIO (99)

#define LOG_ERROR(scope, msg) \
    printf("Error in " scope " (file %s, line %d): " msg "\n", __FILE__, __LINE__); \
    if (errno) { \
        perror("Description: "); \
    }
#define LOG_ERROR_1(scope, msg, par) \
    printf("Error in " scope " (file %s, line %d): " msg "\n", __FILE__, __LINE__, par); \
    if (errno) { \
        perror("Description: "); \
    }


/* -------------------- Threading functions ----------------- */

touchstone_os_result
touchstone_os_threadAttrInit(
    touchstone_os_threadAttr *attr)
{
    attr->schedClass = TOUCHSTONE_OS_SCHED_DEFAULT;
    attr->schedPriority = MIN_SCHED_PRIO;
    attr->stackSize = 0;
    return touchstone_os_resultSuccess;
}

touchstone_os_result
touchstone_os_threadWaitExit(
    touchstone_os_threadId threadId,
    void **result_value)
{
    touchstone_os_result rv;
    int result;

    result = pthread_join (threadId, result_value);
    if (result) {
        rv = touchstone_os_resultFail;
    } else {
        rv = touchstone_os_resultSuccess;
    }
    return rv;
}

static touchstone_os_schedClass
touchstone_os_procAttrGetClass (
    void)
{
    touchstone_os_schedClass schedClass;
    int policy;

    policy = sched_getscheduler(getpid());
    switch (policy) {
    case SCHED_FIFO:
    case SCHED_RR:
        schedClass = TOUCHSTONE_OS_SCHED_REALTIME;
        break;
    case SCHED_OTHER:
        schedClass = TOUCHSTONE_OS_SCHED_TIMESHARE;
        break;
    case -1:
        LOG_ERROR_1("touchstone_os_procAttrGetClass", "sched_getscheduler failed with error %d", errno);
        schedClass = TOUCHSTONE_OS_SCHED_DEFAULT;
        break;
    default:
        LOG_ERROR_1("touchstone_os_procAttrGetClass", "sched_getscheduler unexpected return value %d", policy);
        schedClass = TOUCHSTONE_OS_SCHED_DEFAULT;
        break;
    }
    return schedClass;
}

static touchstone_os_priority
touchstone_os_procAttrGetPriority (
    void)
{
    struct sched_param param;

    param.sched_priority = 0;
    if (sched_getparam (getpid(), &param) == -1) {
        LOG_ERROR_1("os_procAttrGetPriority", "sched_getparam failed with error %d", errno);
    }

    return param.sched_priority;
}


touchstone_os_result
touchstone_os_threadCreate (
    touchstone_os_threadId *threadId,
    const char *name,
    const touchstone_os_threadAttr *threadAttr,
    void *(* start_routine)(void *),
    void *arg)
{
    pthread_attr_t attr;
    struct sched_param sched_param;
    touchstone_os_result rv = touchstone_os_resultSuccess;
    touchstone_os_threadAttr tattr;
    touchstone_os_priority prio;
    int result;
    int policy;

    tattr = *threadAttr;
    if (tattr.schedClass == TOUCHSTONE_OS_SCHED_DEFAULT) {
        tattr.schedClass = touchstone_os_procAttrGetClass ();
        tattr.schedPriority = touchstone_os_procAttrGetPriority ();
    }
    if (pthread_attr_init (&attr) != 0 ||
        sched_getparam(getpid(), &sched_param) != 0 ||
        pthread_attr_setscope (&attr, PTHREAD_SCOPE_SYSTEM) != 0 ||
        pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_JOINABLE) != 0 ||
        pthread_attr_setinheritsched (&attr, PTHREAD_EXPLICIT_SCHED) != 0) {
        rv = touchstone_os_resultFail;
    }

    if (rv == touchstone_os_resultSuccess && tattr.stackSize != 0) {
        if (pthread_attr_setstacksize (&attr, tattr.stackSize) != 0) {
            rv = touchstone_os_resultFail;
        }
    }

    if (rv == touchstone_os_resultSuccess) {
        if (tattr.schedClass == TOUCHSTONE_OS_SCHED_REALTIME) {
            if (getuid() == 0 || geteuid() == 0) {
                result = pthread_attr_setschedpolicy (&attr, SCHED_FIFO);
                if (result != 0) {
                    LOG_ERROR_1("touchstone_os_threadCreate", "pthread_attr_setschedpolicy failed with error %d", result);
                }
            } else {
                LOG_ERROR("touchstone_os_threadCreate", "scheduling policy can not be set because of privilege problems");
                result = pthread_attr_setschedpolicy (&attr, SCHED_OTHER);
                if (result != 0) {
                    LOG_ERROR_1("touchstone_os_threadCreate", "pthread_attr_setschedpolicy failed with error %d", result);
                }
            }
        } else {
            result = pthread_attr_setschedpolicy (&attr, SCHED_OTHER);
            if (result != 0) {
                LOG_ERROR_1("touchstone_os_threadCreate", "pthread_attr_setschedpolicy failed with error %d", result);
            }
        }
        pthread_attr_getschedpolicy(&attr, &policy);
        prio = tattr.schedPriority;
        if ((prio < MIN_SCHED_PRIO) || (prio > MAX_SCHED_PRIO)) {
            LOG_ERROR_1("touchstone_os_threadCreate", "scheduling priority %d outside valid range for the policy and reverted to valid value", prio);
            prio = (MIN_SCHED_PRIO + MAX_SCHED_PRIO) / 2;
        }
        sched_param.sched_priority =
               ((MAX_SCHED_PRIO-prio)*sched_get_priority_min(policy) + (prio-MIN_SCHED_PRIO)*sched_get_priority_max(policy)) /
               (MAX_SCHED_PRIO - MIN_SCHED_PRIO);

        if ((tattr.schedPriority < sched_get_priority_min(policy)) ||
            (tattr.schedPriority > sched_get_priority_max(policy))) {
            LOG_ERROR_1("touchstone_os_threadCreate", "scheduling priority %d outside valid range for the policy and reverted to valid value", tattr.schedPriority);
        } else {
            sched_param.sched_priority = tattr.schedPriority;
        }
        /* start the thread */
        result = pthread_attr_setschedparam (&attr, &sched_param);
        if (result != 0) {
            LOG_ERROR_1("touchstone_os_threadCreate", "pthread_attr_setschedparam failed with error %d", result);
        } else {
            result = pthread_create(threadId, &attr, start_routine, arg);
            if (result != 0) {
                LOG_ERROR_1("touchstone_os_threadCreate", "pthread_create failed with error %d", result);
                rv = touchstone_os_resultFail;
            } else {
                rv = touchstone_os_resultSuccess;
            }
        }
    }
    pthread_attr_destroy (&attr);
    return rv;
}

void
touchstone_os_threadExit (
    void *thread_result)
{
    pthread_exit (thread_result);
    return;
}


/* ------------------- Sleep functions --------------------- */

touchstone_os_result
touchstone_os_nanoSleep (
    touchstone_os_time delay)
{
    struct timespec t;
    struct timespec r;
    int result;
    touchstone_os_result rv;

    t.tv_sec = delay.tv_sec;
    t.tv_nsec = delay.tv_nsec;
    result = nanosleep (&t, &r);
    while (result && errno == EINTR) {
        t = r;
        result = nanosleep (&t, &r);
    }
    if (result == 0) {
        rv = touchstone_os_resultSuccess;
    } else {
        rv = touchstone_os_resultFail;
    }
    return rv;
}

/* ----------------- File and dir functions ----------------- */

int
touchstone_os_mkdir(
    const char *path)
{
    int result;

    result = mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);

    return result;
}


/* ------------------------ Timer class --------------------- */

#define SIGSTART SIGRTMIN
#define SIGEND   SIGRTMAX

static void
touchstone_os_timerSigHandler(
    int sig,
    siginfo_t *info,
    void *ignored)
{
    touchstone_os_timer _this = (touchstone_os_timer)(info->si_value.sival_ptr);

    if (_this != NULL) {
        sem_post(&_this->semaphore);
    }
}

static void
touchstone_os_timerSetTimeoutAttributes(
    touchstone_os_timer _this,
    touchstone_os_time  interval)
{
    if (_this != NULL) {
        _this->timeout.it_value.tv_sec = interval.tv_sec;
        _this->timeout.it_value.tv_nsec = interval.tv_nsec;
        _this->timeout.it_interval.tv_sec = interval.tv_sec;
        _this->timeout.it_interval.tv_nsec = interval.tv_nsec;
    }
}


touchstone_os_timer
touchstone_os_timerCreate(
    touchstone_os_time interval)
{
    touchstone_os_timer result;
    int clock_id = CLOCK_REALTIME;
    struct sigevent evp;
    int status;
    /* This static var won't work for single-memoryspace
       OS-es like VXWorks. But for now, they are quite handy! */
    static int sigNoToUse = -1;

    result = (touchstone_os_timer)malloc(sizeof(*result));
    if (result != NULL) {
        result->kind = TK_UNKNOWN; /* Fill in after success only */
        result->interval_sec = interval.tv_sec;
        result->interval_nsec = interval.tv_nsec;
        /* Can not use SIGRTMIN as static initializer because it is
           not a constant. Doing one-time initialization now */
        if (sigNoToUse == -1) {
            sigNoToUse = SIGSTART;
        }
        if (sigNoToUse <= SIGEND) {
            evp.sigev_signo = sigNoToUse;
            evp.sigev_notify = SIGEV_SIGNAL;
            evp.sigev_value.sival_ptr = result;
            status = timer_create(clock_id, &evp, &result->timerid);
            if (status == 0) {
                touchstone_os_timerSetTimeoutAttributes(
                    result, interval);
                result->signo = sigNoToUse++;
                result->kind = TK_POSIXTIMER;
                status = sem_init(&result->semaphore, 0, 0);
                if (status != 0) {
                    LOG_ERROR("touchstone_os_timerCreate", "sem_init failed");
                }
            }
        } else {
            LOG_ERROR("touchstone_os_timerCreate", "sem_init failed");
        }
        if (result->kind == TK_UNKNOWN) {
            /* Fallback to nanosleep mechanism in case the posix
               timer did not work out */
            result->kind = TK_NANOSLEEP;
        }
    }
    return result;
}

void
touchstone_os_timerDestroy(
    touchstone_os_timer _this)
{
    int status;

    if (_this != NULL) {
        if (_this->kind == TK_POSIXTIMER) {
            sem_post(&_this->semaphore);
            status = sem_destroy(&_this->semaphore);
            if (status != 0) {
                LOG_ERROR("touchstone_os_timerDestroy", "sem_destroy failed");
            }
            status = timer_delete(_this->timerid);
            if (status != 0) {
                LOG_ERROR("touchstone_os_timerDestroy", "timer_delete failed");
            }
        }
        free(_this);
    }
}

#define FLAG_RELATIVE 0

void
touchstone_os_timerStart(
    touchstone_os_timer _this)
{
    int status;
    struct sigaction sa;

    if (_this != NULL) {
        if (_this->kind == TK_POSIXTIMER) {
            sigemptyset(&sa.sa_mask);
            sigaddset(&sa.sa_mask, _this->signo);
            sa.sa_flags = SA_SIGINFO;
            sa.sa_sigaction = touchstone_os_timerSigHandler;
            status = sigaction(_this->signo, &sa, NULL);
            if (status != 0) {
                LOG_ERROR("touchstone_os_timerStart", "sigaction failed");
            }
            status = timer_settime(_this->timerid, FLAG_RELATIVE,
                &_this->timeout, NULL);
            if (status != 0) {
                LOG_ERROR("touchstone_os_timerStart", "timer_settime failed");
            }
        }
    }
}

void
touchstone_os_timerWait(
    touchstone_os_timer _this)
{
    siginfo_t info;
    touchstone_os_time interval;

    if (_this != NULL) {
        if (_this->kind == TK_POSIXTIMER) {
            sem_wait(&_this->semaphore);
        } else if (_this->kind == TK_NANOSLEEP) {
            interval.tv_sec = _this->interval_sec;
            interval.tv_nsec = _this->interval_nsec;
            touchstone_os_nanoSleep(interval);
        }
    }
}


/* Getters/setters */

void
touchstone_os_timerIntervalSet(
    touchstone_os_timer _this,
    touchstone_os_time  interval)
{
    int status;

    if (_this != NULL) {
        _this->interval_sec = interval.tv_sec;
        _this->interval_nsec = interval.tv_nsec;
        touchstone_os_timerSetTimeoutAttributes(
            _this, interval);
        status = timer_settime(_this->timerid, FLAG_RELATIVE,
            &_this->timeout, NULL);
        if (status != 0) {
            LOG_ERROR("touchstone_os_timerStart", "timer_settime failed");
        }
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



