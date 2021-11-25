#
#==============================================================================
#                           COPYRIGHT NOTICE
#
#   Copyright (C) 2002-2006 GE Fanuc
#   International Copyright Secured.  All Rights Reserved.
#
#------------------------------------------------------------------------------
#   Description: Makefile for the GE Fanuc VMEbus driver Utilities
#------------------------------------------------------------------------------
#
#==============================================================================
#

# Project variables
# 'uname -r' sets kernel revision to the currently running kernel revision
# KERNELSRC must point to the Linux kernel source code.
KERNELREV:=$(shell uname -r)
KERNELSRC:=/lib/modules/$(KERNELREV)/build
LIBNAME:=libvme.so

# The standard tools and options
CC:=gcc
CFLAGS=-I../include -Wall -I/lib/modules/$(KERNELREV)/build/include
LD:=ld
LDFLAGS=-Llib -lvme

%.o: %.c
	$(CC) -c -o $(LDFLAGS) $@ $<

.PHONY: all clean

UTILITIES := 	catch_berr \
		dma_read \
		get_lm \
		get_mailbox \
		hello_vme \
		peek \
		peek_slv32 \
		poke \
		slave_peek \
		slv_create16 \
		slv_create32 \
		slv_create

all: $(UTILITIES)

$(UTILITIES):CFLAGS+=-O2

clean:
	rm -f $(UTILITIES)
	rm -f *.o
