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

#Make sure to export all variable to sub-makes
export

MAKEFILES_DIR    := makefiles
GENMAKEFILES_DIR := $(MAKEFILES_DIR)/generated


#Determine available configurations
AVAILABLE_CONFIGS := $(wildcard $(GENMAKEFILES_DIR)/*)
#Determine whether requested config is available
ifeq ($(origin CONFIG_NAME),undefined)
  $(warning No configuration name given, trying to obtain default configuration)
  FULL_CONFIG_NAME := $(firstword $(AVAILABLE_CONFIGS))
  ifeq ($(FULL_CONFIG_NAME),)
    $(error No default configuration available. Please run configure.sh)
  else
    CONFIG_NAME := $(notdir $(FULL_CONFIG_NAME))
    $(warning Using default configuration $(CONFIG_NAME).)
    $(warning If this is not what you want, use 'make CONFIG_NAME=<YourConfig>')
  endif
else
  ifeq ($(findstring $(CONFIG_NAME),$(notdir $(AVAILABLE_CONFIGS))),)
    $(error Requested configuration $(CONFIG_NAME) not found. Please run configure.sh or specify other configuration name)
  else
    FULL_CONFIG_NAME := $(GENMAKEFILES_DIR)/$(CONFIG_NAME)
  endif
endif
CONFIG_DIR      := $(FULL_CONFIG_NAME)
SCRIPTS_DIR     := scripts
IDL		:= ddsTouchStone.idl

include $(CONFIG_DIR)/Makefile.defs

ifeq ($(TARGET_KERNEL_NAME),AIX)
	include $(MAKEFILES_DIR)/$(DDS_VENDOR)/Makefile.defs.aix
else
	include $(MAKEFILES_DIR)/$(DDS_VENDOR)/Makefile.defs
endif

.PHONY: tools clean cleanall clobber

RELEASE_FILE     := release.$(CONFIG_NAME).com

OS_DIR		 := os/$(OS_ABSTRACTION)
VENDOR_DIR	 := vendor/$(DDS_VENDOR)
GEN_BASE_DIR	 := vendor/$(DDS_VENDOR)/generated
OBJ_DIR          := objs/$(CONFIG_NAME)
BIN_CONFIG_DIR   := bin/$(CONFIG_NAME)
IDL_DIR		 := $(TOUCHSTONE_HOME)/idl

STAMP_NAME	 := .STAMP

OS_SOURCES	 := touchstone_os_abstraction
OS_FILES	 := $(addprefix $(OS_DIR)/,$(addsuffix .c,$(OS_SOURCES)))
OS_OBJS		 := $(addprefix $(OBJ_DIR)/,$(addsuffix .o,$(OS_SOURCES)))

OS_INCLUDE_DIRS	 += os/include
OS_INCLUDE_DIRS	 += $(OS_DIR)

VENDOR_INCLUDE_DIRS	+= vendor/include
VENDOR_INCLUDE_DIRS	+= $(VENDOR_DIR)

DDS_INCLUDE_DIRS        += $(IDL_DIR)
DDS_TOOLS_INCLUDE_DIRS  += $(IDL_DIR)

define make_target
$(MAKE) -f $(MAKEFILES_DIR)/Makefile.$(2) $(1)
endef

define template_make
$(foreach target,$(TARGETS),$(call make_target,$(1),$(target));)
endef

all:
	@sh scripts/check_settings.sh $(TARGET_MACHINE) $(TARGET_KERNEL_NAME) $(TARGET_KERNEL_RELEASE) $(DDS_VENDOR) && \
	cat scripts/template_release.com | \
	sed -e "s%__CONFIG_NAME__%$(CONFIG_NAME)%" | \
	sed -e "s%__TARGET_MACHINE__%$(TARGET_MACHINE)%" | \
	sed -e "s%__TARGET_KERNEL_NAME__%$(TARGET_KERNEL_NAME)%" | \
	sed -e "s%__TARGET_KERNEL_RELEASE__%$(TARGET_KERNEL_RELEASE)%" | \
	sed -e "s%__DDS_VENDOR__%$(DDS_VENDOR)%" | \
        sed -e "s%__BIN_DIR__%$(TOUCHSTONE_HOME)/bin%" | \
	sed -e "s%__CONFIG_BIN_DIR__%$(TOUCHSTONE_HOME)/$(BIN_CONFIG_DIR)%" | \
	sed -e "s%__CONFIG_JAR__%$(TOUCHSTONE_HOME)/$(BIN_CONFIG_DIR)/touchstone.jar%" \
        > $(RELEASE_FILE)

	$(call template_make,$@)

clean:
	@$(call template_make,$@) \
	rm -f $(RELEASE_FILE)

tools touchstone_c touchstone_cpp touchstone_java:
	@$(call make_target,all,$@)

cleanall:
	@echo Removing generated directories; \
	rm -rf $(addprefix bin/,$(notdir $(AVAILABLE_CONFIGS))) $(addprefix objs/,$(AVAILABLE_CONFIGS)); \
	find vendor -name generated | xargs rm -rf

clobber: cleanall
	@echo Removing generated makefiles and scripts; \
	rm -rf $(GENMAKEFILES_DIR) $(addsuffix .com,$(addprefix release.,$(AVAILABLE_CONFIGS)))
