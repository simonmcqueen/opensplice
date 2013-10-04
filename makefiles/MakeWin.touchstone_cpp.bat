REM
REM DDSTouchStone: a scenario-driven Open Source benchmarking framework
REM for evaluating the performance of OMG DDS compliant implementations.
REM
REM Copyright (C) 2008-2009 PrismTech Ltd.
REM ddstouchstone@prismtech.com
REM
REM This library is free software; you can redistribute it and/or
REM modify it under the terms of the GNU Lesser General Public
REM License Version 3 dated 29 June 2007, as published by the
REM Free Software Foundation.
REM
REM This library is distributed in the hope that it will be useful,
REM but WITHOUT ANY WARRANTY; without even the implied warranty of
REM MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
REM Lesser General Public License for more details.
REM
REM You should have received a copy of the GNU Lesser General Public
REM License along with DDSTouchStone; if not, write to the Free Software
REM Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

@ECHO OFF

CALL makefiles\Makewin.defs_cpp.bat

SET OS_LDLIBS=/LIBPATH:"%MSVC%\lib"
SET OS_LDLIBS=%OS_LDLIBS% /LIBPATH:"%PLATFORMSDK%\lib"

SET TOUCHSTONE_DIR=%TOUCHSTONE%\cpp
SET DDSHELP_DIR=ddshelp\cpp
SET DDSHELP_INCLUDES=/I"%DDSHELP_DIR%"
SET GEN_DIR=%GEN_BASE_DIR%\cpp
SET GEN_INCLUDES=/I"%GEN_DIR%"

SET DDSHELP_ITEMS=ConditionMgr DDSError ParticipantMgr PublisherMgr ReaderMgr SubscriberMgr TopicMgr WriterMgr
SET TOUCHSTONE_ITEMS=Main Partition Processor Receiver Transceiver Transmitter Transponder

SET LINKOBJS=
FOR %%f IN (%OS_SOURCE% %GENERATED_CPP_SOURCE% %DDSHELP_ITEMS% %TOUCHSTONE_ITEMS%) DO SET LINKOBJS=!LINKOBJS! %OBJ_DIR%\%%f.obj

IF NOT EXIST %GEN_DIR% MD %GEN_DIR%
CALL scripts\%DDS_VENDOR%\idlCompilation.bat "%IDL_DIR%\%IDL%.idl" "%GEN_DIR%" cpp -I"%DDS_CPP_IDL_INCLUDE%" -I"%IDL_DIR%"

IF NOT EXIST %OBJ_DIR% MD %OBJ_DIR%

FOR %%f IN (%GENERATED_CPP_SOURCE%) DO %CPP_COMPILER% /EHsc /nologo %OS_INCLUDES% %DDS_CPP_INCLUDES% /c "%TOUCHSTONE_HOME%\%GEN_DIR%\%%f.cpp" /Fo%OBJ_DIR%\%%f.obj
FOR %%f IN (%OS_SOURCE%) DO %CPP_COMPILER% /nologo %OS_INCLUDES% %DDS_CPP_INCLUDES% /c "%OS_DIR%\%%f.c" /Fo%OBJ_DIR%\%%f.obj
FOR %%f IN (%DDSHELP_ITEMS%) DO %CPP_COMPILER% /wd4996 /EHsc /nologo %VENDOR_INCLUDES% %GEN_INCLUDES% %DDS_CPP_INCLUDES% %OS_INCLUDES% /c "%DDSHELP_DIR%\%%f.cpp" /Fo%OBJ_DIR%\%%f.obj
FOR %%f IN (%TOUCHSTONE_ITEMS%) DO %CPP_COMPILER% /wd4996 /EHsc /nologo %DDSHELP_INCLUDES% %VENDOR_INCLUDES% %GEN_INCLUDES% %DDS_CPP_INCLUDES% %OS_INCLUDES% /c "%TOUCHSTONE_DIR%\%%f.cpp" /Fo%OBJ_DIR%\%%f.obj

IF NOT EXIST %BIN_DIR% MD %BIN_DIR%
%CPP_LINKER% /nologo %LINKOBJS% %OS_LDLIBS% %DDS_CPP_LDLIBS% /OUT:%BIN_DIR%\touchstone_cpp.exe
