# $Id$

THIS_MAKEFILE := $(lastword $(MAKEFILE_LIST))
export MAKEFILE_DIR := $(realpath $(dir $(THIS_MAKEFILE)))

empty:=
space:= $(empty) $(empty)
$(if $(findstring $(space),$(MAKEFILE_DIR)),$(error ERROR: The path must not include any spaces))

-include $(MAKEFILE_DIR)/.local.mk
-include $(MAKEFILE_DIR)/mk/rules.mk
include $(MAKEFILE_DIR)/mk/kpath.mk
include $(MAKEFILE_DIR)/mk/git.mk

TARGET ?= $(notdir $(patsubst %/,%,$(MAKEFILE_DIR)))
DEVNAME ?= $(subst -,,$(TARGET))
export INSTALL_MOD_DIR ?= extra 

MAKEFLAGS += --no-print-directory
# build quietly by default.  For a verbose build, run "make V=1"
ifndef V
	QUIET := @
endif

SOURCES := $(notdir $(wildcard  $(addsuffix /*.c, $(MAKEFILE_DIR))))
SOURCES := $(filter-out *.mod.c, $(SOURCES))
$(TARGET)-objs := $(SOURCES:.c=.o) 

export GITREVFILE := $(PWD)/.git.h

ccflags-y += -DDEVNAME=$(DEVNAME) -Wno-date-time -Wno-error
ccflags-y += -DDEBUG
obj-m += $(TARGET).o

INCLUDES := $(wildcard   $(addsuffix /*.h, $(MAKEFILE_DIR)) )

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
INSTALL_FULL_PATH ?= $(INSTALL_MOD_PATH)/lib/modules/$(KERNELRELEASE)/$(INSTALL_MOD_DIR)
install: $(TARGET).ko
	$(QUIET) echo "Install $(TARGET).ko to [$(INSTALL_FULL_PATH)] ..." 
	$(QUIET) install -d -m 0755 ${INSTALL_FULL_PATH}
	$(QUIET) install -t ${INSTALL_FULL_PATH} -m 0644 $(TARGET).ko
endif

modules_install: $(TARGET).ko
	$(MAKE) -C $(O) M=$(PWD) modules_install

.PHONY: rebuild
rebuild:
	$(MAKE) clean
	$(MAKE) all

.PHONY: dis
dis: $(TARGET).ko
	@echo "Disassemble create..." # -DSx
	$(QUIET) $(SDK_PREFIX) $$OBJDUMP  -Sx $(TARGET)-unstripped.ko  > .$(TARGET).dis

.PHONY: mod
mod: $(TARGET).ko
# modules
$(TARGET).ko: $(GITREVFILE)  $(THIS_MAKEFILE) $(SOURCES) $(INCLUDES)
	$(QUIET) echo KERNELRELEASE: $(KERNELRELEASE);
	$(QUIET) echo CROSS_COMPILE: $${CROSS_COMPILE};
	$(QUIET) $(MAKE) -j$(shell nproc) -C $(O) M=$(PWD) $@
	$(QUIET) mv $(TARGET).ko $(TARGET)-unstripped.ko
	$(QUIET) $${CROSS_COMPILE}strip --strip-unneeded  $(TARGET)-unstripped.ko -o $(TARGET).ko

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

