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
import java.util.*;

public class Dispatcher {
    private final HashMap<Condition, Handler> m_conditions = new HashMap<Condition, Handler>();
    private final WaitSet m_waitset = new WaitSet();
    private final GuardConditionMgr m_shutdown_condition = new GuardConditionMgr();
    private final Attachment m_shutdown_attachment = new Attachment(this,
								    m_shutdown_condition);

    public Dispatcher() {}

    public void finalize() {
	// REVISIT - detach any remaining conditions?
    }

    public void run() {
	// System.out.println("Dispatcher::run()");
	try {
	    m_shutdown_attachment.attach(new Handler() {
		    public boolean handle_condition(Condition condition) {
			return false;
		    }
		});
	    boolean proceed = true;
	    ConditionSeqHolder conditions = new ConditionSeqHolder();
	    while (proceed) {
		int retcode = m_waitset._wait(conditions,
					      DURATION_INFINITE.value);
		DDSError.check(retcode,
			       "Waitset::wait",
			       null);
		// System.out.println("wait() returned " + conditions.value.length + " conditions");
		for (int i = 0; i < conditions.value.length; i++) {
		    Condition condition = conditions.value[i];
		    assert(condition != null);
		    // System.out.println("condition: " + condition);
		    Handler handler = m_conditions.get(condition);
		    if (handler == null) {
			throw new DDSError("UNKNOWN_CONDITION_RETURNED",
					   "Waitset::wait",
					   null);
		    }
		    try {
			proceed = handler.handle_condition(condition);
		    }
		    catch (Exception e) {
			System.err.println("caught exception: " + e);
			e.printStackTrace();
			throw new DDSError("DISPATCH FAILED",
					   "Handler::handle_condition",
					   handler.toString());
		    }
		}
	    }
	}
	finally {
	    m_shutdown_attachment.detach();
	    m_shutdown_condition.value().set_trigger_value(false);
	}
    }

    public void shutdown() {
	// System.out.println("Dispatcher::shutdown()");
	m_shutdown_condition.value().set_trigger_value(true);
    }

    void attach_condition(Condition condition,
			  Handler handler) {
	// System.out.println("Dispatcher::attach_condition(" + condition + ")");
	assert(condition != null);
	if (m_conditions.containsKey(condition)) {
	    throw new DDSError("CONDITION_ALREADY_ATTACHED",
			       "Dispatcher::attach_condition",
			       handler.toString());
	}
	m_conditions.put(condition,
			 handler);
	int retcode = m_waitset.attach_condition(condition);
	DDSError.check(retcode,
		       "Waitset::attach_condition",
		       handler.toString());
    }

    void detach_condition(Condition condition) {
	// System.out.println("Dispatcher::detach_condition(" + condition + ")");
	assert(condition != null);
	if (m_conditions.containsKey(condition)) {
	    m_conditions.remove(condition);
	    int retcode = m_waitset.detach_condition(condition);
	    DDSError.check(retcode,
			   "Waitset::detach_condition",
			   null);
	}
    }
}
