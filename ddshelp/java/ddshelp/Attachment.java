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

public class Attachment {
    private final Dispatcher m_dispatcher;
    private final ConditionMgr m_condition;

    public Attachment(Dispatcher dispatcher,
		      ConditionMgr condition) {
	m_dispatcher = dispatcher;
	m_condition = condition;
    }

    public void finalize() {	// REVISIT - useful?
	detach();
    }

    public void attach(Handler handler) {
	m_dispatcher.attach_condition(m_condition.value(),
				      handler);
    }

    public void detach() {
	if (m_condition.value() != null) {
	    m_dispatcher.detach_condition(m_condition.value());
	}
    }
}
