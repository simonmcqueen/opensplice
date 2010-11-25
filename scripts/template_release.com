# Release file generated for DDSTouchStone configuration __CONFIG_NAME__

# Created for the following settings:
# Architecture      = __TARGET_MACHINE__
# Operating System  = __TARGET_KERNEL_NAME__
# OS Kernel version = __TARGET_KERNEL_RELEASE__
# DDS vendor        = __DDS_VENDOR__

if [ -n "$PATH" ]
then
    PATH=$PATH:__CONFIG_BIN_DIR__:__BIN_DIR__
else
   export PATH=__CONFIG_BIN_DIR__:__BIN_DIR__
fi
PATH=$PATH:__CONFIG_BIN_DIR__/bin

if [ -n "$CLASSPATH" ]
then
    CLASSPATH=$CLASSPATH:__CONFIG_JAR__
else
   export CLASSPATH=__CONFIG_JAR__
fi

