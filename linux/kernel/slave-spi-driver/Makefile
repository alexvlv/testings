# $Id$

THIS_MAKEFILE := $(lastword $(MAKEFILE_LIST))
export MAKEFILE_DIR := $(realpath $(dir $(THIS_MAKEFILE)))

export CROSS_COMPILE=aarch64-linux-gnu-
export ARCH=arm64

KBUILD_CFLAGS += -Wno-date-time

#TARGET := $(notdir $(patsubst %/,%,$(MAKEFILE_DIR)))
TARGET := spi-slave

override KDIR_LNK="$(MAKEFILE_DIR)/.kdir"
export KDIR  = $(shell readlink -f $(KDIR_LNK) 2>/dev/null)

# /lib/modules/$(KERNELRELEASE)/extra
# /lib/modules/$(shell uname -r)/extra
KERNELRELEASE_FILE = $(KDIR)/include/config/kernel.release
ifeq ("$(wildcard $(KERNELRELEASE_FILE))","")
$(warning File [$(KERNELRELEASE_FILE)] does not exists)
endif
KERNELRELEASE = $(shell cat $(KERNELRELEASE_FILE) 2>/dev/null)

override INSTALL_LNK="$(MAKEFILE_DIR)/.install"
INSTALL_ROOT_DIR = $(shell readlink -f $(INSTALL_LNK) 2>/dev/null)
INSTALL_DIR  = $(INSTALL_ROOT_DIR)/lib/modules/$(KERNELRELEASE)/extra/

MAKEFLAGS += --no-print-directory
# build quietly by default.  For a verbose build, run "make V=1"
ifndef V
	QUIET := @
endif
SRC :=  $(TARGET)-main $(TARGET)-hw

export GITREVFILE := $(PWD)/.git.h

define gitrev
HASH=`git -C "$MAKEFILE_DIR" log -n 1 --format="%h" 2>/dev/null`; \
STAR=`git -C "$MAKEFILE_DIR"  status --porcelain 2>/dev/null`; [ -n "$STAR" ] && STAR="*"; \
CNT=`git -C "$MAKEFILE_DIR" rev-list HEAD  2>/dev/null | wc -l | awk '{print $1}'`; \
BRANCH=`git -C "$MAKEFILE_DIR" rev-parse --abbrev-ref HEAD 2>/dev/null`; \
DATE=`git -C "$MAKEFILE_DIR" log -n 1 --format="%cd" --date="format:%Y-%m-%d %H:%M" 2>/dev/null`; \
REV="r$CNT-$HASH$STAR $BRANCH $DATE"; \
echo "GIT $REV"; \
echo "#ifndef VERSION \n#define VERSION \"$REV\" \n#endif" > $GITREVFILE;
endef

obj-m      += $(TARGET).o
$(TARGET)-objs := $(addsuffix .o, $(SRC) )


INCLUDES := $(wildcard   $(addsuffix /*.h, $(PWD)) )
SOURCES := $(addsuffix .c, $(SRC) )

install: $(TARGET).ko $(INSTALL_DIR)
	$(QUIET) echo "Install $(TARGET).ko to [$(INSTALL_DIR)] ..." 
	$(QUIET) cp -f $(TARGET).ko $(INSTALL_DIR)

$(TARGET).ko: $(GITREVFILE) $(THIS_MAKEFILE) $(SOURCES)  $(INCLUDES)
	$(QUIET) $(MAKE) -j$(shell nproc) -C $(KDIR) M=$(PWD) $(TARGET).ko

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
	$(QUIET) rm -f  $(TARGET).ko *.o .*.cmd *.mod.c *.mod *.*~ *.symvers modules.order; 
	$(QUIET) rm -f -r .tmp_versions $(GITREVFILE)
