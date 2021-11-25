#
#==============================================================================
#                           COPYRIGHT NOTICE
#
#   Copyright (C) 2002-2003 GE Fanuc
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
LDFLAGS=


# Install paths
prefix:=
exec_prefix:=$(prefix)/usr
bindir:=$(exec_prefix)/bin
includedir:=$(exec_prefix)/include
libdir:=$(exec_prefix)/lib
mandir:=$(exec_prefix)/man
moduledir:=$(prefix)/lib/modules/$(KERNELREV)/kernel/drivers/vme


.PHONY: all install uninstall clean


UTILITIES:=vme_peek \
           vme_poke \
           vme_slave_peek \
           vme_slave_poke \
           vme_catch_interrupt \
           vme_generate_interrupt \
           vme_dma_read \
           vme_dma_write \
           vme_sysreset \
           vme_acquire_bus \
           vme_release_bus \
           vme_endian \
           vme_rmw


all: $(UTILITIES)


install: all
	install -m0755 -oroot -groot -d $(mandir)/man1
	install -m0644 -oroot -groot -D ../doc/*.1 $(mandir)/man1/
	install -m0755 -oroot -groot -D $(UTILITIES) $(bindir)/

	
clean:
	rm -f $(UTILITIES)
	rm -f *.o

	
uninstall: clean
	for file in $(UTILITIES); do \
            echo $$file | sed 's|test|$(bindir)|' | xargs rm -f; \
        done
	ls ../doc/*.1 | sed 's|../doc|$(mandir)/man1|' | xargs rm -f

	
$(UTILITIES):CFLAGS+=-O2 -Iinclude
$(UTILITIES):LDFLAGS+=-Llib -lvme
$(UTILITIES): %: %.c vme_test.o ../lib/$(LIBNAME) Makefile
	$(CC) $(CFLAGS) $< -o $@ vme_test.o $(LDFLAGS)

vme_test.o: vme_test.c vme_test.h ../include/vme/vme.h \
            ../include/vme/universe.h ../include/vme/vme_api.h Makefile

