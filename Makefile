#
#==============================================================================
#                           COPYRIGHT NOTICE
#
#   Copyright (C) 2002-2006 GE Fanuc
#   International Copyright Secured.  All Rights Reserved.
#
#------------------------------------------------------------------------------
#   Description: Top level Makefile for the GE Fanuc VMEbus driver driver
#==============================================================================
#

# ARCH := ppc

# Project variables
# 'uname -r' sets kernel revision to the currently running kernel revision
# KERNELSRC must point to the Linux kernel source code.

MOD_MAJOR:=3
MOD_MINOR:=6

MOD_VERSION:=$(MOD_MAJOR).$(MOD_MINOR)$(if $(MOD_PATCH),.)$(MOD_PATCH)
KERNELREV:=$(shell uname -r)
PROJNAME:=vme_universe
PATCHLEVEL:=$(shell uname -r | cut -d. -f2)
VMIINCPATH:=$(shell pwd)/include/
VMILIBPATH:=$(shell pwd)/lib
DEVDIR:=/dev/bus/vme

ifeq ($(ARCH), ppc)
KERNELSRC:=/home/7050/linux-2.4.26
MOD_MAKE:=MakefilePPC
else
KERNELSRC:=/lib/modules/$(KERNELREV)/build
ifeq ($(PATCHLEVEL), 4)
MOD_MAKE:=Makefile2.4
else
MOD_MAKE:=Makefile
endif
endif

ifeq ($(ARCH), ppc)
export ARCH
endif
export MOD_MAJOR
export MOD_MINOR
export MOD_VERSION
export MOD_MAKE
export VMIINCPATH
export VMILIBPATH
export DEVDIR
export PROJNAME

.PHONY: all install uninstall clean

all: $(KERNELSRC)/include/linux/module.h
	$(MAKE) -C module -f $(MOD_MAKE) all; 
	$(MAKE) -C lib    -f $(MOD_MAKE) all;
	$(MAKE) -C test   -f $(MOD_MAKE) all; 
	$(MAKE) -C TOOLS  -f $(MOD_MAKE) all;
ifeq ($(ARCH), ppc)
	$(MAKE) -C rtp    -f $(MOD_MAKE) all; 
endif

install: $(KERNELSRC)/include/linux/module.h
	$(MAKE) -C module -f $(MOD_MAKE) install; 
	$(MAKE) -C lib    -f $(MOD_MAKE) install;
	$(MAKE) -C test   -f $(MOD_MAKE) all; 
	$(MAKE) -C TOOLS  -f $(MOD_MAKE) all;
ifeq ($(ARCH), ppc)
	$(MAKE) -C rtp    -f $(MOD_MAKE) all; 
endif

uninstall: $(KERNELSRC)/include/linux/module.h
	$(MAKE) -C module -f $(MOD_MAKE) uninstall; 
	$(MAKE) -C lib    -f $(MOD_MAKE) clean;
	$(MAKE) -C test   -f $(MOD_MAKE) clean; 
	$(MAKE) -C TOOLS  -f $(MOD_MAKE) clean;
ifeq ($(ARCH), ppc)
	$(MAKE) -C rtp    -f $(MOD_MAKE) clean; 
endif

clean: $(KERNELSRC)/include/linux/module.h
	$(MAKE) -C module -f $(MOD_MAKE) clean; 
	$(MAKE) -C lib    -f $(MOD_MAKE) clean;
	$(MAKE) -C test   -f $(MOD_MAKE) clean; 
	$(MAKE) -C TOOLS  -f $(MOD_MAKE) clean;
ifeq ($(ARCH), ppc)
	$(MAKE) -C rtp    -f $(MOD_MAKE) clean; 
endif

$(KERNELSRC)/include/linux/module.h:
	$(error It appears that your kernel source is not installed. \
                To build this module, the kernel source must be accessible \
                via the path $(KERNELSRC).)

