# Makefile for PPE modules

ccflags-y := -I$(obj) -I$(obj)/..
ccflags-y += -Wall -Werror

export BUILD_ID = \"Build Id: $(shell date +'%m/%d/%y, %H:%M:%S')\"
ccflags-y += -DNSS_PPE_BUILD_ID="$(BUILD_ID)"

KERNELVERSION := $(word 1, $(subst ., ,$(KERNELVERSION))).$(word 2, $(subst ., ,$(KERNELVERSION)))

obj-y += drv/
ifeq ($(ppe-vp),)
ifeq ($(ppe-tun),)
ifeq ($(ppe-ds),)
obj-y += clients/
endif
endif
endif
obj ?= .
