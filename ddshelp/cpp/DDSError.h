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

#ifndef DDSHELP_DDS_ERROR_H
#define DDSHELP_DDS_ERROR_H

#ifdef _MSC_VER
// Some code uses '*this' in initialisers
#pragma warning(disable:4355)
#endif

#include <ccpp_dds_dcps.h>
#include <iostream>

namespace DDSHelp
{
  class DDSError
  {
  public:
    DDSError(DDS::ReturnCode_t retcode,
	     const char* operation = "NO_OPERATION",
	     const char* identifier = "NO_IDENTIFIER");

    DDSError(const char* description,
	     const char* operation = "NO_OPERATION",
	     const char* identifier = "NO_IDENTIFIER");

    const char*
    description() const
    { return m_description; }

    const char*
    operation() const
    { return m_operation; }

    const char*
    identifier() const
    { return m_identifier; }

  private:
    DDS::String_var m_description;
    DDS::String_var m_operation;
    DDS::String_var m_identifier;
  };
}

std::ostream&
operator<<(std::ostream& s,
	   const DDSHelp::DDSError& e);

#endif // DDSHELP_DDS_ERROR_H
