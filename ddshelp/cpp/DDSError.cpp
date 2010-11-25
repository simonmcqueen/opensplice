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

#include "DDSError.h"

using namespace DDSHelp;
using namespace DDS;
using namespace std;

DDSError::DDSError(DDS::ReturnCode_t retcode,
		   const char* operation,
		   const char* identifier)
{
  static const char* DDS_RESULT[] = {
    "DDS_RETCODE_OK",
    "DDS_RETCODE_ERROR",
    "DDS_RETCODE_UNSUPPORTED",
    "DDS_RETCODE_BAD_PARAMETER",
    "DDS_RETCODE_PRECONDITION_NOT_MET",
    "DDS_RETCODE_OUT_OF_RESOURCES",
    "DDS_RETCODE_NOT_ENABLED",
    "DDS_RETCODE_IMMUTABLE_POLICY",
    "DDS_RETCODE_INCONSISTENT_POLICY",
    "DDS_RETCODE_ALREADY_DELETED",
    "DDS_RETCODE_TIMEOUT",
    "DDS_RETCODE_NODATA",
    "DDS_RETCODE_ILEGAL_OPERATION",
  };

  if (retcode >= 0 && retcode <= RETCODE_ILLEGAL_OPERATION)
    {
      m_description = DDS_RESULT[retcode];
    }
  else
    {
      m_description = "unrecognized return code";
    }

  m_operation = (operation) ? operation : "OPERATION_NOT_SUPPLIED";
  m_identifier = (identifier) ? identifier : "IDENTIFIER_NOT_SUPPLIED";
}

DDSError::DDSError(const char* description,
		   const char* operation,
		   const char* identifier)
{
  m_description = (description) ? description : "DESCRIPTION_NOT_SUPPLIED";
  m_operation = (operation) ? operation : "OPERATION_NOT_SUPPLIED";
  m_identifier = (identifier) ? identifier : "IDENTIFIER_NOT_SUPPLIED";
}

ostream&
operator<<(ostream& s,
	   const DDSError& e)
{
  s << e.operation() << " failed with " << e.description() << " for " << e.identifier();
  return s;
}
