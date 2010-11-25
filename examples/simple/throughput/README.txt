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

DDS TouchStone Simple Throughput Example
--------------------------------------------------------------------------

The following DDS TouchStone scenario is designed to produce throughput data for the
following possible communication configurations:

  1 transmitter      to      1 receiver
  1 transmitter      to      many receivers
  many transmitters  to      1 receiver
  many transmitters  to      many receivers

These configurations are selectable by controlling the number of TouchStone
instances deployed. 

This example uses TouchStone grouping to allow scalable expansion of the nodes
and TouchStone instances used.

  Transmitters use group id   2001
  Receivers use group id      1001

Therefore to expand or reduce the node configuration for this scenario simply 
increase or decrease the number of TouchStones instances in the groups.


--------------------------------------------------------------------------
Transmitter Default Settings
--------------------------------------------------------------------------

  group_id=2001
  transmitter_id=1
  partition_id=1
  topic_id=1
  topic_kind=BEST_EFFORT
  scheduling_class=TIMESHARING
  thread_priority=0
  message_size=512
  message_per_burst=55
  burst_period=50


--------------------------------------------------------------------------
Receiver Default Settings
--------------------------------------------------------------------------

  group_id=1001
  receiver_id=1
  partition_id=1
  topic_id=1
  topic_kind=BEST_EFFORT
  scheduling_class=TIMESHARING
  thread_priority=0
  report_period=10000
  polling_period-10


--------------------------------------------------------------------------
Example Output
--------------------------------------------------------------------------

Each receiver will gather the messages sent by the Transmitter and release
a report every 10 seconds (see report_period). These reports are viewable
in the watcher or excelerator.

Sample Report:

Received receiver report from (31,1,1)
    Read       = 8066662 bytes/sec
    Read       = 984 samples/sec


where (touchstone id, receiver id, partition id)
samples/sec == messages/sec


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
Running the Example (Single Node Mode)
--------------------------------------------------------------------------

From <Touchstone_HOME>/examples/simple/throughput/ run the following commands:

Start OpenSplice:
  ospl start

Start the receiver TouchStone Group:
  start_touchstones.sh <language> <no> 1001 <start_id>

Start the transmitter TouchStone Group:
  start_touchstones.sh <language> <no> 2001 <start_id>

where:
  <language>    =  desired touchstone language
  <no>          =  number of touchstone instances
  <start_id>    =  starting id for the touchstone instances

Run the example recording:
 start_recording simple_throughput_recording.dat


--------------------------------------------------------------------------
Running the Example (Multiple Node Mode)
--------------------------------------------------------------------------

Run these commands on the nodes that are to host receivers, from the
<Touchstone_HOME>/examples/simple/throughput/ directory:

Start OpenSplice
  ospl start

Start the receiver TouchStone Group:
  start_touchstones.sh <language> <no> 1001 <start_id>

where:
  <language>    =  desired touchstone language
  <no>          =  number of touchstone instances
  <start_id>    =  starting id for the touchstone instances

--------------------------------------------------------------------------

Run these commands on the nodes that are to host transmitters, from the
<Touchstone_HOME>/examples/simple/throughput/ directory:

Start OpenSplice
  ospl start

Start the transmitter TouchStone Group:
  start_touchstones.sh <language> <no> 2001 <start_id>

where:
  <language>    =  desired touchstone language
  <no>          =  number of touchstone instances
  <start_id>    =  starting id for the touchstone instances

--------------------------------------------------------------------------

To start the example run the following command once from any node:
  start_recording simple_throughput_recording.dat


--------------------------------------------------------------------------
Viewing Example Output (Both Single and Multi Mode)
--------------------------------------------------------------------------

In a new shell that has the OpenSplice and TouchStone release scripts
correctly sourced (see: To Set Example Environment) run the following command
to view the results from the scenario.

Start the Watcher:
  watcher

Please Note: The Excelerator can also be used to gather the scenario output.


