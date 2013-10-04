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

#!/bin/sh
# select compiler version
if [ -n "$SELECTED_C" ]
then
    echo
    echo Configuring C for AIX, inspecting the IBM compiler suite
    C_CNAME=xlC_r
    C_COMPILER=`which $C_CNAME`
    #C_CVERSION=`$C_COMPILER -qversion`
    C_CVERSION=8.0

    C_COPTIONS=''
    CPP_COPTIONS=''
    C_LINKOPTS='-brtl'

    echo Found C compiler $C_COMPILER version $C_CVERSION
fi
if [ -n "$SELECTED_CPP" ]
then
    echo
    echo Configuring C++ for AIX, inspecting the IBM compiler suite
    CPP_CNAME=xlC_r
    CPP_COMPILER=`which $CPP_CNAME`
    #CPP_CVERSION=`$CPP_COMPILER -qversion`
    CPP_CVERSION=8.0

    CPP_COPTIONS=''

    echo Found C++ compiler $CPP_COMPILER version $CPP_CVERSION
fi
if [ -n "$SELECTED_JAVA" ]
then
    echo
    echo Configuring Java for AIX, inspecting the Sun JDK
    JAVA_CNAME=javac
    JAVA_COMPILER=`which $JAVA_CNAME`
    #query compiler version
    JAVA_CVERSION=`$JAVA_COMPILER -version 2>&1 | head -1`
    #strip off first part
    JAVA_CVERSION=${JAVA_CVERSION#javac }
    echo Found java compiler $JAVA_COMPILER version $JAVA_CVERSION
fi
