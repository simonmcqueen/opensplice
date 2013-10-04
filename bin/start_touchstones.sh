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
if [ "$#" != "4" ]
then
  echo "Usage: `basename $0` <language> <no of touchstones> <group id> <start id>"
  echo "<language>		choices are: c / cpp / java"
  echo "<no of touchstones>	number of touchstones to be created"
  echo "<group id>		group id for this collection of touchstones"
  echo "<start id>		starting id for touchstone instances "
  exit 0
fi

LANG=$1
NOFSTONES=$2
GROUPID=$3
STARTID=$4

case "$LANG" in
  "java")
  COMMAND="java touchstone.Main"
  TEST=`echo $CLASSPATH | grep "touchstone.jar"`
  if [ ${#TEST} == 0 ]
  then
    echo "java touchstone not available"
    echo "Please run the release.com.<config> script and ensure that DDSTouchStone has been built with java"
    exit 0
  fi
  ;;

  "cpp")
  COMMAND="touchstone_cpp"
  TEST=`which $COMMAND | grep "no $COMMMAND"`
  if [ ${#TEST} != 0 ]
  then
    echo "cpp touchstone not available"
    echo "Please run the release.com.<config> script and ensure that DDSTouchStone has been built with cpp"
    exit 0
  fi
  ;;

  "c")
  COMMAND="touchstone_c"
  TEST=`which $COMMAND | grep "no $COMMMAND"`
  if [ ${#TEST} != 0 ]
  then
    echo "c touchstone not available"
    echo "Please run the release.com.<config> script and ensure that DDSTouchStone has been built with c"
    exit 0
  fi
  ;;

  *)
  echo "Usage: `basename $0` <language> <no of touchstones> <group id> <start id>"
  echo "<language>		choices are: c / cpp / java"
  echo "<no of touchstones>	number of touchstones to be created"
  echo "<group id>		group id for this collection of touchstones"
  echo "<start id>		starting id for touchstone instances "
  exit 0
  ;;
esac

let "ENDID = $STARTID + $NOFSTONES - 1"

echo "Starting $NOFSTONES $1 touchstones, having ids $STARTID to $ENDID, all in group $GROUPID"

for ((i=$STARTID; i <= $ENDID ; i++))
do
    echo "  starting touchstone using: $COMMAND $i $GROUPID"
    sleep 1
   $COMMAND $i $GROUPID &
done
echo "Completed"
