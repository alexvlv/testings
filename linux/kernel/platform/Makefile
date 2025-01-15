# $Id$

THIS_MAKEFILE := $(lastword $(MAKEFILE_LIST))
export MAKEFILE_DIR := $(realpath $(dir $(THIS_MAKEFILE)))

empty:=
space:= $(empty) $(empty)
$(if $(findstring $(space),$(MAKEFILE_DIR)),$(error ERROR: The path must not include any spaces))

-include $(MAKEFILE_DIR)/.local.mk
-include $(MAKEFILE_DIR)/rules.mk
include $(MAKEFILE_DIR)/kpath.mk

TARGET ?= $(notdir $(patsubst %/,%,$(MAKEFILE_DIR)))
DEVNAME ?= $(subst -,,$(TARGET))
export INSTALL_MOD_DIR ?= extra

MAKEFLAGS += --no-print-directory
# build quietly by default.  For a verbose build, run "make V=1"
ifndef V
  QUIET := @
endif

#$(info O=[$(O)])

SOURCES := $(notdir $(wildcard  $(addsuffix /*.c, $(MAKEFILE_DIR))))
SOURCES := $(filter-out *.mod.c, $(SOURCES))
$(TARGET)-objs := $(SOURCES:.c=.o) 

INCLUDES := $(notdir $(wildcard  $(addsuffix /*.h, $(MAKEFILE_DIR))))

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
obj-m     += $(TARGET).o

override INSTALL_LNK=$(MAKEFILE_DIR)/.install
ifneq (,$(wildcard $(INSTALL_LNK)))
INSTALL_DIR ?= $(shell readlink -f $(INSTALL_LNK) 2>/dev/null)
endif
export INSTALL_MOD_PATH ?= $(INSTALL_DIR)

ifeq (,$(INSTALL_DIR))
all: $(TARGET).ko
else
all: install
#$(MAKE) -C $(O) M=$(PWD) modules_install
INSTALL_FULL_PATH := $(INSTALL_MOD_PATH)/lib/modules/$(KERNELRELEASE)/$(INSTALL_MOD_DIR)
install: $(TARGET).ko
	$(QUIET) echo "Install $(TARGET).ko to [$(INSTALL_FULL_PATH)] ..." 
	$(QUIET) mkdir -p  $(INSTALL_FULL_PATH)
	$(QUIET) cp -f $(TARGET).ko $(INSTALL_FULL_PATH)
endif

.PHONY: mod
mod: $(TARGET).ko

$(TARGET).ko: $(GITREVFILE)  $(THIS_MAKEFILE) $(SOURCES) $(INCLUDES)
	$(QUIET) echo KERNELRELEASE: $(KERNELRELEASE);
	$(QUIET) $(SDK_PREFIX) echo CROSS_COMPILE: $${CROSS_COMPILE};
	$(QUIET) $(SDK_PREFIX) $(MAKE) $(NPROC) -C $(O) M=$(PWD) $(TARGET).ko
	$(QUIET) cp -f $(TARGET).ko $(TARGET)-unstripped.ko
	$(QUIET) $(SDK_PREFIX) $${CROSS_COMPILE}strip --strip-unneeded $(TARGET).ko

modules_install: $(TARGET).ko
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules_install

$(GITREVFILE):
	$(QUIET) $(value gitrev)

.PHONY: dtbs
dtbs:
	$(MAKE) -C dts

.PHONY: clean
clean:
	$(QUIET) rm -f *.o *~ core .depend .*.cmd *.ko *.mod.c *.mod
	$(QUIET) rm -f Module.markers Module.symvers modules.order
	$(QUIET) rm -rf .tmp_versions Modules.symvers
	$(QUIET) rm -f $(GITREVFILE)
