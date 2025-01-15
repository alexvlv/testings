# $Id$

THIS_MAKEFILE := $(lastword $(MAKEFILE_LIST))
export MAKEFILE_DIR := $(realpath $(dir $(THIS_MAKEFILE)))

empty:=
space:= $(empty) $(empty)
$(if $(findstring $(space),$(MAKEFILE_DIR)),$(error ERROR: The path must not include any spaces))

override RELPATH = ..
-include $(MAKEFILE_DIR)/$(RELPATH)/.local.mk
include $(MAKEFILE_DIR)/$(RELPATH)/kpath.mk

override INSTALL_LNK=$(MAKEFILE_DIR)/.install
ifneq (,$(wildcard $(INSTALL_LNK)))
DTB_INSTALL_DIR ?= $(shell readlink -f $(INSTALL_LNK) 2>/dev/null)
endif

override OVERLAYS_LNK=$(MAKEFILE_DIR)/.overlays
ifneq (,$(wildcard $(OVERLAYS_LNK)))
OVERLAYS_DIR ?= $(shell readlink -f $(OVERLAYS_LNK) 2>/dev/null)
endif

MAKEFLAGS += --no-print-directory
# build quietly by default.  For a verbose build, run "make V=1"
ifndef V
	QUIET := @
endif

source_dirs := $(MAKEFILE_DIR)
include_dirs := $(MAKEFILE_DIR)/include
include_dirs += $(O)/source/include
include_dirs += $(O)/source/arch/arm64/boot/dts/freescale

SOURCES :=  $(wildcard   $(addsuffix /*.dts, $(source_dirs)))
TARGETS :=  $(SOURCES:.dts=.dtb)
SOURCES := $(wildcard   $(addsuffix /*.dtso, $(source_dirs)) )
OVERLAYS := $(SOURCES:.dtso=.dtbo)

DTC_PREFIX ?= $(O)/scripts/dtc/

CPP := $(COMMAND_PREFIX)cpp
DTC := $(DTC_PREFIX)dtc

CPPLAGS:= -nostdinc -undef -x assembler-with-cpp

DTCLAGS := -Wno-avoid_unnecessary_addr_size \
-Wno-simple_bus_reg -Wno-unit_address_vs_reg \
-Wno-unique_unit_address -Wno-graph_child_address

O := $(PWD)
export BUILDDIR = $(O)

obj_dir := $(BUILDDIR)/obj
REVERSES := $(addprefix $(obj_dir)/,$(SOURCES))

all: install
.PHONY: install
install: install_dtbs install_overlays

ifneq (,$(DTB_INSTALL_DIR))
install_dtbs: $(obj_dir) $(TARGETS) $(OVERLAYS) $(DTB_INSTALL_DIR)
	$(QUIET) echo "Install $(notdir $(TARGETS)) to [$(DTB_INSTALL_DIR)] ..." 
	$(QUIET) cp -f $(TARGETS) $(DTB_INSTALL_DIR)
else
install_dtbs: $(obj_dir) $(TARGETS)
endif

ifneq (,$(OVERLAYS_DIR))
install_overlays: $(obj_dir) $(OVERLAYS) $(OVERLAYS_DIR)
	$(QUIET) echo "Install overlays $(notdir $(OVERLAYS)) to [$(OVERLAYS_DIR)] ..." 
	$(QUIET) cp -f $(OVERLAYS) $(OVERLAYS_DIR)
else
install_overlays: $(obj_dir) $(OVERLAYS)
endif

.PHONY: reverse
reverse: $(obj_dir) $(REVERSES)

$(obj_dir)/%.dts:%.dtb
	@echo Decompiling $@ from $< ...
	$(QUIET) $(DTC) -Idtb -Odts $(DTCLAGS) $< -o $@

%.dtb:%.dts $(obj_dir) $(shell find $(include_dirs) -type f)
	@echo Compiling $(notdir $@) from $(notdir $<) ...
	$(QUIET) $(CPP) $(CPPLAGS) $(addprefix -I,$(include_dirs))  $< > $(obj_dir)/$(notdir $*).pre
	$(QUIET) $(DTC) -@ -Idts -Odtb $(DTCLAGS) $(obj_dir)/$(notdir $*).pre -o $@

%.dtbo:%.dtso $(obj_dir)
	@echo Compiling overlay $(notdir $@) from $(notdir $<) ...
	$(QUIET) $(CPP) $(CPPLAGS) $(addprefix -I,$(include_dirs))  $< > $(obj_dir)/$(notdir $*).pre
	$(QUIET) $(DTC) -@ -Idts -Odtb $(DTCLAGS) $(obj_dir)/$(notdir $*).pre -o $@

$(INSTALL_DIR):
	$(QUIET) echo "Creating install directory: [$@]..."
	$(QUIET) mkdir -p $@

$(OVERLAYS_DIR):
	$(QUIET) echo "Creating overlays directory: [$@]..."
	$(QUIET) mkdir -p $@

$(obj_dir):
	$(QUIET)  mkdir $(obj_dir)
	
clean:
	$(QUIET) rm -f  *.dtb*
	$(QUIET) rm -f -r $(obj_dir)
  
