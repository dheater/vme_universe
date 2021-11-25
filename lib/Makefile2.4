#
#==============================================================================
#                           COPYRIGHT NOTICE
#
#   Copyright (C) 2002-2003 GE Fanuc
#   International Copyright Secured.  All Rights Reserved.
#
#------------------------------------------------------------------------------
#   Description: Makefile for the GE Fanuc VMEbus driver API library
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
DEVDIR:=/dev/bus/vme
FIXUPFLAGS:=$(shell ../fixupflags.sh)

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


all: $(LIBNAME).$(MOD_VERSION)


install: all $(DEVDIR)
	install -m0755 -oroot -groot -d $(includedir)/vme
	install -m0644 -oroot -groot -D ../include/vme/*.h $(includedir)/vme/
	install -m0644 -oroot -groot $(LIBNAME).$(MOD_VERSION) $(libdir)
	/sbin/ldconfig
	ln -sf $(LIBNAME).$(MOD_MAJOR) $(libdir)/$(LIBNAME)
	install -m0755 -oroot -groot -d $(mandir)/man3
	install -m0644 -oroot -groot -D ../doc/*.3 $(mandir)/man3/


uninstall: clean
	ls ../include/vme/*.h | sed 's|include|$(includedir)|' | xargs rm -f
	rm -f $(libdir)/$(LIBNAME)*
	ls ../doc/*.3 | sed 's|../doc|$(mandir)/man3|' | xargs rm -f
	/sbin/ldconfig


clean:
	rm -f *.o
	rm -f $(LIBNAME)*


LIB_OBJECTS:=vme_main.o \
             vme_master.o \
             vme_slave.o \
             vme_dma.o \
             vme_interrupt.o \
             vme_ctl.o \
             vme_vrai.o \
             vme_location_monitor.o

$(LIB_OBJECTS):CFLAGS+=-O2 -fPIC
$(LIBNAME).$(MOD_VERSION): $(LIB_OBJECTS)
	$(LD) $(LDFLAGS) -shared -soname $(LIBNAME).$(MOD_MAJOR) -o $@ $(LIB_OBJECTS)
	/sbin/ldconfig -v -n 
	ln -sf $(LIBNAME).$(MOD_VERSION) $(LIBNAME)

vme_ctl.o: vme_ctl.c ../include/vme/vme.h ../include/vme/vme_api.h Makefile

vme_dma.o: vme_dma.c ../include/vme/vme.h ../include/vme/vme_api.h Makefile

vme_interrupt.o: vme_interrupt.c ../include/vme/vme.h \
                 ../include/vme/vme_api.h Makefile

vme_main.o: vme_main.c ../include/vme/vme.h ../include/vme/vme_api.h Makefile

vme_master.o: vme_master.c ../include/vme/vme.h ../include/vme/vme_api.h \
              Makefile

vme_slave.o: vme_slave.c ../include/vme/vme.h ../include/vme/vme_api.h \
             Makefile

vme_vrai.o: vme_vrai.c ../include/vme/vme.h ../include/vme/vme_api.h Makefile

vme_location_monitor.o: vme_location_monitor.c ../include/vme/vme.h \
                        ../include/vme/vme_api.h Makefile

