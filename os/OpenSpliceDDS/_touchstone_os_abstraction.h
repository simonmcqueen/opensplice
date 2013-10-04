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

#ifndef __OPENSPLICE_TOUCHSTONE_OS_ABSTRACTION_H
#define __OPENSPLICE_TOUCHSTONE_OS_ABSTRACTION_H

#include "os.h"

typedef os_threadId       _touchstone_os_threadId;
typedef os_timeSec         _touchstone_os_timeSec;

struct _touchstone_os_timer_s {
    _touchstone_os_timeSec interval_sec;
    int interval_nsec;
};

#endif /* __OPENSPLICE_TOUCHSTONE_OS_ABSTRACTION_H */
