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

DDS TouchStone Simple Latency Example
--------------------------------------------------------------------------

The following DDS TouchStone scenario is designed to produce latency data for the
following possible communication configurations:

  1 transceiver      to      1 transponder
  1 transceiver      to      many transponders
  many transceivers  to      1 transponder
  many transceivers  to      many transponders

These configurations are selectable by controlling the number of TouchStone
instances deployed. 

This example uses TouchStone grouping to allow scalable expansion of the nodes
and TouchStone instances used.

  Transceivers use group id   2001
  Transponders use group id   1001

Therefore to expand or reduce the node configuration for this scenario simply 
increase or decrease the number of TouchStones instances in the groups.


--------------------------------------------------------------------------
Transceiver Default Settings
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
Transponder Default Settings
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

Each Transceiver will gather the message replies from the Transponder and release
a report every 10 seconds (see report_period). These reports are viewable
in the watcher or excelerator.

Sample Report:

Received Transceiver report from (2,1,1)
    Send latency: 
    100.0% : cnt= 199, min= 67, avg= 98, max= 235, dev= 39.66
    99.9% : cnt= 198, min= 67, avg= 97, max= 224, dev= 38.54
    99.0% : cnt= 197, min= 67, avg= 97, max= 221, dev= 37.56
    90.0% : cnt= 179, min= 67, avg= 87, max= 159, dev= 23.54
    Send Source latency: 
    100.0% : cnt= 199, min= 1, avg= 2, max= 6, dev= 1.18
    99.9% : cnt= 198, min= 1, avg= 2, max= 6, dev= 1.16
    99.0% : cnt= 197, min= 1, avg= 2, max= 6, dev= 1.14
    90.0% : cnt= 179, min= 1, avg= 2, max= 5, dev= 0.75
    Send Delivery latency: 
    100.0% : cnt= 199, min= 21, avg= 36, max= 103, dev= 20.57
    99.9% : cnt= 198, min= 21, avg= 36, max= 102, dev= 20.06
    99.0% : cnt= 197, min= 21, avg= 36, max= 102, dev= 19.55
    90.0% : cnt= 179, min= 21, avg= 31, max= 68, dev= 11.93
    Send Arrival latency: 
    100.0% : cnt= 199, min= 44, avg= 59, max= 131, dev= 18.19
    99.9% : cnt= 198, min= 44, avg= 59, max= 118, dev= 17.50
    99.0% : cnt= 197, min= 44, avg= 58, max= 114, dev= 17.03
    90.0% : cnt= 179, min= 44, avg= 54, max= 88, dev= 11.09
    Echo latency: 
    100.0% : cnt= 199, min= 74, avg= 81, max= 138, dev= 8.90
    99.9% : cnt= 198, min= 74, avg= 81, max= 126, dev= 7.96
    99.0% : cnt= 197, min= 74, avg= 81, max= 110, dev= 7.30
    90.0% : cnt= 179, min= 74, avg= 79, max= 93, dev= 4.94
    Echo Source latency: 
    100.0% : cnt= 199, min= 1, avg= 1, max= 2, dev= 0.35
    99.9% : cnt= 198, min= 1, avg= 1, max= 2, dev= 0.34
    99.0% : cnt= 197, min= 1, avg= 1, max= 2, dev= 0.34
    90.0% : cnt= 179, min= 1, avg= 1, max= 2, dev= 0.21
    Echo Delivery latency: 
    100.0% : cnt= 199, min= 17, avg= 20, max= 39, dev= 2.61
    99.9% : cnt= 198, min= 17, avg= 20, max= 30, dev= 2.23
    99.0% : cnt= 197, min= 17, avg= 20, max= 26, dev= 2.12
    90.0% : cnt= 179, min= 17, avg= 19, max= 24, dev= 1.51
    Echo Arrival latency: 
    100.0% : cnt= 199, min= 55, avg= 60, max= 111, dev= 6.94
    99.9% : cnt= 198, min= 55, avg= 60, max= 87, dev= 5.94
    99.0% : cnt= 197, min= 55, avg= 60, max= 86, dev= 5.63
    90.0% : cnt= 179, min= 55, avg= 58, max= 69, dev= 3.58
    Trip latency: 
    100.0% : cnt= 199, min= 143, avg= 180, max= 374, dev= 47.21
    99.9% : cnt= 198, min= 143, avg= 180, max= 344, dev= 45.28
    99.0% : cnt= 197, min= 143, avg= 179, max= 331, dev= 43.85
    90.0% : cnt= 179, min= 143, avg= 168, max= 258, dev= 27.76
    Inter arrival time: 
    100.0% : cnt= 198, min= 49981, avg= 50069, max= 50277, dev= 45.12
    99.9% : cnt= 197, min= 49981, avg= 50068, max= 50211, dev= 42.72
    99.0% : cnt= 196, min= 49981, avg= 50067, max= 50193, dev= 41.58
    90.0% : cnt= 178, min= 49981, avg= 50057, max= 50142, dev= 28.98


where (touchstone id, transceiver id, partition id)
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

From <Touchstone_HOME>/examples/simple/latency/ run the following commands:

Start OpenSplice:
  ospl start

Start the transponder TouchStone Group:
  start_touchstones.sh <language> <no> 1001 <start_id>

Start the transceiver TouchStone Group:
  start_touchstones.sh <language> <no> 2001 <start_id>

where:
  <language>    =  desired touchstone language
  <no>          =  number of touchstone instances
  <start_id>    =  starting id for the touchstone instances

Run the example recording:
 start_recording simple_latency_recording.dat


--------------------------------------------------------------------------
Running the Example (Multiple Node Mode)
--------------------------------------------------------------------------

Run these commands on the nodes that are to host transponders, from the
<Touchstone_HOME>/examples/simple/latency/ directory:

Start OpenSplice
  ospl start

Start the transponder TouchStone Group:
  start_touchstones.sh <language> <no> 1001 <start_id>

where:
  <language>    =  desired touchstone language
  <no>          =  number of touchstone instances
  <start_id>    =  starting id for the touchstone instances

--------------------------------------------------------------------------

Run these commands on the nodes that are to host transceivers, from the
<Touchstone_HOME>/examples/simple/latency/ directory:

Start OpenSplice
  ospl start

Start the transceiver TouchStone Group:
  start_touchstones.sh <language> <no> 2001 <start_id>

where:
  <language>    =  desired touchstone language
  <no>          =  number of touchstone instances
  <start_id>    =  starting id for the touchstone instances

--------------------------------------------------------------------------

To start the example run the following command once from any node:
  start_recording simple_latency_recording.dat


--------------------------------------------------------------------------
Viewing Example Output (Both Single and Multi Mode)
--------------------------------------------------------------------------

In a new shell that has the OpenSplice and TouchStone release scripts
correctly sourced (see: To Set Example Environment) run the following command
to view the results from the scenario.

Start the Watcher:
  watcher

Please Note: The Excelerator can also be used to gather the scenario output.


