# $Id$

RELPATH ?= .
KDIR_LNK=$(MAKEFILE_DIR)/$(RELPATH)/.kdir
ifneq (,$(wildcard $(KDIR_LNK)))
  KDIR ?= $(shell readlink -f $(KDIR_LNK) 2>/dev/null)
endif

ifneq (,$(KDIR))
  KERNEL_SRC ?= $(KDIR)
  O ?= $(KDIR)
endif

KERNELRELEASE_FILE := $(O)/include/config/kernel.release
ifeq ("$(wildcard $(KERNELRELEASE_FILE))","")
  $(warning WARNING: [$(KERNELRELEASE_FILE)] does not exists)
else
  KERNELRELEASE ?= $(shell cat $(KERNELRELEASE_FILE) 2>/dev/null)
endif

SDK_DIR ?= $(shell readlink -f $(MAKEFILE_DIR)/$(RELPATH)/.sdk 2>/dev/null)
SDK_ENV_FILE = $(SDK_DIR)/environment-setup-armv8a-poky-linux
ifneq (,$(wildcard $(SDK_ENV_FILE)))
  SDK_PREFIX = . $(SDK_ENV_FILE) &&
  NPROC = -j$(shell nproc)
else
  export CROSS_COMPILE?=aarch64-linux-gnu-
  export ARCH?=arm64
  SDK_PREFIX = 
endif
