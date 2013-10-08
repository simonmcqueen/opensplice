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

#ifndef DDSHELP_CONDITION_MGR_H
#define DDSHELP_CONDITION_MGR_H

#include <ccpp_dds_dcps.h>
#include <assert.h>

namespace DDSHelp
{
  class EntityMgr;
  class ReaderMgrBase;

  class ConditionMgr
  {
  public:
    DDS::Condition_ptr
    operator->()
    { assert(m_condition); return m_condition; }

    operator DDS::Condition_ptr()
    { assert(m_condition); return m_condition; }

    operator bool()
    { return m_condition != 0; }

  protected:
    ConditionMgr()
      : m_condition(0)
    {}

    ~ConditionMgr()
    { assert(!m_condition); }

  private:
    ConditionMgr(const ConditionMgr&); // disabled
    ConditionMgr& operator=(const ConditionMgr&); // disabled

  protected:
    DDS::Condition_ptr m_condition;	// managed by derived class
  };

  class GuardConditionMgr
    : public ConditionMgr
  {
  public:
    GuardConditionMgr()
    { m_condition = &m_guard_condition; }

    ~GuardConditionMgr()
    { m_condition = 0; }

    DDS::GuardCondition_ptr
    operator->()
    { return &m_guard_condition; }

    operator DDS::GuardCondition_ptr()
    { return &m_guard_condition; }

    operator bool()
    { return true; }

  private:
    DDS::GuardCondition m_guard_condition;
  };

  class StatusConditionMgr
    : public ConditionMgr
  {
  public:
    StatusConditionMgr(EntityMgr& entity)
      : m_entity(entity)
    {}

    ~StatusConditionMgr()
    { release(); }

    DDS::StatusCondition_ptr
    operator->()
    { assert(m_status_condition); return m_status_condition; }

    operator DDS::StatusCondition_ptr()
    { assert(m_status_condition); return m_status_condition; }

    operator bool()
    { return m_status_condition.in() != 0; }

    void
    get();

    void
    release();

  private:
    EntityMgr& m_entity;
    DDS::StatusCondition_var m_status_condition;
  };

  class ReadConditionMgr
    : public ConditionMgr
  {
  public:
    ReadConditionMgr(ReaderMgrBase& reader)
      : m_reader(reader)
    {}

    ~ReadConditionMgr()
    { destroy(); }

    DDS::ReadCondition_ptr
    operator->()
    { assert(m_read_condition); return m_read_condition; }

    operator DDS::ReadCondition_ptr()
    { assert(m_read_condition); return m_read_condition; }

    operator bool()
    { return m_read_condition.in() != 0; }

    void
    create(DDS::SampleStateMask sample_states,
	   DDS::ViewStateMask view_states,
	   DDS::InstanceStateMask instance_states);

    void
    destroy();

  private:
    ReaderMgrBase& m_reader;
    DDS::ReadCondition_var m_read_condition;
  };

  class QueryConditionMgr
    : public ConditionMgr
  {
  public:
    QueryConditionMgr(ReaderMgrBase& reader)
      : m_reader(reader)
    {}

    ~QueryConditionMgr()
    { destroy(); }

    DDS::QueryCondition_ptr
    operator->()
    { assert(m_query_condition); return m_query_condition; }

    operator DDS::QueryCondition_ptr()
    { assert(m_query_condition); return m_query_condition; }

    operator bool()
    { return m_query_condition.in() != 0; }

    void
    create(DDS::SampleStateMask sample_states,
	   DDS::ViewStateMask view_states,
	   DDS::InstanceStateMask instance_states,
	   const char* query_expression,
	   const DDS::StringSeq& query_parameters);

    void
    destroy();

  private:
    ReaderMgrBase& m_reader;
    DDS::QueryCondition_var m_query_condition;
  };
}

#endif // DDSHELP_CONDITION_MGR_H
