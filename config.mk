# Copyright (c) 2022-2023 tangchunhui@coros.com
#
# SPDX-License-Identifier: Apache-2.0

KCFGDIR=$(TOPDIR)/include
DCFGDIR=$(TOPDIR)/configs

SRC_INC=$(TOPDIR)/source/inc
SRC_LIB=$(TOPDIR)/source/lib
SRC_SRC=$(TOPDIR)/source/src

$(shell mkdir -p $(SRC_INC))
$(shell mkdir -p $(SRC_LIB))

EXT_INC=$(TOPDIR)/extern/inc
EXT_LIB=$(TOPDIR)/extern/lib

VFS_DIR=$(TOPDIR)/vfs
VFS_LIB=$(VFS_DIR)/usr/lib
VFS_BIN=$(VFS_DIR)/usr/bin
VFS_MOD=$(VFS_DIR)/lib/modules

$(shell mkdir -p $(VFS_LIB))
$(shell mkdir -p $(VFS_BIN))
$(shell mkdir -p $(VFS_MOD))

#####################################################################################

CURCONFIG ?= .config
DEFCONFIG ?= $(DCFGDIR)/defconfig

AUTOCONF = $(KCFGDIR)/config/auto.conf
ifeq ($(AUTOCONF), $(wildcard $(AUTOCONF)))
	include $(AUTOCONF)
else ifeq ($(CURCONFIG), $(wildcard $(CURCONFIG)))
	include $(CURCONFIG)
endif

#####################################################################################

AS		= $(CROSS_COMPILER)as
LD		= $(CROSS_COMPILER)ld
CC		= $(CROSS_COMPILER)gcc
#CPP		= $(CC) -E
CXX		= $(CROSS_COMPILER)g++
AR		= $(CROSS_COMPILER)ar -rsv
NM		= $(CROSS_COMPILER)nm
STRIP	= $(CROSS_COMPILER)strip
OBJCOPY = $(CROSS_COMPILER)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
RANLIB	= $(CROSS_COMPILER)ranlib

BUILD_DATE=$(shell date '+%Y-%m-%d\ %H:%M:%S')

CPPFLAGS:= --include $(KCFGDIR)/generated/autoconf.h -I $(SRC_INC) -I $(EXT_INC)

ifdef CONFIG_DEBUG
CPPFLAGS += -g -DDEBUG
else
CPPFLAGS += -Os
endif

CFLAGS  := $(CPPFLAGS) -fPIC -Wall -O2 -Werror
LDFLAGS := -Bstatic -EL

#####################################################################################

export TOPDIR BUILD_DATE

#####################################################################################
