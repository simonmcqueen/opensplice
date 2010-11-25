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

public class FilteredTopicMgr extends TopicDescriptionMgr {
    private final TopicMgr m_related_topic;
    private ContentFilteredTopic m_filtered_topic;

    public FilteredTopicMgr(TopicMgr related_topic) {
	super(related_topic.participant());
	m_related_topic = related_topic;
    }

    public void finalize() {	// REVISIT - useful?
	destroy();
    }

    public ContentFilteredTopic value() {
	return m_filtered_topic;
    }

    public void create(String topic_name,
		       String filter_expression,
		       String[] expression_parameters) {
	destroy();
	m_topic_name = topic_name;
	m_filtered_topic = participant().value().create_contentfilteredtopic(m_topic_name,
									     m_related_topic.value(),
									     filter_expression,
									     expression_parameters);
	check(m_filtered_topic,
	      "FAILED",
	      "DomainParticipant::create_contentfilteredtopic");
	m_topic_description = m_filtered_topic; // REVISIT - use method?
    }

    public void destroy() {
	if (m_filtered_topic != null) {
	    int retcode = participant().value().delete_contentfilteredtopic(m_filtered_topic);
	    check(retcode,
		  "DomainParticipant::delete_contectfilteredtopic");
	    m_filtered_topic = null;
	    m_topic_description = null;
	    m_topic_name = null;
	}
    }
}
