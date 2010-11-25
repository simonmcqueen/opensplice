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

DDS TouchStone Skeleton Scenario Example
--------------------------------------------------------------------------

This example file contains a list of recorder commands that can serve as a
basis for manually creating DDS Touchstone scenarios. It contains a dummy
version of all different commands known to the touchstone application. By
copying, pasting and editing, touchstone entities can be added or adjusted
for the scenario. It can then be saved as <YourScenarioName>.dat.

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

The exact parameters needed for running your scenario created from the
skeleton scenario depends on the changes made. Please inspect the README.txt
in the other examples included with DDS Touchstone to get an idea of how to
start your own example.

--------------------------------------------------------------------------
Viewing Example Output (Both Single and Multi Mode)
--------------------------------------------------------------------------

In a new shell that has the OpenSplice and TouchStone release scripts
correctly sourced (see: To Set Example Environment) run the following command
to view the results from the scenario.

Start the Watcher:
  watcher

Please Note: The Excelerator can also be used to gather the scenario output.
