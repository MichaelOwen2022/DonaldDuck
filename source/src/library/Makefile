# Copyright (c) 2022-2023 tangchunhui@coros.com
#
# SPDX-License-Identifier: Apache-2.0

include $(TOPDIR)/config.mk

#####################################################################################

obj-$(CONFIG_LIBRARY_TEMPLATE) += template
obj-$(CONFIG_LIBRARY_MINIWAVE) += miniwave

#####################################################################################

all: $(obj-y)
	@for dir in $(obj-y) ; do $(MAKE) -C $$dir all || exit "$$?"; done

clean:
	@for dir in $(obj-y) ; do $(MAKE) -C $$dir clean ; done
