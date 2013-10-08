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

public class TopicMgr extends TopicDescriptionMgr {
    private final TypeSupport m_type_support;
    private Topic m_topic;

    public TopicMgr(ParticipantMgr participant,
		    TypeSupport type_support) {
	super(participant);
	m_type_support = type_support;
    }

    public void finalize() {	// REVISIT - useful?
	destroy();
    }

    public Topic value() {
	return m_topic;
    }

    public void create(String topic_name,
		       TopicQos topic_qos) {
	destroy();
	m_topic_name = topic_name;
	String type_name = m_type_support.get_type_name();
	int retcode = m_type_support.register_type(participant().value(),
						   type_name);
	check(retcode,
	      "TypeSupport::register_type");
	m_topic = participant().value().create_topic(m_topic_name,
						     type_name,
						     topic_qos,
						     null,
						     ANY_STATUS.value);
	check(m_topic,
	      "FAILED",
	      "DomainParticipant::create_topic");
	m_topic_description = m_topic; // REVISIT - use method?
    }

    public void create(String topic_name) {
	create(topic_name,
	       TOPIC_QOS_DEFAULT.value);
    }

    public void destroy() {
	if (m_topic != null) {
	    int retcode = participant().value().delete_topic(m_topic);
	    check(retcode,
		  "DomainParticipant::delete_topic");
	    m_topic = null;
	    m_topic_description = null;
	    m_topic_name = null;
	}
    }
}
