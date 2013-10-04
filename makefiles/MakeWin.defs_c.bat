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

REM Try to figure out where VC and the Windows SDK are installed
SET "MSVC=%VCINSTALLDIR%"
IF NOT "%WindowsSdkDir%"=="" SET "PLATFORMSDK=%WindowsSdkDir%"
IF NOT "%FrameworkSdkDir%"=="" SET "PLATFORMSDK=%FrameworkSdkDir%"

IF "%MSVC%"=="" ECHO No MS Visual C installation found, you might want to set it manually in MakeWin.defs.bat
IF "%PLATFORMSDK%"=="" ECHO No Windows Platform SDK directory found, you might want to set it manually in MakeWin.defs.bat

SET C_COMPILER=cl
SET C_COMPILERNAME=VisualStudio
SET C_COMPILERVERSION=9.0
SET C_LINKER=link
