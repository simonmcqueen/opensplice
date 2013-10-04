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

#ifndef DDSHELP_DISPATCHER_H
#define DDSHELP_DISPATCHER_H

#include "ConditionMgr.h"
#include "DDSError.h"
#include <map>

namespace DDSHelp
{
  class ProcessorBase
  {
  public:
    bool
    process_shutdown(DDS::Condition_ptr condition)
    { return false; }
  };

  template <class P>
  class Attachment;

  template <class P>
  class Dispatcher
  {
  public:
    typedef bool (P::*ConditionFunction)(DDS::Condition_ptr);

    explicit
    Dispatcher(P& processor);

    ~Dispatcher();

    void
    run();

    void
    shutdown();

  private:
    typedef std::map<DDS::Condition_ptr, ConditionFunction> ConditionMap;

    Dispatcher(const Dispatcher&); // disabled
    Dispatcher& operator=(const Dispatcher&); // disabled

    void
    attach_condition(DDS::Condition_ptr condition,
		     ConditionFunction function);

    void
    detach_condition(DDS::Condition_ptr condition);

    P& m_processor;
    ConditionMap m_conditions;
    DDS::WaitSet m_waitset;
    GuardConditionMgr m_shutdown_condition;
    Attachment<P> m_shutdown_attachment;

    friend class Attachment<P>;
  };

  template <class P>
  class Attachment
  {
  public:
    Attachment(Dispatcher<P>& dispatcher,
	       ConditionMgr& condition)
      : m_dispatcher(dispatcher),
	m_condition(condition)
    {}

    ~Attachment()
    { detach(); }

    void
    attach(typename Dispatcher<P>::ConditionFunction function)
    { m_dispatcher.attach_condition(m_condition, function); }

    void
    detach()
    { if (m_condition) { m_dispatcher.detach_condition(m_condition); }}

  private:
    Attachment(const Attachment&); // disabled
    Attachment& operator=(const Attachment&); // disabled

    Dispatcher<P>& m_dispatcher;
    ConditionMgr& m_condition;
  };

  template <class P>
  Dispatcher<P>::Dispatcher(P& processor)
    : m_processor(processor),
      m_shutdown_attachment(*this, m_shutdown_condition)
  {
    m_shutdown_attachment.attach(&P::process_shutdown);
  }

  template <class P>
  Dispatcher<P>::~Dispatcher()
  {
    m_shutdown_attachment.detach();
    assert(m_conditions.empty());
  }

  template <class P>
  void
  Dispatcher<P>::run()
  {
    bool proceed = true;
    DDS::ConditionSeq conditions;
    while (proceed)
      {
	DDS::ReturnCode_t retcode = m_waitset.wait(conditions, DDS::DURATION_INFINITE);
	if (retcode != DDS::RETCODE_OK)
	  {
	    throw DDSError(retcode, "Waitset::wait");
	  }

	//      cout << "wait() returned " << conditions.length() << " conditions" << endl;
	for (int i = 0; i < conditions.length(); i++)
	  {
	    DDS::Condition_ptr condition = conditions[i];
	    //	  cout << "condition: " << condition << endl;
	    typename ConditionMap::iterator iter(m_conditions.find(condition));
	    if (iter == m_conditions.end())
	      {
		throw DDSError("UNKNOWN_CONDITION_RETURNED", "Waitset::wait");
	      }
	    ConditionFunction function = (*iter).second;
	    proceed = (m_processor.*function)(condition);
	  }
      }

    m_shutdown_condition->set_trigger_value(false);
  }

  template <class P>
  void
  Dispatcher<P>::shutdown()
  {
    m_shutdown_condition->set_trigger_value(true);
  }

  template <class P>
  void
  Dispatcher<P>::attach_condition(DDS::Condition_ptr condition,
				  ConditionFunction function)
  {
    //  cout << "Dispatcher::attach_condition(" << condition << ")" << endl;

    assert(condition != 0);

    typename ConditionMap::iterator iter(m_conditions.find(condition));
    if (iter != m_conditions.end())
      {
	throw DDSError("CONDITION_ALREADY_ATTACHED", "Dispatcher::attach_condition");
      }
    m_conditions.insert(make_pair(DDS::Condition::_duplicate(condition), function));

    DDS::ReturnCode_t retcode = m_waitset.attach_condition(condition);
    if (retcode != DDS::RETCODE_OK)
      {
	throw DDSError(retcode, "Waitset::attach_condition");
      }
  }

  template <class P>
  void
  Dispatcher<P>::detach_condition(DDS::Condition_ptr condition)
  {
    //  cout << "Dispatcher::detach_condition(" << condition << ")" << endl;

    assert(condition != 0);

    typename ConditionMap::iterator iter(m_conditions.find(condition));
    if (iter != m_conditions.end())
      {
	m_conditions.erase(condition);

	DDS::ReturnCode_t retcode = m_waitset.detach_condition(condition);
	if (retcode != DDS::RETCODE_OK)
	  {
	    throw DDSError(retcode, "Waitset::detach_condition");
	  }

	DDS::release(condition);
      }
  }
}

#endif // DDSHELP_DISPATCHER_H
