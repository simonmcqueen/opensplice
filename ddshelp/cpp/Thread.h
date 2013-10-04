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

// -*- mode: c++ -*-

#ifndef DDSHELP_THREAD_H
#define DDSHELP_THREAD_H

/* Use OS abstraction for threading functionality */
#include "touchstone_os_abstraction.h"

#include <limits.h>

namespace DDSHelp
{
  template <class P>
  class Thread
  {
  public:
    enum Policy { DEFAULT, REALTIME, TIMESHARE };

    typedef void (P::*ThreadFunction)();

    explicit
    Thread(P& processor);

    ~Thread();

    void
    create(ThreadFunction function,
	   Policy policy = DEFAULT,
	   int priority = INT_MIN);

    void
    join();

  private:
    Thread(const Thread&); // disabled
    Thread& operator=(const Thread&); // disabled

    static void*
    start(void* arg);

    P& m_processor;
    bool m_active;
    ThreadFunction m_function;
    touchstone_os_threadId m_threadId;
  };

  template <class P>
  Thread<P>::Thread(P& processor)
    : m_processor(processor),
      m_active(false)
  {
  }

  template <class P>
  Thread<P>::~Thread()
  {
    if (m_active)
      {
	// cancel
	// join
      }
  }

  template <class P>
  void
  Thread<P>::create(ThreadFunction function,
		    Policy policy,
		    int priority)
  {
    assert(!m_active);

    m_function = function;

    touchstone_os_threadAttr attr;
    touchstone_os_result result;

    result = touchstone_os_threadAttrInit(&attr);
    if (result != touchstone_os_resultSuccess)
      {
	throw "touchstone_os_threadAttrInit failed";	// REVISIT
      }

    // REVISIT - set policy & priority here

    result = touchstone_os_threadCreate(
                            &m_threadId,
                            "",
			    &attr,
			    &start,
			    this);
    if (result != touchstone_os_resultSuccess)
      {
	throw "touchstone_os_threadCreate failed";	// REVISIT
      }

    /*
    touchstone_os_threadAttrDestroy(&attr);
    if (result != touchstone_os_resultSuccess)
      {
	throw "touchstone_os_threadAttrDestroy failed"; // REVISIT
      }
    */

    m_active = true;
  }

  template <class P>
  void
  Thread<P>::join()
  {
    assert(m_active);
  
    touchstone_os_result result;

    result = touchstone_os_threadWaitExit(m_threadId, 0);
    if (result != touchstone_os_resultSuccess)
      {
	throw "touchstone_os_threadWaitExit failed"; // REVISIT
      }

    m_function = 0;
    m_active = false;
  }

  template <class P>
  void*
  Thread<P>::start(void* arg)
  {
    Thread<P>* thread = reinterpret_cast<Thread<P>*>(arg);

    assert(thread->m_function);
    ThreadFunction function = thread->m_function;
    (thread->m_processor.*function)();
    return 0;
  }
}

#endif // DDSHELP_THREAD_H
