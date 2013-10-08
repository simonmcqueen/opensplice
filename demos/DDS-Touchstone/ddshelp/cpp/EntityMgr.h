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

#ifndef DDSHELP_ENTITY_MGR_H
#define DDSHELP_ENTITY_MGR_H

#include <ccpp_dds_dcps.h>
#include <assert.h>

namespace DDSHelp
{
  class EntityMgr
  {
  public:
    DDS::Entity_ptr
    operator->()
    { assert(m_entity); return m_entity; }

    operator DDS::Entity_ptr()
    { assert(m_entity); return m_entity; }

    operator bool()
    { return m_entity != 0; }

  protected:
    EntityMgr()
      : m_entity(0)
    {}

    ~EntityMgr()
    { assert(!m_entity); }

  private:
    EntityMgr(const EntityMgr&); // disabled
    EntityMgr& operator=(const EntityMgr&); // disabled

  protected:
    DDS::Entity_ptr m_entity;	// managed by derived class
  };
}

#endif // DDSHELP_ENTITY_MGR_H
