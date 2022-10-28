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

lib-$(CONFIG_LIBRARY_TEMPLATE_STATIC) += enable_static
lib-$(CONFIG_LIBRARY_TEMPLATE_SHARED) += enable_shared

CFLAGS += -DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR) -DBUILD_DATE=\"$(BUILD_DATE)\"
CFLAGS += -DNAME_STRING=\"lib$(NAME_STRING)\"

#####################################################################################

ELF = lib$(NAME_STRING)

all: $(obj-y) $(lib-y)

enable_static:
	$(AR) $(ELF).a.$(VERSION_MAJOR).$(VERSION_MINOR) $(obj-y)
	ln -sf $(ELF).a.$(VERSION_MAJOR).$(VERSION_MINOR) $(ELF).a.$(VERSION_MAJOR)
	ln -sf $(ELF).a.$(VERSION_MAJOR).$(VERSION_MINOR) $(ELF).a
	cp -af $(ELF).a* $(SRC_LIB)
	cp -af *.h $(SRC_INC)

enable_shared:
	$(CC) -shared -o $(ELF).so.$(VERSION_MAJOR).$(VERSION_MINOR) $(obj-y)
	$(STRIP) $(ELF).so.$(VERSION_MAJOR).$(VERSION_MINOR)
	ln -sf $(ELF).so.$(VERSION_MAJOR).$(VERSION_MINOR) $(ELF).so.$(VERSION_MAJOR)
	ln -sf $(ELF).so.$(VERSION_MAJOR).$(VERSION_MINOR) $(ELF).so
	cp -af $(ELF).so* $(SRC_LIB)
	cp -af $(ELF).so* $(VFS_LIB)
	cp -af *.h $(SRC_INC)

#########################################################################

clean:
	rm -f *.o $(ELF).a* $(ELF).so* $(SRC_LIB)/$(ELF).* $(VFS_LIB)/$(ELF).*
