#
#==============================================================================
#                           COPYRIGHT NOTICE
#
#   Copyright (C) 2002 GE Fanuc
#   International Copyright Secured.  All Rights Reserved.
#
#------------------------------------------------------------------------------
#   Description: Top level Makefile for the GE Fanuc VMEbus driver
#------------------------------------------------------------------------------
#
#==============================================================================
#


LIBNAME:=liblegacy_vme.a


# The standard tools and options
CC:=cc
CFLAGS=-Iinclude -Wall
LD:=ld
LDFLAGS=


# Install paths
prefix:=/usr
includedir:=$(prefix)/include
libdir:=$(prefix)/lib


.PHONY: all install uninstall clean


all:lib/$(LIBNAME)


install: all
	install -m0755 -oroot -groot -d $(includedir)/vmic
	install -m0644 -oroot -groot -D include/vmic/*.h $(includedir)/vmic/
	install -m0644 -oroot -groot -D lib/$(LIBNAME) $(libdir)/$(LIBNAME)
	/sbin/ldconfig


uninstall:
	ls include/vmic/*.h | sed 's|include|$(includedir)|' | xargs rm -f
	rm -f $(libdir)/$(LIBNAME)


clean:
	find . -name "*\.o" | xargs rm -f
	find . -name "*\.so*" | xargs rm -f


lib/$(LIBNAME): lib/$(LIBNAME)(lib/vme_api.o)
	ranlib $@

lib/vme_api.o: lib/vme_api.c include/vmic/universe.h include/vmic/vme.h \
               include/vmic/vme_api.h Makefile

