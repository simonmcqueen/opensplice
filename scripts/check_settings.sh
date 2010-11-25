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

# Copyright 2008 PrismTech

if [ ! -n "$4" ]
then
  echo "Usage: `basename $0` <architecture> <kernel_name> <kernel_version> <vendor>"
fi


TARGET_MACHINE=$1
TARGET_KERNEL_NAME=$2
TARGET_KERNEL_RELEASE=$3
TARGET_VENDOR=$4


# First test whether this is the correct target
# Determine machine properties
MACHINE=`uname -m`
KERNEL_NAME=`uname -s`
KERNEL_RELEASE=`uname -r`

MISMATCH=

if [ "$MACHINE" != "$TARGET_MACHINE" ]
then
    echo "Current architecture is $MACHINE, configuration is created for $TARGET_MACHINE"
    MISMATCH=yes
fi

if [ "$KERNEL_NAME" != "$TARGET_KERNEL_NAME" ]
then
    echo "Current operating system is $KERNEL_NAME, configuration is created for $TARGET_KERNEL_NAME"
    MISMATCH=yes
fi

if [ "$KERNEL_RELEASE" != "$TARGET_KERNEL_RELEASE" ]
then
    echo "Current OS-version is $KERNEL_RELEASE, configuration is created for $TARGET_KERNEL_RELEASE"
    MISMATCH=yes
fi

if [ "$TARGET_VENDOR" = "OpenSpliceDDS" ]
then
    TEST=`which spliced 2>/dev/null`
    if [ "_$TEST" = "_" ]
    then
        echo "DDS infrastructure not properly configured of initialized"
        echo "Configuration is created for $TARGET_VENDOR but can not find spliced exectuable"
        MISMATCH=yes
    fi
else
    MISMATCH=yes
    echo "Configuration done for unknown DDS vendor $TARGET_VENDOR"
fi

if [ "$MISMATCH" = "yes" ]
then
   echo "Current system properties do not match configured properties."
   printf "Do you want to continue [yes]? "
   read ANSWER
   if [ -z "$ANSWER" ]; then ANSWER=yes; fi
   if [ "_$ANSWER" != "_yes" ]; then exit 64; fi
else
    echo "Current system matches settings architecture=$TARGET_MACHINE kernel_name=$TARGET_KERNEL_NAME kernel_version=$TARGET_KERNEL_RELEASE vendor=$TARGET_VENDOR"
fi

