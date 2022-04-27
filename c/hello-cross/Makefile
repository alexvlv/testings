# $Id$

ARCH := aarch64

export MAKEFILE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
PROJECT := $(notdir $(patsubst %/,%,$(MAKEFILE_DIR)))

SUFFIX := -$(ARCH)
ifeq ($(ARCH),aarch64)
	COMMAND_PREFIX := aarch64-linux-gnu-
else	
	EXT := $(SUFFIX)
endif
ifeq ($(ARCH),)
	$(error ARCH not set)
endif

O := $(PWD)
export BUILDDIR = $(O)

BINARY := $(BUILDDIR)/$(PROJECT)$(EXT)

# Where to copy the resulting executables
override INSTALL_LNK="$(MAKEFILE_DIR)/.install-$(ARCH)"
INSTALLDIR = $(shell readlink -f $(INSTALL_LNK) 2>/dev/null)

# build quietly by default.  For a verbose build, run "make V=1"
ifndef V
	QUIET := @
endif
MAKEFLAGS += --no-print-directory

export CPP := $(COMMAND_PREFIX)g++
export CC := $(COMMAND_PREFIX)gcc
export AR := $(COMMAND_PREFIX)ar
export LD := $(COMMAND_PREFIX)ld
export STRIP := $(COMMAND_PREFIX)strip

source_dirs := $(MAKEFILE_DIR) 
include_dirs := $(BUILDDIR) $(source_dirs) 

obj_dir := $(BUILDDIR)/obj$(SUFFIX)

CFLAGS:= -pipe -Wall -ggdb

#LIBS=-lpthread -lftd2xx
#LDFLAGS := -lstdc++fs
#LDFLAGS += -L$(STAGING_DIR)/target-mips_24kc_musl/usr/lib

SOURCES := $(wildcard   $(addsuffix /*.cpp, $(source_dirs) ) )
SOURCES += $(wildcard   $(addsuffix /*.c, $(source_dirs) ) )
SOURCES += $(wildcard   $(addsuffix /*.bin, $(source_dirs) ) )
OBJECTS := $(SOURCES:.c=.o)  
OBJECTS := $(OBJECTS:.cpp=.o)  
OBJECTS := $(OBJECTS:.bin=.o)  
OBJECTS := $(notdir $(OBJECTS) )
OBJECTS := $(addprefix $(obj_dir)/,$(OBJECTS))
VPATH := $(source_dirs) $(include_dirs) 

export GITREVFILE := $(BUILDDIR)/.git.h

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


default: aarch64

.PHONY: aarch64
aarch64:
	$(QUIET) $(MAKE) -C $(MAKEFILE_DIR) O=$(BUILDDIR) install ARCH=aarch64

.PHONY: x86
x86:
	$(QUIET) $(MAKE) -f $(MAKEFILE_DIR)/Makefile O=$(BUILDDIR) install ARCH=x86

.PHONY: clean$(SUFFIX)
clean-$(SUFFIX):
	$(MAKE) clean ARCH=$(ARCH)

.PHONY: install
install: $(BINARY)
	@echo Install $^ binaries to $(INSTALLDIR)
	$(QUIET) install -d $(INSTALLDIR); install $^ $(INSTALLDIR)

$(BINARY): $(BINARY)-unstripped 
	@echo Strip `basename $@` from $^ ...
	$(QUIET) $(STRIP) -o $@ $^

$(BINARY)-unstripped: git $(obj_dir) $(OBJECTS)
	@echo Linking `basename $@` from $^...
	$(QUIET)  $(CPP) $(OBJECTS)  $(LDFLAGS) $(LIBS) -o $@ 
	
.PHONY: git
git:
	$(QUIET) rm -f $(BUILDDIR)/.git.h
	$(QUIET) $(MAKE) -C $(MAKEFILE_DIR) O=$(BUILDDIR) .git.h

.git.h:
	$(QUIET) $(value gitrev)

.PHONY: clean
clean:
	@echo Clean all...
	$(QUIET) rm -f $(BUILDDIR)/$(PROJECT) $(BUILDDIR)/$(PROJECT)-* $(BUILDDIR)/.git.h; rm -f -r $(BUILDDIR)/obj-*;

$(obj_dir):
	$(QUIET)  mkdir $(obj_dir)

include $(wildcard $(obj_dir)/*.d) 

$(obj_dir)/%.o:%.c 
	@echo Compiling  $*.o from $<..
	$(QUIET) $(CC) $(CFLAGS)  -c -MMD $(addprefix -I,$(include_dirs)) $<  -o $@

$(obj_dir)/%.o:%.cpp
	@echo Compiling $*.o from $<..
	$(QUIET) $(CPP) $(CFLAGS)  -c -MMD $(addprefix -I,$(include_dirs)) $<  -o $@

$(obj_dir)/%.o:%.bin 
	@echo Create blob object $*.o from $<..
	$(QUIET) $(LD) -r -b binary $< -o $@

