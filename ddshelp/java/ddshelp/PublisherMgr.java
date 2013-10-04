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

public class PublisherMgr {
    private final ParticipantMgr m_participant;
    private String m_partition_name;
    private Publisher m_publisher;

    public PublisherMgr(ParticipantMgr participant) {
	m_participant = participant;
    }

    public void finalize() {	// REVISIT - useful?
	destroy();
    }

    public Publisher value() {
	return m_publisher;
    }

    public ParticipantMgr participant() {
	return m_participant;
    }

    public void create(PublisherQos publisher_qos) {
	destroy();
	m_partition_name = "PARTITION_NAME_NOT_SPECIFIED"; // REVISIT - get from qos?
	m_publisher = m_participant.value().create_publisher(publisher_qos,
							     null,
							     ANY_STATUS.value);
	check(m_publisher,
	      "FAILED",
	      "DomainParticipant::create_publisher");
    }

    public void create() {
	create(PUBLISHER_QOS_DEFAULT.value);
    }

    public void create_w_partition(String partition_name) {
	destroy();
	m_partition_name = partition_name;
	PublisherQosHolder publisher_qos = new PublisherQosHolder();
	m_participant.value().get_default_publisher_qos(publisher_qos);
	publisher_qos.value.partition.name = new String[1];
	publisher_qos.value.partition.name[0] = m_partition_name;
	m_publisher = m_participant.value().create_publisher(publisher_qos.value,
							     null,
							     ANY_STATUS.value);
	check(m_publisher,
	      "FAILED",
	      "DomainParticipant::create_publisher");
    }

    public void	destroy() {
	if (m_publisher != null) {
	    int retcode = m_participant.value().delete_publisher(m_publisher);
	    check(retcode,
		  "DomainParticipant::delete_publisher");
	    m_publisher = null;
	    m_partition_name = null;
	}
    }

    public void check(int retcode,
		      String operation) {
	DDSError.check(retcode,
		       operation,
		       m_partition_name);
    }

    public void check(Object object,
		      String description,
		      String operation) {
	DDSError.check(object,
		       description,
		       operation,
		       m_partition_name);
    }
}
