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

public class ParticipantMgr {
    private final static DomainParticipantFactory sm_factory = DomainParticipantFactory.get_instance();
    private String m_domain_id;
    private DomainParticipant m_participant;

    public ParticipantMgr() {}

    public void finalize() {	// REVISIT - useful?
	destroy();
    }

    public DomainParticipant value() {
	return m_participant;
    }

    public void	create(String domain_id,
		       DomainParticipantQos participant_qos) {
	destroy();
	m_domain_id = domain_id;
	m_participant = sm_factory.create_participant(m_domain_id,
						      participant_qos,
						      null,
						      ANY_STATUS.value);
	check(m_participant,
	      "DOMAIN_NOT_RUNNING",
	      "DomainParticipantFactory::create_participant");
    }

    public void	create(String domain_id) {
	create(domain_id,
	       PARTICIPANT_QOS_DEFAULT.value);
    }

    public void	destroy() {
	if (m_participant != null) {
	    int retcode = sm_factory.delete_participant(m_participant);
	    check(retcode,
		  "DomainParticipantFactory::delete_participant");
	    m_participant = null;
	    m_domain_id = null;
	}
    }

    public void check(int retcode,
		      String operation) {
	DDSError.check(retcode,
		       operation,
		       m_domain_id);
    }

    public void check(Object object,
		      String description,
		      String operation) {
	DDSError.check(object,
		       description,
		       operation,
		       m_domain_id);
    }
}
