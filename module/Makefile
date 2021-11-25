#
#==============================================================================
#                           COPYRIGHT NOTICE
#
#   Copyright (C) 2002-2003 GE Fanuc
#   International Copyright Secured.  All Rights Reserved.
#
#------------------------------------------------------------------------------
#   Description: Makefile for the GE Fanuc VMEbus driver module (2.6 kernel)
#------------------------------------------------------------------------------
#
#==============================================================================
#

DEVMAJOR:=221
DEVMINOR:=8


ifneq ($(KERNELRELEASE),)
obj-m := vme_universe.o
vme_universe-objs := vme_main.o \
                     vme_master.o \
                     vme_slave.o \
                     vme_dma.o \
                     vme_interrupt.o \
                     universe.o \
                     vme_ctl.o \
                     vme_vrai.o \
                     vme_location_monitor.o \
                     vmivme.o

#CFLAGS_vme_universe.o += -DMOD_MAJOR=$(DEVMAJOR) -DMOD_NAME=\"$(PROJNAME)\" \

EXTRA_CFLAGS += -DMOD_MAJOR=$(DEVMAJOR) -DMOD_NAME=\"$(PROJNAME)\" \
                         -DVME_UNIVERSE_VERSION=\"$(MOD_VERSION)\" -I$(VMIINCPATH)

else

KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)

# Install paths
moduledir:= /lib/modules/$(shell uname -r)
mandir:= /usr/man

all:
	echo $(VMIINCPATH)
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules clean

install: all $(DEVDIR)
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules_install
	install -m0755 -oroot -groot -d $(mandir)/man9
	install -m0644 -oroot -groot -D ../doc/*.9 $(mandir)/man9/
	/sbin/depmod

uninstall: clean
	rm -f $(moduledir)/extra/$(PROJNAME).*
	ls ../doc/*.9 | sed 's|../doc|$(mandir)/man9|' | xargs rm -f
	/sbin/depmod

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	
# Major and minor device numbers and device names come from devices.txt
# in kernel source documentation
$(DEVDIR):
	mkdir -p $@
	mknod --mode=666 $(DEVDIR)/ctl c $(DEVMAJOR) $(DEVMINOR)

endif
