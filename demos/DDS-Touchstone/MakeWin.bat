@ECHO OFF

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

IF "_%1"=="_" (
  SET TARGET=all
) ELSE (
  SET TARGET=%1
)

SETLOCAL ENABLEDELAYEDEXPANSION

SET IDL=ddsTouchStone
SET CONFIG_NAME=windows

call makefiles\MakeWin.defs.bat
call makefiles\%DDS_VENDOR%\MakeWin.bat

SET TARGET_NAME=%TARGET_FULLNAME%.%OS_ABSTRACTION%.%CONFIGURATION_NAME%

SET OS_DIR=os\%OS_ABSTRACTION%
SET VENDOR_DIR=vendor\%DDS_VENDOR%
SET GEN_BASE_DIR=vendor\%DDS_VENDOR%\generated
SET OBJ_DIR=objs\%CONFIG_NAME%
SET BIN_DIR=bin\%CONFIG_NAME%
SET IDL_DIR=%TOUCHSTONE_HOME%\idl

SET OS_SOURCES=touchstone_os_abstraction

SET OS_INCLUDES=/I"%MSVC%\include"
SET OS_INCLUDES=%OS_INCLUDES% /I"%PLATFORMSDK%\include"
SET OS_INCLUDES=%OS_INCLUDES% /Ios\include
SET OS_INCLUDES=%OS_INCLUDES% /I"%OS_DIR%"

SET VENDOR_INCLUDES=/Ivendor\include
SET VENDOR_INCLUDES=%VENDOR_INCLUDES% /I"%VENDOR_DIR%"

SET OS_SOURCE=touchstone_os_abstraction
 
SET TOUCHSTONE=touchstone


IF "%TARGET%"=="clean" GOTO Clean
IF "%TARGET%"=="cleanall" GOTO CleanAll
IF "%TARGET%"=="touchstone_c" GOTO Touchstone_c
IF "%TARGET%"=="touchstone_cpp" GOTO Touchstone_cpp
IF "%TARGET%"=="touchstone_java" GOTO Touchstone_java
IF "%TARGET%"=="tools" GOTO Tools
IF "%TARGET%"=="all" GOTO Touchstone_c

ECHO Unknown target %TARGET%
GOTO End

:Clean
ECHO Making clean...
FOR %%f IN (%OBJ_DIR% %BIN_DIR%) DO (
  IF EXIST %%f (
    ECHO Removing %%f...
    RMDIR /S /Q %%f
  )
)
GOTO End


:CleanAll
ECHO Making cleanall...
FOR %%f IN (objs bin %GEN_BASE_DIR%) DO (
  IF EXIST %%f (
    ECHO Removing %%f...
    RMDIR /S /Q %%f
  )
)
GOTO End


:Touchstone_c
echo Making touchstone_c
CALL makefiles\MakeWin.touchstone_c.bat
IF "%TARGET%"=="touchstone_c" GOTO End


:Touchstone_cpp
echo Making touchstone_cpp
CALL makefiles\MakeWin.touchstone_cpp.bat
IF "%TARGET%"=="touchstone_cpp" GOTO End

:Touchstone_java
echo Making touchstone_java
CALL makefiles\MakeWin.touchstone_java.bat
IF "%TARGET%"=="touchstone_java" GOTO End

:Tools
echo Making tools
CALL makefiles\MakeWin.tools.bat


:End