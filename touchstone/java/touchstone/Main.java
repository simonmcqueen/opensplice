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

package touchstone;

import DDS.DOMAIN_ID_DEFAULT;

class Main {
    private static Processor sm_processor;

    public static void main(String[] args) {
	// System.out.println("main()");

	String application_name = Main.class.getCanonicalName();

	if (args.length < 1 || args.length > 2) {
	    System.err.println("Usage: " + application_name + " application_id [group_id]");
	    System.exit(1);
	}

	int application_id = Integer.valueOf(args[0]);
	int group_id = (args.length == 2) ? Integer.valueOf(args[1]) : application_id;

	try {
	    Runtime.getRuntime().addShutdownHook(new Thread() {
		    public void run() {
			// System.out.println("shutdown hook called");
			sm_processor.shutdown();
		    }
		});

	    sm_processor = new Processor(application_name,
					 application_id,
					 group_id,
					 DOMAIN_ID_DEFAULT.value);

	    sm_processor.run();
	    // System.out.println("run returned");

	    sm_processor = null;
	}
	catch (Exception exception) {
	    System.err.println("caught Exception: " + exception);
	    exception.printStackTrace();
	    System.exit(1);
	}
    }
}
