# $Id$

RELPATH ?= .

OPENWRT_LNK=$(MAKEFILE_DIR)/$(RELPATH)/.openwrt
ifneq (,$(wildcard $(OPENWRT_LNK)))
	OPENWRT ?= $(shell readlink -f $(OPENWRT_LNK) 2>/dev/null)
endif

ifneq (,$(OPENWRT))
	KDIR := $(firstword $(wildcard $(OPENWRT)/build_dir/target-mips_24kc_musl/linux-ath79*/linux-6.*))
	export STAGING_DIR ?= $(OPENWRT)/staging_dir
	# Auto-detect toolchain path
	TOOLCHAIN := $(firstword $(wildcard $(OPENWRT)/staging_dir/toolchain-mips_24kc_gcc-*/))
	export CROSS_COMPILE ?= $(TOOLCHAIN)/bin/mips-openwrt-linux-musl-
	export ARCH ?= mips
endif


KDIR_LNK ?= $(MAKEFILE_DIR)/$(RELPATH)/.kdir
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
