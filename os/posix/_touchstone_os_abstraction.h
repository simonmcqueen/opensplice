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

#ifndef __POSIX_TOUCHSTONE_OS_ABSTRACTION_H
#define __POSIX_TOUCHSTONE_OS_ABSTRACTION_H

#include <pthread.h>

#include <time.h>
#include <signal.h>
#include <semaphore.h>

typedef pthread_t       _touchstone_os_threadId;
typedef time_t          _touchstone_os_timeSec;

typedef enum touchstone_os_timerkind_e {
    TK_UNKNOWN,    /* Unexpected situation */
    TK_POSIXTIMER, /* Posix timer using RT signals */
    TK_NANOSLEEP   /* Fallback when no more timers are available */
} touchstone_os_timerkind;

struct _touchstone_os_timer_s {
    touchstone_os_timerkind kind;
    /* fields needed for using the posix timer */
    timer_t                 timerid;
    struct itimerspec       timeout;
    int                     signo;
    sem_t                   semaphore;
    /* fields needed for doing simple nanosleep */
    _touchstone_os_timeSec  interval_sec;
    int                     interval_nsec;
};

#endif /* __POSIX_TOUCHSTONE_OS_ABSTRACTION_H */
