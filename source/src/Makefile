# Copyright (c) 2022-2023 tangchunhui@coros.com
#
# SPDX-License-Identifier: Apache-2.0

include $(TOPDIR)/config.mk

#####################################################################################

obj-$(CONFIG_LIBRARY) += library
obj-$(CONFIG_SAMPLES) += samples

#####################################################################################

all: $(obj-y)
	@for dir in $(obj-y) ; do $(MAKE) -C $$dir all || exit "$$?"; done

clean:
	@for dir in $(obj-y) ; do $(MAKE) -C $$dir clean ; done
