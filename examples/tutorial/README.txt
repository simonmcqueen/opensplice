#
# DDSTouchStone: a scenario-driven Open Source benchmarking framework
# for evaluating the performance of OMG DDS compliant implementations.
#
# Copyright (C) 2008-2009 PrismTech Ltd.
# ddstouchstone@prismtech.com
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License Version 3 dated 29 June 2007, as published by the
# Free Software Foundation.

# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with DDSTouchStone; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

DDS TouchStone Tutorial Example
--------------------------------------------------------------------------

This example is detailed in DDSTouchStone_User_Guide.pdf


--------------------------------------------------------------------------
How to Run
--------------------------------------------------------------------------
Prerequisites
--------------------------------------------------------------------------

Install OpenSplice DDS HDE V3.3 or above
Install DDS TouchStone V1.1.2 or above

Configure and Build DDS TouchStone


--------------------------------------------------------------------------
Setting the Example Environment
--------------------------------------------------------------------------

Set OpenSplice Environment:

Run the following command from the OpenSplice root directory:

  . release.com

Set TouchStone Environment:

Run the following command from the TouchStone root directory:

  . release.<config_name>.com

where <config_name> is the name of the desired TouchStone configuration


--------------------------------------------------------------------------
Running the Example
--------------------------------------------------------------------------

From <Touchstone_HOME>/examples/tutorial/ run the following commands:

Start OpenSplice:
  ospl start

Start the transmitter / transceiver TouchStone Group:
  start_touchstones.sh <language> <no> 1 <start_id>

Start the receiver / transponder TouchStone Group:
  start_touchstones.sh <language> <no> 2 <start_id>

where:
  <language>    =  desired touchstone language
  <no>          =  number of touchstone instances
  <start_id>    =  starting id for the touchstone instances

Run the example recording:
 start_recording tutorial_recording.dat


--------------------------------------------------------------------------
Viewing the Example Output
--------------------------------------------------------------------------

In a new shell that has the OpenSplice and TouchStone release scripts
correctly sourced (see: Setting the Example Environment) run the following command
to view the results from the scenario.

Start the Watcher:
  watcher

Please Note: The Excelerator can also be used to gather the scenario output.


