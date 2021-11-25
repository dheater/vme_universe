
KERNELREV:=$(shell uname -r)
KERNELSRC:=/lib/modules/$(KERNELREV)/build

MODNAME:=vme_example.o
MODDIR:=/lib/modules/$(KERNELREV)/kernel/drivers/vme

CFLAGS = -D__KERNEL__ -DMODULE -I$(KERNELSRC)/include -O -Wall

all: $(MODNAME)

install: all
	install -m0644 -oroot -groot -D $(MODNAME) $(MODDIR)/$(MODNAME)
	/sbin/depmod

uninstall:
	rm -f $(MODDIR)/$(MODNAME)

$(MODNAME): vme_module.o
	$(LD) -r $^ -o $@

clean:
	rm -f *.o *~
