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

package ddshelp;

import DDS.*;

public class DDSError extends RuntimeException {
    public final String description;
    public final String operation;
    public final String identifier;

    private final static String DDS_RESULT[] = {
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

    public static void check(Object object,
			     String description,
			     String operation,
			     String identifier) {
	if (object == null) {
	    throw new DDSError(description, operation, identifier);
	}
    }

    public static void check(int retcode,
			     String operation,
			     String identifier) {
	if (retcode != 0) {
	    String description = (retcode > 0 && retcode <= RETCODE_ILLEGAL_OPERATION.value)
		? DDS_RESULT[retcode] : "unrecognized return code";
	    throw new DDSError(description, operation, identifier);
	}
    }

    public DDSError(String theDescription,
		    String theOperation,
		    String theIdentifier) {
	description = theDescription;
	operation = theOperation;
	identifier = theIdentifier;
    }

    public String getMessage() {
	return operation + " failed with " + description + " for " + identifier;
    }
}
