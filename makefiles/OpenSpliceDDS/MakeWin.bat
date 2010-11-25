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

REM Define the correct include paths for the C compilation

REM Set names of generated C files
SET GENERATED_C_SOURCE=%IDL%SacDcps %IDL%SplDcps

REM Set DDS-specific C include dirs
SET DDS_C_INCLUDES=/I"%OSPL_HOME%\include"
SET DDS_C_INCLUDES=%DDS_C_INCLUDES% /I"%OSPL_HOME%\include\sys"
SET DDS_C_INCLUDES=%DDS_C_INCLUDES% /I"%OSPL_HOME%\include\dcps\C\SAC"

REM Set DDS-specific C libraries
SET DDS_C_LDLIBS=/LIBPATH:"%OSPL_HOME%\lib"
SET DDS_C_LDLIBS=%DDS_C_LDLIBS% ddsos.lib
SET DDS_C_LDLIBS=%DDS_C_LDLIBS% ddsdatabase.lib
SET DDS_C_LDLIBS=%DDS_C_LDLIBS% dcpssac.lib


REM Set names of generated C++ files
SET GENERATED_CPP_SOURCE=%IDL% %IDL%Dcps %IDL%Dcps_impl %IDL%SplDcps

REM Set DDS-specific C++ include dirs
SET DDS_CPP_INCLUDES=/I"%OSPL_HOME%\include"
SET DDS_CPP_INCLUDES=%DDS_CPP_INCLUDES% /I"%OSPL_HOME%\include\sys"
SET DDS_CPP_INCLUDES=%DDS_CPP_INCLUDES% /I"%OSPL_HOME%\include\dcps\C++\SACPP"

SET DDS_CPP_IDL_INCLUDE=%OSPL_HOME%\include\dcps\C++\SACPP

REM Set DDS-specific C++ libraries
SET DDS_CPP_LDLIBS=/LIBPATH:"%OSPL_HOME%\lib"
SET DDS_CPP_LDLIBS=%DDS_CPP_LDLIBS% ddsos.lib
SET DDS_CPP_LDLIBS=%DDS_CPP_LDLIBS% ddsdatabase.lib
SET DDS_CPP_LDLIBS=%DDS_CPP_LDLIBS% dcpsgapi.lib
SET DDS_CPP_LDLIBS=%DDS_CPP_LDLIBS% dcpssacpp.lib


REM Set names of generated Java files
SET GENERATED_JAVA_SUBDIR=DDSTouchStone

SET DDS_JARS=%OSPL_HOME%\jar\dcpssaj.jar

