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

# Check for proper no. of command line args.
if [ "$#" = "0" ]
then
  echo "Usage: `basename $0` <Recording Filename> [Recorder ID]"
  exit 0
fi

COMMAND="recorder --autoplay -v" 
RFILE=$1
ID=$2

if [ "$ID" = "" ]
then
 ID="99"
fi

if [ ! -f $RFILE ]
then
  echo "Recording Replay Cancelled"
  echo "Can not locate file: $RFILE"
  exit 0
fi

echo "Starting Recording"
echo "$COMMAND $ID $RFILE"
$COMMAND $ID $RFILE 
