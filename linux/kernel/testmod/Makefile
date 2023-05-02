# $Id$

THIS_MAKEFILE := $(lastword $(MAKEFILE_LIST))
export MAKEFILE_DIR := $(realpath $(dir $(THIS_MAKEFILE)))

empty:=
space:= $(empty) $(empty)
$(if $(findstring $(space),$(MAKEFILE_DIR)),$(error ERROR: The path must not include any spaces))

-include $(MAKEFILE_DIR)/rules.mk

TARGET ?= $(notdir $(patsubst %/,%,$(MAKEFILE_DIR)))
DEVNAME ?= $(subst -,,$(TARGET))

override KDIR_LNK=$(MAKEFILE_DIR)/.kdir
ifneq (,$(wildcard $(KDIR_LNK)))
export KDIR  = $(shell readlink -f $(KDIR_LNK) 2>/dev/null)
else
$(error ERROR Link to kernel (.kdir) does not exists)
endif

KERNELRELEASE_FILE = $(KDIR)/include/config/kernel.release
ifeq ("$(wildcard $(KERNELRELEASE_FILE))","")
$(warning File [$(KERNELRELEASE_FILE)] does not exists)
endif
KERNELRELEASE = $(shell cat $(KERNELRELEASE_FILE) 2>/dev/null)

override INSTALL_LNK="$(MAKEFILE_DIR)/.install"
INSTALL_ROOT_DIR = $(shell readlink -f $(INSTALL_LNK) 2>/dev/null)

KMOD_PATH ?= extra
INSTALL_DIR  = $(INSTALL_ROOT_DIR)/lib/modules/$(KERNELRELEASE)/$(KMOD_PATH)/

SDK_DIR = $(shell readlink -f $(MAKEFILE_DIR)/.sdk 2>/dev/null)
SDK_ENV_FILE = $(SDK_DIR)/environment-setup-armv8a-poky-linux
ifneq (,$(wildcard $(SDK_ENV_FILE)))
  SDK_PREFIX = . $(SDK_ENV_FILE) &&
else
  export CROSS_COMPILE=aarch64-linux-gnu-
  export ARCH=arm64
  SDK_PREFIX = 
endif


MAKEFLAGS += --no-print-directory
# build quietly by default.  For a verbose build, run "make V=1"
ifndef V
	QUIET := @
endif

SOURCES := $(notdir $(wildcard  $(addsuffix /*.c, $(MAKEFILE_DIR))))
SOURCES := $(filter-out *.mod.c, $(SOURCES))
$(TARGET)-objs := $(SOURCES:.c=.o) 

export GITREVFILE := $(PWD)/.git.h

define gitrev
HASH=`git -C "$MAKEFILE_DIR" log -n 1 --format="%h" 2>/dev/null`; \
STAR=`git -C "$MAKEFILE_DIR"  status --porcelain 2>/dev/null`; [ -n "$STAR" ] && STAR="*"; \
CNT=`git -C "$MAKEFILE_DIR" rev-list --count HEAD  2>/dev/null`; \
BRANCH=`git -C "$MAKEFILE_DIR" rev-parse --abbrev-ref HEAD 2>/dev/null`; \
DATE=`git -C "$MAKEFILE_DIR" log -n 1 --format="%cd" --date="format:%Y-%m-%d %H:%M" 2>/dev/null`; \
REV="r$CNT-$HASH$STAR $BRANCH $DATE"; \
echo "GIT $REV"; \
echo "#ifndef GIT_REVISION \n#define GIT_REVISION \"$REV\" \n#endif" > $GITREVFILE;
endef

ccflags-y += -DDEVNAME=$(DEVNAME) -Wno-date-time
ccflags-y += -DDEBUG
obj-m      += $(TARGET).o

INCLUDES := $(wildcard   $(addsuffix /*.h, $(MAKEFILE_DIR)) )


ifneq ($(wildcard $(MAKEFILE_DIR)/dts/.),)
all: install dts
else
#$(info DTS dir [$(MAKEFILE_DIR)/dts] does not exists)
all: install
endif

.PHONY: dis
dis: $(TARGET).ko
	@echo "Disassemble create..." # -DSx
	$(QUIET) $(SDK_PREFIX) $$OBJDUMP  -Sx $(TARGET)-unstripped.ko  > .$(TARGET).dis

install: $(TARGET).ko $(INSTALL_DIR)
	$(QUIET) echo "Install $(TARGET).ko to [$(INSTALL_DIR)] ..." 
	$(QUIET) cp -f $(TARGET).ko $(INSTALL_DIR)

$(TARGET).ko: $(GITREVFILE) $(THIS_MAKEFILE) $(SOURCES)  $(INCLUDES)
	$(QUIET) $(SDK_PREFIX) \
       echo CROSS_COMPILE: $$CROSS_COMPILE ; \
       $(MAKE) -j$(shell nproc) -C $(KDIR) M=$(PWD) $@
	$(QUIET) mv $@ $(TARGET)-unstripped.ko
	$(QUIET) $(SDK_PREFIX) $$STRIP $(TARGET)-unstripped.ko --strip-unneeded  -o $@ 

.PHONY: git
git:
	$(QUIET) rm -f $(GITREVFILE)
	$(QUIET) $(MAKE) -C $(MAKEFILE_DIR) $(GITREVFILE)

$(GITREVFILE):
	$(QUIET) $(value gitrev)

$(INSTALL_DIR):
	$(QUIET) echo "Creating install directory: [$@]..."
	$(QUIET) mkdir -p $@

clean:
	$(QUIET) rm -f  $(TARGET).ko $(TARGET)-unstripped.ko *.o .*.o.* .*.cmd *.mod.c *.mod *.*~ *.symvers modules.order; 
	$(QUIET) rm -f -r .tmp_versions $(GITREVFILE)
ifneq ($(wildcard $(MAKEFILE_DIR)/dts/.),)
	$(QUIET) $(MAKE) -C $(MAKEFILE_DIR)/dts O=$(MAKEFILE_DIR)/dts clean

.PHONY: dts
dts:
	$(QUIET) $(MAKE) -C $(MAKEFILE_DIR)/dts O=$(MAKEFILE_DIR)/dts
endif

