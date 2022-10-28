# Copyright (c) 2022-2023 tangchunhui@coros.com
#
# SPDX-License-Identifier: Apache-2.0

include $(TOPDIR)/config.mk

CURRENT_DIR := $(shell pwd)
NAME_STRING := $(subst $(suffix $(CURRENT_DIR)),,$(shell basename $(CURRENT_DIR)))
CURRENT_MAJOR = $(subst .,,$(suffix $(CURRENT_DIR)))
VERSION_MAJOR := $(if $(CURRENT_MAJOR),$(CURRENT_MAJOR),0)
VERSION_MINOR := 1

#####################################################################################

obj-y = $(patsubst %.c, %.o, $(wildcard *.c))

CFLAGS += -DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR) -DBUILD_DATE=\"$(BUILD_DATE)\"
CFLAGS += -DNAME_STRING=\"$(NAME_STRING)\"

SRC_LIBS += -ltemplate

#####################################################################################

ELF = $(NAME_STRING)

all: $(obj-y)
	$(CC) $(CFLAGS) $(LIBS) $(obj-y) \
	-L $(SRC_LIB) $(SRC_LIBS) \
	-o $(ELF)
	$(STRIP) $(ELF)
	chmod 755 $(ELF)
	cp -a $(ELF) $(VFS_BIN)

#########################################################################

clean:
	rm -f *.o $(ELF) $(VFS_BIN)/$(ELF)
