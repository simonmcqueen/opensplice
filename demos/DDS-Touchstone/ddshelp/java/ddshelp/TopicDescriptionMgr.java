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

class TopicDescriptionMgr {
    private final ParticipantMgr m_participant;
    protected String m_topic_name;
    protected TopicDescription m_topic_description; // managed by derived class

    protected TopicDescriptionMgr(ParticipantMgr participant) {
	m_participant = participant;
    }

    public TopicDescription value() {
	return m_topic_description;
    }

    public ParticipantMgr participant() {
	return m_participant;
    }

    public String topic_name() {
	return m_topic_name;
    }

    public void check(int retcode,
		      String operation) {
	DDSError.check(retcode,
		       operation,
		       m_topic_name);
    }

    public void check(Object object,
		      String description,
		      String operation) {
	DDSError.check(object,
		       description,
		       operation,
		       m_topic_name);
    }
}
