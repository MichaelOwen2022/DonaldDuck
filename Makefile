# Copyright (c) 2022-2023 tangchunhui@coros.com
#
# SPDX-License-Identifier: Apache-2.0

TOPDIR=$(shell pwd)

include $(TOPDIR)/config.mk

#####################################################################################

obj-y = $(SRC_SRC)

all: $(CURCONFIG) syncconfig $(obj-y)
	@for dir in $(obj-y) ; do $(MAKE) -C $$dir all || exit "$$?"; done

clean: $(CURCONFIG)
	@for dir in $(obj-y) ; do $(MAKE) -C $$dir clean ; done

distclean: clean
	rm -rf $(CURCONFIG) $(CURCONFIG).old $(KCFGDIR) $(SRC_INC) $(SRC_LIB) $(VFS_DIR)

$(CURCONFIG):
	@echo >&2 '***'
	@echo >&2 '*** Configuration file "$@" not found!'
	@echo >&2 '***'
	@echo >&2 '*** Please run some configurator'
	@echo >&2 '*** (e.g. "make menuconfig" or "make *_defconfig").'
	@echo >&2 '***'
	@/bin/false

menuconfig:
	./scripts/kconfig/mconf Kconfig

syncconfig: $(CURCONFIG)
	./scripts/kconfig/conf --syncconfig Kconfig

savedefconfig:
	./scripts/kconfig/conf --savedefconfig=$(DEFCONFIG) Kconfig

defconfig:
	./scripts/kconfig/conf --defconfig=$(DEFCONFIG) Kconfig

FINDCONFIG=$(shell find $(DCFGDIR) -name $@ | head -1)
LOADCONFIG=$(if $(FINDCONFIG),$(FINDCONFIG),$@)

%_defconfig:
	./scripts/kconfig/conf --defconfig=$(LOADCONFIG) Kconfig

#####################################################################################
