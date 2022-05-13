# $Id$

THIS_MAKEFILE := $(lastword $(MAKEFILE_LIST))
export MAKEFILE_DIR := $(realpath $(dir $(THIS_MAKEFILE)))
PROJECT := $(notdir $(patsubst %/,%,$(MAKEFILE_DIR)))

.PHONY: test
test:
	@echo MAKEFILE_DIR: [$(MAKEFILE_DIR)] PROJECT: [$(PROJECT)]

#  Based on kernel main Makefile
#this-makefile := $(lastword $(MAKEFILE_LIST))
#abs_srctree := $(realpath $(dir $(this-makefile)))
