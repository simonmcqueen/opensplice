@ECHO OFF

CALL makefiles\Makewin.defs_java.bat

SET TOUCHSTONE_JAR=%BIN_DIR%\%TOUCHSTONE%.jar
SET TOUCHSTONE_DIR=%TOUCHSTONE%\java\touchstone
SET DDSHELP=ddshelp
SET DDSHELP_DIR=%DDSHELP%\java\ddshelp
SET GEN_DIR=%GEN_BASE_DIR%\java
SET GEN_OBJ_DIR=%OBJ_DIR%

SET DDSHELP_ITEMS=Attachment ConditionMgr DDSError Dispatcher EntityMgr FilteredTopicMgr GuardConditionMgr ParticipantMgr PublisherMgr QueryConditionMgr ReadConditionMgr ReaderMgr StatusConditionMgr SubscriberMgr TopicDescriptionMgr TopicMgr WriterMgr
SET TOUCHSTONE_ITEMS=Main Partition Processor Receiver Transceiver Transmitter Transponder

IF NOT EXIST %GEN_DIR% MD %GEN_DIR%
CALL scripts\%DDS_VENDOR%\idlCompilation.bat "%IDL_DIR%\%IDL%.idl" "%GEN_DIR%" java -I"%IDL_DIR%"

IF NOT EXIST %OBJ_DIR% MD %OBJ_DIR%

echo Compiling generated Java code
%JAVA_COMPILER% %JAVA_COPTIONS% -sourcepath %GEN_DIR%\%GENERATED_JAVA_SUBDIR% -d %GEN_OBJ_DIR% -classpath "%OBJ_DIR%;%DDS_JARS%" %GEN_DIR%\%GENERATED_JAVA_SUBDIR%\*.java
echo Compiling ddshelp Java code
%JAVA_COMPILER% %JAVA_COPTIONS% -sourcepath %DDSHELP_DIR% -d %OBJ_DIR% -classpath "%OBJ_DIR%;%DDS_JARS%" %DDSHELP_DIR%\*.java
echo Compiling touchstone Java code
%JAVA_COMPILER% %JAVA_COPTIONS% -sourcepath %TOUCHSTONE_DIR% -d %OBJ_DIR% -classpath "%OBJ_DIR%;%DDS_JARS%" %TOUCHSTONE_DIR%\*.java

IF NOT EXIST %BIN_DIR% MD %BIN_DIR%
echo Creating touchstone jar file
jar cf %TOUCHSTONE_JAR% -C "%OBJ_DIR%" %GENERATED_JAVA_SUBDIR% -C "%OBJ_DIR%" %DDSHELP% -C "%OBJ_DIR%" %TOUCHSTONE%
