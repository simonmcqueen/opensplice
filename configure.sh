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
# Determine machine properties
MACHINE=`uname -m`
KERNEL_NAME=`uname -s`
KERNEL_RELEASE=`uname -r`
HOST_FULLNAME=$MACHINE.$KERNEL_NAME.$KERNEL_RELEASE
# Currently no cross compilation supported by this configure script
# TARGET_FULLNAME=$HOST_FULLNAME

echo
echo Configuring DDSTouchStone for compilation on $HOST_FULLNAME
echo

TOUCHSTONE_HOME=`pwd | sed 's~^/nfs/[^/]*~~'`

C_COPTIONS="-g -m32"
CPP_COPTIONS="-g -m32"

# Select DDS vendor...
#DDS_VENDORS=`ls vendor`
# Remove uninteresting subdirs
#DDS_VENDORS=`echo $DDS_VENDORS | sed -r "s/(include|CVS|RTIDDS|OpenDDS)//g"`
DDS_VENDORS=OpenSpliceDDS
DDS_VENDOR=""
while [ -z "$DDS_VENDOR" ]
do
    echo
    echo "Available DDS vendors are:"
    n=1
    for i in $DDS_VENDORS
    do
        echo $n ">" $i
        n=`expr $n + 1`
    done
    printf "Please select a DDS vendor number: "
    read ARGUMENT
    n=1
    for i in $DDS_VENDORS
    do
        if [ "$n" = "$ARGUMENT" ]
        then
            DDS_VENDOR=$i
        fi
        n=`expr $n + 1`
    done
    if [ -z "$DDS_VENDOR" ]
    then
        echo
        echo "Invalid selection, try again!"
    fi
done

# Select OS abstraction implementation
#OS_ABSTRACTIONS=`ls os`
# Remove uninteresting subdirs
#OS_ABSTRACTIONS=`echo $OS_ABSTRACTIONS | sed -r "s/(include|CVS|ACE)//g"`
OS_ABSTRACTIONS="OpenSpliceDDS posix"
OS_ABSTRACTION=""
while [ -z "$OS_ABSTRACTION" ]
do
    echo
    echo "Available OS abstraction implementations are:"
    n=1
    for i in $OS_ABSTRACTIONS
    do
        echo $n ">" $i
        n=`expr $n + 1`
    done
    printf "Please select a OS abstraction implementation number: "
    read ARGUMENT
    n=1
    for i in $OS_ABSTRACTIONS
    do
        if [ "$n" = "$ARGUMENT" ]
        then
            OS_ABSTRACTION=$i
        fi
        n=`expr $n + 1`
    done
    if [ -z "$OS_ABSTRACTION" ]
    then
        echo
        echo "Invalid selection, try again!"
    fi
done

# Select targets to build
TARGET_ALL=all_languages_and_tools
TARGET_C=touchstone_c
TARGET_CPP=touchstone_cpp
TARGET_JAVA=touchstone_java
TARGET_TOOLS=tools
TARGET_DONE="<done>"

if [ "$KERNEL_NAME" = 'AIX' ]
then
    TOUCHSTONE_TARGETS="$TARGET_C $TARGET_TOOLS $TARGET_DONE"
else
    TOUCHSTONE_TARGETS="$TARGET_ALL $TARGET_C $TARGET_CPP $TARGET_JAVA $TARGET_TOOLS $TARGET_DONE"
fi

SELECTED_TARGETS=""
NOMORE_TARGETS=""
while [ -z "$NOMORE_TARGETS" ]
do
    PRINT_TARGETS=""
    n=1
    for i in $TOUCHSTONE_TARGETS
    do
        FOUND=""
        for j in $SELECTED_TARGETS
        do
            if [ "$i" = "$j" ]
            then
                FOUND=TRUE
            fi
        done
        if [ -z "$FOUND" ]
        then
            PRINT_TARGETS="$PRINT_TARGETS $i"
        fi
    done
    if [ "$PRINT_TARGETS" = " $TARGET_DONE" ]
    then
          NOMORE_TARGETS=TRUE
    else
        echo
        echo Available target components are:
        for i in $PRINT_TARGETS
        do
            echo $n ">" $i
            n=`expr $n + 1`
        done
        if [ -z "$SELECTED_TARGETS" ]
        then
            printf "Select desired target component by number: "
        else
            printf "Select another target component or select <done> when done: "
        fi
        read ARGUMENT
        n=1
        SELECTED_TARGET=""
        for i in $PRINT_TARGETS
        do
            if [ "$n" = "$ARGUMENT" ]
            then
                SELECTED_TARGET=$i
            fi
            n=`expr $n + 1`
        done
        if [ -z "$SELECTED_TARGET" ]
        then
            echo
            echo "Invalid selection, try again!"
        else
            if [ "$SELECTED_TARGET" = "$TARGET_DONE" ]
            then
              if [ -z "$SELECTED_TARGETS" ]
               then
                  printf "\nPLEASE FIRST SELECT A TARGET COMPONENT TO INSTALL\n"
              else  
                  NOMORE_TARGETS=TRUE
              fi
            else
                SELECTED_TARGETS="$SELECTED_TARGETS $SELECTED_TARGET"
                if [ "$SELECTED_TARGET" = "$TARGET_C" ]; then SELECTED_C=TRUE; fi
                if [ "$SELECTED_TARGET" = "$TARGET_CPP" ]; then SELECTED_CPP=TRUE; fi
                if [ "$SELECTED_TARGET" = "$TARGET_JAVA" ]; then SELECTED_JAVA=TRUE; fi
                if [ "$SELECTED_TARGET" = "$TARGET_TOOLS" ]; then SELECTED_C=TRUE; fi
                if [ "$SELECTED_TARGET" = "$TARGET_ALL" ];
                  then
                   SELECTED_C=TRUE
                   SELECTED_CPP=TRUE
                   SELECTED_JAVA=TRUE
                   SELECTED_TARGETS="$TARGET_C $TARGET_CPP $TARGET_JAVA $TARGET_TOOLS"
                   NOMORE_TARGETS=TRUE
                fi 
            fi
        fi
    fi
done


GENMAKEFILE_DIR=makefiles/generated

CONFIGURE_SCRIPT=scripts/$KERNEL_NAME/configure.sh
if test -f $CONFIGURE_SCRIPT
then
    . ./$CONFIGURE_SCRIPT

    if [ -n "$SELECTED_CPP" ]; then DEFAULT_CONFIGNAME=$CPP_CNAME.$CPP_CVERSION; fi
    if [ -z "$DEFAULT_CONFIGNAME" ]; then
        if [ -n "$SELECTED_JAVA" ]; then DEFAULT_CONFIGNAME=$JAVA_CNAME.$JAVA_CVERSION
        else DEFAULT_CONFIGNAME=$C_CNAME.$C_CVERSION; fi
    fi
    echo
    printf "Please enter a symbolic name for this configuration [$DEFAULT_CONFIGNAME]: "
    read ARGUMENT
    if [ -z "$ARGUMENT" ]
    then
        CONFIG_NAME=$DEFAULT_CONFIGNAME
    else
        CONFIG_NAME=$ARGUMENT
    fi
    CONFIG_DIR=$GENMAKEFILE_DIR/$CONFIG_NAME
    if test ! -d $CONFIG_DIR
    then
        mkdir -p $CONFIG_DIR
    fi


    #Makefile wilth generic include statements
    MAKEFILE_FULLNAME=$CONFIG_DIR/Makefile.defs

    if test -f $MAKEFILE_FULLNAME
    then
        printf "$MAKEFILE_FULLNAME exists, overwrite [yes]? "
        read ANSWER
        if [ -z "$ANSWER" ]; then ANSWER=yes; fi
        if [ "_$ANSWER" = "_yes" ]
        then
            rm $MAKEFILE_FULLNAME
        else
            echo Exiting now...
            exit
        fi
    fi

    cat > $MAKEFILE_FULLNAME <<-xxx
TOUCHSTONE_HOME       := $TOUCHSTONE_HOME
TARGET_MACHINE        := $MACHINE
TARGET_KERNEL_NAME    := $KERNEL_NAME
TARGET_KERNEL_RELEASE := $KERNEL_RELEASE
DDS_VENDOR            := $DDS_VENDOR
OS_ABSTRACTION        := $OS_ABSTRACTION
CONFIGURATION_NAME    := $CONFIG_NAME
TARGETS               := $SELECTED_TARGETS
xxx

    # Makefile for generating C code

    MAKEFILE_FULLNAME=$CONFIG_DIR/Makefile.defs_c
    if [ -n "$SELECTED_C" ]
    then
        if test -f $MAKEFILE_FULLNAME
        then
            printf "$MAKEFILE_FULLNAME exists, overwrite [yes]? "
            read ANSWER
            if [ -z "$ANSWER" ]; then ANSWER=yes; fi
            if [ "_$ANSWER" = "_yes" ]
            then
                rm $MAKEFILE_FULLNAME
            else
                echo Exiting now...
                exit
            fi
        fi

        cat > $MAKEFILE_FULLNAME <<-xxx
C_COMPILER           := $C_COMPILER
C_COMPILERNAME       := $C_CNAME
C_COMPILERVERSION    := $C_CVERSION
C_LINKER             := $C_COMPILER
C_COPTIONS           := $C_COPTIONS
C_LINKOPTS           := $C_LINKOPTS
C_SYSLIBS            := $C_SYSLIBS
xxx
    else
        if test -f $MAKEFILE_FULLNAME
        then
            rm $MAKEFILE_FULLNAME
        fi
    fi



    #Makefile for generating C++ code

    MAKEFILE_FULLNAME=$CONFIG_DIR/Makefile.defs_cpp
    if [ -n "$SELECTED_CPP" ]
    then
        if test -f $MAKEFILE_FULLNAME
        then
            printf "$MAKEFILE_FULLNAME exists, overwrite [yes]? "
            read ANSWER
            if [ -z "$ANSWER" ]; then ANSWER=yes; fi
            if [ "_$ANSWER" = "_yes" ]
            then
                rm $MAKEFILE_FULLNAME
            else
                echo Exiting now...
                exit
            fi
        fi

        cat > $MAKEFILE_FULLNAME <<-xxx
CPP_COMPILER           := $CPP_COMPILER
CPP_COMPILERNAME       := $CPP_CNAME
CPP_COMPILERVERSION    := $CPP_CVERSION
CPP_LINKER             := $CPP_COMPILER
CPP_COPTIONS           := $CPP_COPTIONS
CPP_LINKOPTS           := $CPP_LINKOPTS
CPP_SYSLIBS            := $CPP_SYSLIBS
xxx
    else
        if test -f $MAKEFILE_FULLNAME
        then
            rm $MAKEFILE_FULLNAME
        fi
    fi


    # Makefile for generating Java code

    MAKEFILE_FULLNAME=$CONFIG_DIR/Makefile.defs_java
    if [ -n "$SELECTED_JAVA" ]
    then
        if test -f $MAKEFILE_FULLNAME
        then
            printf "$MAKEFILE_FULLNAME exists, overwrite [yes]? "
            read ANSWER
            if [ -z "$ANSWER" ]; then ANSWER=yes; fi
            if [ "_$ANSWER" = "_yes" ]
            then
                rm $MAKEFILE_FULLNAME
            else
                echo Exiting now...
                exit
            fi
        fi

        cat > $MAKEFILE_FULLNAME <<-xxx
JAVA_COMPILER           := $JAVA_COMPILER
JAVA_COMPILERNAME       := $JAVA_CNAME
JAVA_COMPILERVERSION    := $JAVA_CVERSION
JAVA_COPTIONS           := -Xlint:unchecked
xxx
    else
        if test -f $MAKEFILE_FULLNAME
        then
            rm $MAKEFILE_FULLNAME
        fi
    fi
else
    echo Scriptfile $CONFIGURE_SCRIPT not found.
    echo Exiting now...
    exit
fi

echo
echo Configuration successful, exiting...
