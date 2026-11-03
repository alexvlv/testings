ARCHES := arm64 aarch64 x86
DEFAULT_ARCH := aarch64

export MAKEFILE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
PROJECT := $(notdir $(patsubst %/,%,$(MAKEFILE_DIR)))

empty:=
space:= $(empty) $(empty)
$(if $(findstring $(space),$(MAKEFILE_DIR)),$(error ERROR: The path must not include any spaces))

SUFFIX := -$(ARCH)

ifneq ($(ARCH),arm64)
  EXT := $(SUFFIX)
endif

ifeq ($(ARCH),aarch64)
  COMMAND_PREFIX := aarch64-linux-gnu-
  export CXX = $(COMMAND_PREFIX)g++
  export CC = $(COMMAND_PREFIX)gcc
  export AR = $(COMMAND_PREFIX)ar
  export LD = $(COMMAND_PREFIX)ld
  export STRIP = $(COMMAND_PREFIX)strip
else
  export STRIP ?= strip
endif

O := $(PWD)
export BUILDDIR = $(O)

BINARY := $(BUILDDIR)/$(PROJECT)$(EXT)

# Where to copy the resulting executables
override INSTALL_LNK="$(MAKEFILE_DIR)/.install$(EXT)"
INSTALLDIR = $(shell readlink -f $(INSTALL_LNK) 2>/dev/null)

# build quietly by default.  For a verbose build, run "make V=1"
ifndef V
	QUIET := @
endif
MAKEFLAGS += --no-print-directory

source_dirs := $(MAKEFILE_DIR)
include_dirs := $(BUILDDIR) $(source_dirs)

obj_dir := $(BUILDDIR)/obj$(SUFFIX)

CFLAGS += -pipe -Wall -ggdb

LIBS = -lpthread
#LDFLAGS := -lstdc++fs
#LDFLAGS += -L$(LIBFTDI_DIR)/lib

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
HASH=`git -C "$1" log -n 1 --format="%h" 2>/dev/null`; \
STAR=`git -C "$1" status --porcelain 2>/dev/null`; [ -n "$$STAR" ] && STAR="*"; \
CNT=`git -C "$1" rev-list HEAD 2>/dev/null | wc -l | awk '{print $$1}'`; \
BRANCH=`git -C "$1" rev-parse --abbrev-ref HEAD 2>/dev/null`; \
DATE=`git -C "$1" log -n 1 --format="%cd" --date="format:%Y-%m-%d %H:%M" 2>/dev/null`; \
REV="r$$CNT-$$HASH$$STAR $$BRANCH $$DATE"; \
echo "GIT $$REV"
endef

.PHONY: $(ARCHES) $(ARCHES:%=%/%)
$(ARCHES:%=%/%):
	$(QUIET) $(MAKE) -C $(MAKEFILE_DIR) \
		O=$(BUILDDIR) \
		$(word 2,$(subst /, ,$@)) \
		ARCH=$(word 1,$(subst /, ,$@))

# ARCH
ifeq ($(ARCH),)
$(ARCHES):
	$(QUIET) $(MAKE) $@/all
%:
	$(QUIET) $(MAKE) $(DEFAULT_ARCH)/$@
else
all: install

.PHONY: install
install: $(BINARY)
	@echo Install $^ binaries to $(INSTALLDIR)
	$(QUIET) install -Dm755 $^ -t $(INSTALLDIR)

.PHONY: strip
strip: $(BINARY)

.PHONY: clean
clean:
	$(QUIET) echo Clean $(ARCH) ...
	$(QUIET) rm -f $(BUILDDIR)/$(BINARY) $(BUILDDIR)/$(PROJECT)-* $(BUILDDIR)/.git.h; rm -f -r $(BUILDDIR)/obj-$(ARCH);

endif # ARCH

$(BINARY): $(BINARY)-unstripped
	@echo Strip `basename $@` from $^ ...
	$(QUIET) $(STRIP) -o $@ $^

$(BINARY)-unstripped: $(BUILDDIR)/.git.h $(obj_dir) $(OBJECTS)
	@echo Linking $(ARCH) `basename $@` from $^...
	$(QUIET) $(CC) $(OBJECTS)  $(LDFLAGS) $(LIBS) -o $@

.PHONY: git git_clean
git: git_clean $(BUILDDIR)/.git.h

git_clean:
	$(QUIET) rm -f $(BUILDDIR)/.git.h

$(BUILDDIR)/.git.h:
	$(eval GIT_REV := $(shell $(call gitrev,$(MAKEFILE_DIR))))
	$(info $(GIT_REV))
	$(file >$@,#ifndef GIT_REVISION)
	$(file >>$@,#define GIT_REVISION "$(GIT_REV)")
	$(file >>$@,#endif)

.PHONY: distclean
distclean:
	@echo Clean all...
	$(QUIET) rm -f $(BUILDDIR)/$(PROJECT) $(BUILDDIR)/$(PROJECT)-* $(BUILDDIR)/.git.h; rm -f -r $(BUILDDIR)/obj-*;

$(obj_dir):
	$(QUIET)  mkdir $(obj_dir)

include $(wildcard $(obj_dir)/*.d)

$(obj_dir)/%.o:%.c
	@echo Compiling  $*.o from $< ...
	$(QUIET) $(CC) $(CFLAGS)  -c -MMD $(addprefix -I,$(include_dirs)) $< -o $@

$(obj_dir)/%.o:%.cpp
	@echo Compiling $*.o from $< ...
	$(QUIET) $(CXX) $(CFLAGS)  -c -MMD $(addprefix -I,$(include_dirs)) $< -o $@

$(obj_dir)/%.o:%.bin
	@echo Create blob object $*.o from $< ...
	$(QUIET) $(LD) -r -b binary $< -o $@
