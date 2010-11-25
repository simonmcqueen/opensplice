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

/* touchstone includes */
#include "Processor.h"
/* ddshelp includes */
#include "DDSError.h"
/* system includes */
#include <iostream>
#include <signal.h>

using namespace DDS;
using namespace std;

Processor* processor_ptr = 0;

void
shutdown_handler(int signal)
{
  if (processor_ptr)
    {
      processor_ptr->shutdown();
    }
}

int
main(int argc,
     char *argv[])
{
  // cout << "main()" << endl;

#ifdef INTEGRITY
  ULong application_id = 1;
  ULong group_id = 1;
#else

  if (argc < 2 || argc > 3)
    {
      cerr << "Usage: " << argv[0] << " application_id [group_id]" << endl;
      return 1;
    }

  ULong application_id = atoi(argv[1]);
  ULong group_id = (argc == 3) ? atoi(argv[2]) : application_id;

#endif   /* INTEGRITY */

  signal(SIGTERM, shutdown_handler);
  signal(SIGINT, shutdown_handler);

  try
    {
      Processor processor(argv[0],
			  application_id,
			  group_id,
			  0);
      processor_ptr = &processor;

      processor.run();

      processor_ptr = 0;
    }
  catch (const DDSError &error)
    {
      cerr << "caught DDSError: " << error << endl;
      return 1;
    }

  return 0;
}
