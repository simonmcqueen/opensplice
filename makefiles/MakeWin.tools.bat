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

CALL makefiles\Makewin.defs_c.bat

SET OS_LDLIBS=/LIBPATH:"%MSVC%\lib"
SET OS_LDLIBS=%OS_LDLIBS% /LIBPATH:"%PLATFORMSDK%\lib"

SET GEN_DIR=%GEN_BASE_DIR%\c
SET GEN_INCLUDES=/I"%GEN_DIR%"

SET TOOLS_DIR=tools
SET TOOLS=errorlog
SET TOOLS=%TOOLS% excelerator
SET TOOLS=%TOOLS% recorder
SET TOOLS=%TOOLS% spotter
SET TOOLS=%TOOLS% watcher
SET TOOLSHELPERS=commandHelpers

SET LINKOBJS=
FOR %%f IN (%OS_SOURCE% %GENERATED_C_SOURCE% %TOOLSHELPERS%) DO SET LINKOBJS=!LINKOBJS! %OBJ_DIR%\%%f.obj

IF NOT EXIST %GEN_DIR% MD %GEN_DIR%
CALL scripts\%DDS_VENDOR%\idlCompilation.bat "%IDL_DIR%\%IDL%.idl" "%GEN_DIR%" c

IF NOT EXIST %OBJ_DIR% MD %OBJ_DIR%

FOR %%f IN (%GENERATED_C_SOURCE%) DO %C_COMPILER% /nologo %OS_INCLUDES% %DDS_C_INCLUDES% /c "%TOUCHSTONE_HOME%\%GEN_DIR%\%%f.c" /Fo%OBJ_DIR%\%%f.obj
FOR %%f IN (%OS_SOURCE%) DO %C_COMPILER% /nologo %OS_INCLUDES% %DDS_C_INCLUDES% /c "%OS_DIR%\%%f.c" /Fo%OBJ_DIR%\%%f.obj
FOR %%f IN (%TOOLSHELPERS%) DO %C_COMPILER% /wd4996 /nologo %VENDOR_INCLUDES% %DDS_C_INCLUDES% %GEN_INCLUDES% %OS_INCLUDES% /c "%TOOLS_DIR%\%%f.c" /Fo%OBJ_DIR%\%%f.obj
FOR %%f IN (%TOOLS%) DO %C_COMPILER% /wd4996 /Dstrcasecmp=strcmpi /nologo %VENDOR_INCLUDES% %DDS_C_INCLUDES% %GEN_INCLUDES% %OS_INCLUDES% /c "%TOOLS_DIR%\%%f.c" /Fo%OBJ_DIR%\%%f.obj

IF NOT EXIST %BIN_DIR% MD %BIN_DIR%
FOR %%f in (%TOOLS%) DO %C_LINKER% /NOLOGO %OBJ_DIR%\%%f.obj %LINKOBJS% %OS_LDLIBS% %DDS_C_LDLIBS% /OUT:%BIN_DIR%\%%f.exe