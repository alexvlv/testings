# $Id$
# $HeadURL: http://server/svn/207_ForeMost/ForeMost-AT/trunk/Firmware/ARM_TCPIP/drv/dmap/Makefile $
# $LastChangedRevision: 1280 $
# $LastChangedDate: 2016-07-08 11:53:36 +0300 (Пт, 08 июл 2016) $
# $LastChangedBy: alexvol $

THIS_MAKEFILE := $(lastword $(MAKEFILE_LIST))
export MAKEFILE_DIR := $(realpath $(dir $(THIS_MAKEFILE)))

export CROSS_COMPILE=aarch64-linux-gnu-
export ARCH=arm64

KBUILD_CFLAGS += -Wno-date-time

TARGET := $(notdir $(patsubst %/,%,$(MAKEFILE_DIR)))

override KDIR_LNK="$(MAKEFILE_DIR)/.kdir"
export KDIR  = $(shell readlink -f $(KDIR_LNK) 2>/dev/null)

MAKEFLAGS += --no-print-directory
# build quietly by default.  For a verbose build, run "make V=1"
ifndef V
	QUIET := @
endif
SRC := $(TARGET)-main

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

target: $(GITREVFILE) $(THIS_MAKEFILE) $(addsuffix .c, $(SRC) )
	$(QUIET) $(MAKE) -j$(shell nproc) -C $(KDIR) M=$(PWD) $(TARGET).ko

.PHONY: git
git:
	$(QUIET) rm -f $(GITREVFILE)
	$(QUIET) $(MAKE) -C $(MAKEFILE_DIR) $(GITREVFILE)

$(GITREVFILE):
	$(QUIET) $(value gitrev)

	
clean:
	$(QUIET) rm -f  $(TARGET).ko *.o .*.cmd *.mod.c *.mod *.*~ *.symvers modules.order; 
	$(QUIET) rm -f -r .tmp_versions $(GITREVFILE)
  
