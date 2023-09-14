# $Id$

THIS_MAKEFILE := $(lastword $(MAKEFILE_LIST))
export MAKEFILE_DIR := $(realpath $(dir $(THIS_MAKEFILE)))

TARGET := $(notdir $(patsubst %/,%,$(MAKEFILE_DIR)))

KERNEL_LNK="$(MAKEFILE_DIR)/../.kdir/source"
KERNEL_SRC = $(shell readlink -f $(KERNEL_LNK) 2>/dev/null)

INSTALL_LNK="$(MAKEFILE_DIR)/.install"
INSTALL_DIR  = $(shell readlink -f $(INSTALL_LNK) 2>/dev/null)

MAKEFLAGS += --no-print-directory
# build quietly by default.  For a verbose build, run "make V=1"
ifndef V
	QUIET := @
endif

source_dirs := $(MAKEFILE_DIR)

include_dirs := $(MAKEFILE_DIR)/include
include_dirs += $(KERNEL_SRC)/include
include_dirs += $(KERNEL_SRC)/arch/arm64/boot/dts/freescale

SOURCES := $(wildcard   $(addsuffix /*.dts, $(source_dirs)) )
TARGETS := $(SOURCES:.dts=.dtb)  

export CPP := $(COMMAND_PREFIX)cpp
export DTC := $(COMMAND_PREFIX)dtc

CPPLAGS:= -nostdinc -undef -x assembler-with-cpp

DTCLAGS := -Wno-avoid_unnecessary_addr_size \
-Wno-simple_bus_reg -Wno-unit_address_vs_reg \
-Wno-unique_unit_address -Wno-graph_child_address

O := $(PWD)
export BUILDDIR = $(O)

obj_dir := $(BUILDDIR)/obj
REVERSES := $(addprefix $(obj_dir)/,$(SOURCES))

install: $(obj_dir) $(TARGETS) $(INSTALL_DIR)
	$(QUIET) echo "Install $(TARGETS) to [$(INSTALL_DIR)] ..." 
	$(QUIET) cp -f $(TARGETS) $(INSTALL_DIR)

.PHONY: reverse
reverse: $(obj_dir) $(REVERSES)

$(obj_dir)/%.dts:%.dtb
	@echo Decompiling $@ from $< ...
	$(QUIET) $(DTC) -Idtb -Odts $(DTCLAGS) $< -o $@

%.dtb:%.dts $(obj_dir) $(shell find $(include_dirs) -type f)
	@echo Compiling $@ from $< ...
	$(QUIET) $(CPP) $(CPPLAGS) $(addprefix -I,$(include_dirs))  $< > $(obj_dir)/$(notdir $*).pre
	$(QUIET) $(DTC) -Idts -Odtb $(DTCLAGS) $(obj_dir)/$(notdir $*).pre -o $@

$(INSTALL_DIR):
	$(QUIET) echo "Creating install directory: [$@]..."
	$(QUIET) mkdir -p $@

$(obj_dir):
	$(QUIET)  mkdir $(obj_dir)
	
clean:
	$(QUIET) rm -f  $(TARGETS)
	$(QUIET) rm -f -r $(obj_dir)
