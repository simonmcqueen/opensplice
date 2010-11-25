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
    echo Configuring C for SunOS, inspecting the GNU compiler suite first
    C_CNAME=gcc
    C_COMPILER=`type $C_CNAME`
    C_COMPILER=`echo $C_COMPILER | sed "s/$C_CNAME not found//g"`
    if [ -z "$C_COMPILER" ]
    then
        echo GNU compiler not found, looking for SunStudio
        C_CNAME=cc
        C_COMPILER=`type $C_CNAME`
        C_COMPILER=`echo $C_COMPILER | sed "s/$C_CNAME not found//g"`
        if [ -z "$C_COMPILER" ]
        then
            echo SunStudio compiler not found, looking for Integrity
            C_CNAME=ccintppc
            C_COMPILER=`type $C_CNAME`
            C_COMPILER=`echo $C_COMPILER | sed "s/$C_CNAME not found//g"`
            if [ -z "$C_COMPILER" ]
            then
                echo Integrity compiler not found
                echo No C compiler found, exiting...
                exit
            else
                C_COMPILER=`which $C_CNAME`
                C_CVERSION=5
                # don't know how to get a version string from integrity
                C_COPTIONS="-bsp mvme5100-750 -os_dir ${I_INSTALL_DIR} -fvolatile -O2 -fsigned-char -DINTEGRITY -I${I_INSTALL_DIR}"
                # integrity ignores CPATH
                for i in `echo $CPATH | tr ':' ' '`
                    do C_COPTIONS="${C_COPTIONS} -I${i}"
                done
                C_LINKOPTS="-bsp mvme5100-750 -os_dir ${I_INSTALL_DIR}"
                C_SYSLIBS="socket shm_client posix_sys_client ivfs"
            fi
        else
            C_COMPILER=`which $C_CNAME`
            C_CVERSION=`version`
            C_CVERSION=`echo $C_CVERSION | sed "s/.*Sun Studio \([0-9]*\).*/SunStudio\1/"`
        fi
    else
        C_COMPILER=`which $C_CNAME`
        C_CVERSION=`$C_COMPILER -dumpversion`
        C_SYSLIBS=m
    fi
    echo Found C compiler $C_COMPILER version $C_CVERSION
fi
if [ -n "$SELECTED_CPP" ]
then
    echo
    echo Configuring C++ for SunOS, inspecting the GNU compiler suite first
    CPP_CNAME=g++
    CPP_COMPILER=`type $CPP_CNAME`
    CPP_COMPILER=`echo $CPP_COMPILER | sed "s/$CPP_CNAME not found//g"`
    if [ -z "$CPP_COMPILER" ]
    then
        echo GNU compiler not found, looking for SunStudio
        CPP_CNAME=cc
        CPP_COMPILER=`type $CPP_CNAME`
        CPP_COMPILER=`echo $CPP_COMPILER | sed "s/$CPP_CNAME not found//g"`
        if [ -z "$CPP_COMPILER" ]
        then
            echo SunStudio compiler not found, looking for Integrity
            CPP_CNAME=cxintppc
            CPP_COMPILER=`type $CPP_CNAME`
            CPP_COMPILER=`echo $CPP_COMPILER | sed "s/$CPP_CNAME not found//g"`
            if [ -z "$CPP_COMPILER" ]
            then
                echo Integrity compiler not found
                echo No C++ compiler found, exiting...
                exit
            else
                CPP_COMPILER=`which $CPP_CNAME`
                CPP_CVERSION=5
                # don't know how to get a version string from integrity
                CPP_COPTIONS="-bsp mvme5100-750 -os_dir ${I_INSTALL_DIR} --exceptions -fvolatile -O2 -fsigned-char -DINTEGRITY -I${I_INSTALL_DIR}"
                # integrity ignores CPATH
                for i in `echo $CPATH | tr ':' ' '`
                    do CPP_COPTIONS="${CPP_COPTIONS} -I${i}"
                done
                CPP_LINKOPTS="-bsp mvme5100-750 -os_dir ${I_INSTALL_DIR}  --exceptions"
                CPP_SYSLIBS="socket shm_client posix_sys_client ivfs"
            fi
        else
            CPP_COMPILER=`which $CPP_CNAME`
            CPP_CVERSION=`version`
            CPP_CVERSION=`echo $CPP_CVERSION | sed "s/.*Sun Studio \([0-9]*\).*/SunStudio\1/"`
        fi
    else
        CPP_COMPILER=`which $CPP_CNAME`
        CPP_CVERSION=`$CPP_COMPILER -dumpversion`
    fi
    echo Found C++ compiler $CPP_COMPILER version $CPP_CVERSION
fi
if [ -n "$SELECTED_JAVA" ]
then
    echo
    echo Configuring Java for SunOS, inspecting the Sun JDK
    JAVA_CNAME=javac
    JAVA_COMPILER=`which $JAVA_CNAME`
    JAVA_CVERSION=`$JAVA_COMPILER -version 2>&1 | head -1`
    JAVA_CVERSION=${JAVA_CVERSION#javac }
    echo Found java compiler $JAVA_COMPILER version $JAVA_CVERSION
fi
