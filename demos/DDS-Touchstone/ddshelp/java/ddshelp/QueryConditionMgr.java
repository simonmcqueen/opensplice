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

// REVISIT - use functions implemented by derived class instead of protected fields?

public class QueryConditionMgr extends ConditionMgr {
    private final ReaderMgr<?> m_reader;
    private QueryCondition m_query_condition;

    public QueryConditionMgr(ReaderMgr<?> reader) {
	m_reader = reader;
    }

    public void finalize() {	// REVISIT - useful?
	destroy();
    }

    public QueryCondition value() {
	return m_query_condition;
    }

    public void create(int sample_states,
		       int view_states,
		       int instance_states,
		       String query_expression,
		       String[] query_parameters) {
	destroy();
	m_query_condition = m_reader.value().create_querycondition(sample_states,
								   view_states,
								   instance_states,
								   query_expression,
								   query_parameters);
	check(m_query_condition,
	      "FAILED",
	      "DataReader::create_querycondition");
	m_condition = m_query_condition;
    }

    public void destroy() {
	if (m_query_condition != null) {
	    int retcode = m_reader.value().delete_readcondition(m_query_condition);
	    check(retcode,
		  "Reader::delete_readcondition");
	    m_condition = null;
	    m_query_condition = null;
	}
    }

    public void check(int retcode,
		      String operation) {
	DDSError.check(retcode,
		       operation,
		       m_reader.topic_description().topic_name());
    }

    public void check(Object object,
		      String description,
		      String operation) {
	DDSError.check(object,
		       description,
		       operation,
		       m_reader.topic_description().topic_name());
    }
}
