/*
===============================================================================
                            COPYRIGHT NOTICE

    Copyright (C) 2002-2006 GE Fanuc
    International Copyright Secured.  All Rights Reserved.

-------------------------------------------------------------------------------

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

   o Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.
   o Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.
   o Neither the name of GE Fanuc nor the names of its contributors may be used
     endorse or promote products derived from this software without specific
     prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

===============================================================================
*/


#include <linux/config.h>
#ifdef CONFIG_SMP
#define __SMP__
#endif

#ifdef  ARCH
#ifdef  MODVERSIONS
#include <linux/modversions.h>
#endif
#endif

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include "vme/universe.h"
#include "vme/vme.h"
#include "vme/vme_api.h"


#ifdef DEBUG
#define DPRINTF(x...)  printk(KERN_DEBUG "VME: "x)
#else
#define DPRINTF(x...)
#endif


EXPORT_SYMBOL_NOVERS(vme_master_window_create);
EXPORT_SYMBOL_NOVERS(vme_master_window_release);
EXPORT_SYMBOL_NOVERS(vme_master_window_phys_addr);
EXPORT_SYMBOL_NOVERS(vme_master_window_map);
EXPORT_SYMBOL_NOVERS(vme_master_window_unmap);
EXPORT_SYMBOL_NOVERS(vme_master_window_translate);

#define VME_MASTER_MAGIC 0x10e30004
#define VME_MASTER_MAGIC_NULL 0x0
#define VME_MASTER_WINDOWS  8

struct __vme_master_window {
	struct resource resource;
	int hardcoded;		/* Flag to indicate that this window was setup
				   by module parameters, so don't muck with it!
				 */
	int exclusive;		/* Indicates that there may be only one handle
				   to this window.  This is necessary to allow
				   window translation.
				 */
	uint32_t phys_base;
	uint32_t vme_base;
	uint32_t size;
	int number;
	void *vptr;
	vme_master_handle_t handles;
};

struct _vme_master_handle {
	int magic;
	struct __vme_master_window *window;
	uint32_t off;
	void *id;
	vme_master_handle_t next;	/* Pointer to the next handle */
	vme_master_handle_t prev;	/* Pointer to the previous handle */
};

struct window_module_parameter {
	unsigned int ctl;
	unsigned int size;
	unsigned int vme_addr;
	unsigned int pci_addr;
};


static struct __vme_master_window master_window[VME_MASTER_WINDOWS] = {
	{{"VMEbus master window 0", 0, 0, IORESOURCE_MEM}, 0, 0, 0, 0, 0,
	 0, NULL,
	 NULL},
	{{"VMEbus master window 1", 0, 0, IORESOURCE_MEM}, 0, 0, 0, 0, 0,
	 1, NULL,
	 NULL},
	{{"VMEbus master window 2", 0, 0, IORESOURCE_MEM}, 0, 0, 0, 0, 0,
	 2, NULL,
	 NULL},
	{{"VMEbus master window 3", 0, 0, IORESOURCE_MEM}, 0, 0, 0, 0, 0,
	 3, NULL,
	 NULL},
	{{"VMEbus master window 4", 0, 0, IORESOURCE_MEM}, 0, 0, 0, 0, 0,
	 4, NULL,
	 NULL},
	{{"VMEbus master window 5", 0, 0, IORESOURCE_MEM}, 0, 0, 0, 0, 0,
	 5, NULL,
	 NULL},
	{{"VMEbus master window 6", 0, 0, IORESOURCE_MEM}, 0, 0, 0, 0, 0,
	 6, NULL,
	 NULL},
	{{"VMEbus master window 7", 0, 0, IORESOURCE_MEM}, 0, 0, 0, 0, 0,
	 7, NULL,
	 NULL}
};

static struct __vme_master_window slsi_window = {
	{"VMEbus SLSI window", 0, 0, IORESOURCE_MEM}, 0, 0, 0, 0, 0, 8,
	NULL, NULL
};

static rwlock_t master_rwlock = RW_LOCK_UNLOCKED;
static struct proc_dir_entry *master_proc_entry;
extern void *universe_base;
extern struct pci_dev *universe_pci_dev;
extern uint32_t pci_lo_bound;
extern uint32_t pci_hi_bound;


/* Module parameters - yes, an array of structs for the windows would be nice,
   but I don't know how to incorporate an array of structs into module
   parameters.
 */
static unsigned int master_window0[4] = { 0, 0, 0, 1 };
static unsigned int master_window1[4] = { 0, 0, 0, 1 };
static unsigned int master_window2[4] = { 0, 0, 0, 1 };
static unsigned int master_window3[4] = { 0, 0, 0, 1 };
static unsigned int master_window4[4] = { 0, 0, 0, 1 };
static unsigned int master_window5[4] = { 0, 0, 0, 1 };
static unsigned int master_window6[4] = { 0, 0, 0, 1 };
static unsigned int master_window7[4] = { 0, 0, 0, 1 };


/* Format is:
   Control register, Size, VMEbus address, optional PCI address
 */
MODULE_PARM(master_window0, "3-4i");
MODULE_PARM(master_window1, "3-4i");
MODULE_PARM(master_window2, "3-4i");
MODULE_PARM(master_window3, "3-4i");
MODULE_PARM(master_window4, "3-4i");
MODULE_PARM(master_window5, "3-4i");
MODULE_PARM(master_window6, "3-4i");
MODULE_PARM(master_window7, "3-4i");


/*============================================================================
 * Hook for display proc page info
 * WARNING: If the amount of data displayed exceeds a page, then we need to
 * change how this page gets registered.
 */
static int read_master_proc_page(char *buf, char **start, off_t offset, int len,
				 int *eof_unused, void *data_unused)
{
	uint32_t ctl, base, bound, to;
	int nbytes = 0;
	int ii;

	read_lock(&master_rwlock);

	ctl = readl(universe_base + UNIV_SLSI);
	if (ctl & UNIV_SLSI__EN)
		nbytes += sprintf(buf + nbytes, "slsi=%#x\n\n", ctl);

	for (ii = 0; ii < VME_MASTER_WINDOWS; ++ii) {
		ctl = readl(universe_base + UNIV_LSI_CTL(ii));
		base = readl(universe_base + UNIV_LSI_BS(ii));
		bound = readl(universe_base + UNIV_LSI_BD(ii));
		to = readl(universe_base + UNIV_LSI_TO(ii));
		if (ctl & UNIV_LSI_CTL__EN) {
			nbytes += sprintf(buf + nbytes,
					  "master window %d:\n  control=%#x\n  "
					  "base=%#x\n  bound=%#x\n  "
					  "translation offset=%#x\n\n",
					  ii, ctl, base, bound, to);
		}
	}

	nbytes += sprintf(buf + nbytes, "\n");

	read_unlock(&master_rwlock);

	return nbytes;
}


/*============================================================================
 * Disable and release resources for a master window
 */
static void free_master_window(int num)
{
	struct __vme_master_window *window = &master_window[num];

	if (window->vptr)
		iounmap(window->vptr);
	window->vptr = NULL;

	writel(readl(universe_base + UNIV_LSI_CTL(num)) &
	       ~UNIV_LSI_CTL__EN, universe_base + UNIV_LSI_CTL(num));

	if (window->resource.end - window->resource.start) {
		if (release_resource(&window->resource)) {
			printk(KERN_WARNING "VME: Error releasing master "
			       "window %d resource\n", num);
		}
		window->resource.start = 0;
		window->resource.end = 0;
	}
}


/*============================================================================
 * Insert a master window handle into the master window handles list
 * NOTE: The calling process should already have acquired any necessary locks
 */
static inline void insert_master_handle(struct __vme_master_window *window,
					vme_master_handle_t handle)
{
	handle->window = window;
	handle->next = window->handles;
	handle->prev = NULL;
	if (handle->next)
		handle->next->prev = handle;

	window->handles = handle;
}


/*============================================================================
 * Remove a master window handle from the master window handles list
 * NOTE: The calling process should already have acquired any necessary locks
 */
static inline void remove_master_handle(vme_master_handle_t handle)
{
	handle->magic = VME_MASTER_MAGIC_NULL;

	if (handle->prev)	/* This is not the first node */
		handle->prev->next = handle->next;
	else			/* This was the first node */
		handle->window->handles = handle->next;

	if (handle->next)	/* If this is not the last node */
		handle->next->prev = handle->prev;

	/* If there are no handles, the window is not hardcoded, and it is not
	   the SLSI window (whose number is above VME_MASTER_WINDOWS) then we
	   can unmap the window and mark it available.
	 */
	if (!handle->window->hardcoded && (NULL == handle->window->handles)
	    && handle->window->number < VME_MASTER_WINDOWS) {
		DPRINTF("Reclaiming master window %d\n",
			handle->window->number);
		free_master_window(handle->window->number);
	}

}


/*============================================================================
 * Recover all master window handles owned by the current process.
 */
void reclaim_master_handles(vme_bus_handle_t bus_handle)
{
	vme_master_handle_t ptr, tptr;
	int ii;

	write_lock(&master_rwlock);

	/* Scan all of the handles for the current bus handle
	 */
	for (ii = 0; ii < VME_MASTER_WINDOWS; ++ii) {
		ptr = master_window[ii].handles;
		while (ptr) {
			if (ptr->id == bus_handle) {
				tptr = ptr;
				ptr = ptr->next;
				remove_master_handle(tptr);
				kfree(tptr);
			} else {
				ptr = ptr->next;
			}
		}
	}

	ptr = slsi_window.handles;
	while (ptr) {
		if (ptr->id == bus_handle) {
			tptr = ptr;
			ptr = ptr->next;
			remove_master_handle(tptr);
			kfree(tptr);
		} else {
			ptr = ptr->next;
		}
	}
	write_unlock(&master_rwlock);
}


/*============================================================================
 * Find a master window matching the given parameters
 */
static int find_master_window(vme_master_handle_t handle, uint32_t ctl,
			      uint32_t vme_addr, size_t size, void *phys_addr)
{
	struct __vme_master_window *window;
	uint32_t base, bound, to;
	int ii;

	write_lock(&master_rwlock);

	for (ii = 0; ii < VME_MASTER_WINDOWS; ++ii) {
		window = &master_window[ii];

		/* Ignore exclusively reserved master windows.
		 */
		if (window->exclusive)
			continue;

		if (ctl == readl(universe_base + UNIV_LSI_CTL(ii))) {
			base = readl(universe_base + UNIV_LSI_BS(ii));
			bound = readl(universe_base + UNIV_LSI_BD(ii));
			to = readl(universe_base + UNIV_LSI_TO(ii));

			/* If this check fails, then either the window handle
			   has gotten corrupted somewhere, or someone has been
			   mucking with this window behind this driver's back.
			 */
			if ((base != window->phys_base)
			    || ((base + to) != window->vme_base)
			    || ((bound - base) != (window->size))) {
				printk(KERN_ERR "VME: Master window %d fails "
				       "consistancy check\n", ii);
				DPRINTF("Expected base=%#x bound=%#x to=%#x\n",
					window->phys_base,
					window->phys_base + window->size,
					window->vme_base - window->phys_base);
				DPRINTF("Found base=%#x bound=%#x to=%#x\n",
					base, bound, to);
				continue;
			}

			if (phys_addr &&
			    (vme_addr != (uint32_t) (phys_addr + to)))
				continue;

			if ((vme_addr >= (base + to)) &&
			    ((vme_addr + size) <= (bound + to))) {
				insert_master_handle(window, handle);
				handle->off = vme_addr - window->vme_base;
				write_unlock(&master_rwlock);
				return 0;
			}
		}
	}

	write_unlock(&master_rwlock);

	return -ENOMEM;
}


/*============================================================================
 * Try getting a resource (range of PCI addresses) from the PCI bus we're on
 */
static int allocate_pci_resource(unsigned long size, unsigned long align,
				 struct resource *new_resource)
{
	/* Determine the bus the Tundra is on */
	struct pci_bus *bus = universe_pci_dev->bus;
	int i;

	do {
		DPRINTF("Attempting to allocate resources from bus %d\n",
			bus->number);
		for (i = 0; i < 4; i++) {
			int retval;
			struct resource *r = bus->resource[i];

			/* Check if that resource exists */
			if (!r) {
				continue;
			}

			/* If the resource is not I/O memory (e.g. I/O ports) */
			if (!(r->flags & IORESOURCE_MEM)) {
				continue;
			}

			/* Print out name of resource for debugging */
			if (r->name) {
				DPRINTF("Checking bus resource with name "
					"\"%s\".\n", r->name);
			}
			DPRINTF("resource.start: %08lX, resource.end: %08lX.\n",
				r->start, r->end);

			/* Try to allocate a new sub-resource from this
			   given the proper size and alignment */
			retval = allocate_resource(r, new_resource, size,
						   pci_lo_bound, pci_hi_bound,
						   align, NULL, NULL);

			/* If this allocation fails, try with next resource
			   (and give debug message) */
			if (retval) {
				if (r->name) {
					DPRINTF("Failed allocating from bus "
						"resource with name \"%s\".\n",
						r->name);
				} else {
					DPRINTF("Failed allocating from bus "
						"resource with number %d.\n",
						i);
				}

				continue;
			} else {
				/* If this allocation succeeds, return what
				   allocate_resource() returned */
				return retval;
			}
		}
	} while ((bus = bus->parent));

	/* Return busy if no resource could be successfully allocated */
	new_resource->start = new_resource->end = 0;
	return -EBUSY;
}


/*============================================================================
 * Do the actual window creation here
 */
static int __create_master_window(vme_master_handle_t handle,
				  struct __vme_master_window *window, int num,
				  uint32_t ctl, uint32_t vme_addr, size_t size,
				  void *phys_addr, int exclusive,
				  int resolution)
{
	uint32_t base, bound, to, off;
	int rval;

	/* Windows 0 and 4 have a 4kb resolution, others have
	   64kb resolution
	 */
	off = vme_addr % resolution;
	size += off;
	size += (size % resolution) ? resolution - (size % resolution) : 0;

	/* If we're given the physical address, then use it,
	   otherwise, let the kernel allocate the memory
	   wherever it wants to.
	 */
	if (phys_addr) {
		phys_addr -= off;
		if ((uint32_t) phys_addr % resolution) {
			write_unlock(&master_rwlock);
			printk(KERN_ERR "VME: Invalid physical address for "
			       "master window %d\n", num);
			return -EINVAL;
		}

		base = (uint32_t) phys_addr;
		bound = base + size;
		to = vme_addr - off - base;

		window->resource.start = base;
		window->resource.end = bound - 1;

		rval = request_resource(&iomem_resource, &window->resource);
		if (rval) {
			window->resource.start = 0;
			window->resource.end = 0;
			write_unlock(&master_rwlock);
			printk(KERN_ERR "VME: Unable to use memory address "
			       "range %#x-%#x\n for master window %d\n",
			       base, bound, num);
			return rval;
		}
	} else {
		rval = allocate_pci_resource(size, resolution,
					     &window->resource);
		if (rval) {
			window->resource.start = 0;
			window->resource.end = 0;
			write_unlock(&master_rwlock);
			printk(KERN_ERR "VME: Failed to allocate memory for "
			       "master window %d\n", num);
			return rval;
		}

		base = window->resource.start;
		bound = window->resource.end + 1;
		to = vme_addr - off - base;
	}

	window->phys_base = base;
	window->vme_base = vme_addr - off;
	window->size = size;
	window->exclusive = exclusive;

	writel(ctl, universe_base + UNIV_LSI_CTL(num));
	writel(base, universe_base + UNIV_LSI_BS(num));
	writel(bound, universe_base + UNIV_LSI_BD(num));
	writel(to, universe_base + UNIV_LSI_TO(num));

	/* Double check that the window setup is consistant
	   with what we expect. If this check fails, then I've
	   probably screwed up something in the driver.
	 */
	base = readl(universe_base + UNIV_LSI_BS(num));
	bound = readl(universe_base + UNIV_LSI_BD(num));
	to = readl(universe_base + UNIV_LSI_TO(num));

	if ((base != window->phys_base) || ((base + to) != window->vme_base)
	    || ((bound - base) != (window->size))) {
		printk(KERN_ERR "VME: Master window %d fails consistancy "
		       "check\n", num);
		DPRINTF("Expected base=%#x bound=%#x to=%#x\n",
			window->phys_base, window->phys_base + window->size,
			window->vme_base - window->phys_base);
		DPRINTF("Found base=%#x bound=%#x to=%#x\n", base, bound, to);
		write_unlock(&master_rwlock);
		return -EIO;
	}

	if (handle) {
		insert_master_handle(window, handle);
		handle->off = off;
	}

	return 0;
}


/*============================================================================
 * Create a new master window
 */
static int create_master_window(vme_master_handle_t handle, uint32_t ctl,
				uint32_t vme_addr, size_t size, void *phys_addr,
				int exclusive)
{
	uint32_t win_ctl;
	int ii, resolution, rval;

	write_lock(&master_rwlock);

	/* Look for a window that is not already enabled
	 */
	for (ii = 0; ii < VME_MASTER_WINDOWS; ++ii) {
		win_ctl = readl(universe_base + UNIV_LSI_CTL(ii));
		if (!(win_ctl & UNIV_LSI_CTL__EN)) {
			resolution = (ii % 4) ? 0x10000 : 0x1000;
			DPRINTF("Attempting to assign master window %d\n"
				"     control %#x, VME address %#x, size %#x\n",
				ii, ctl, vme_addr, size);
			rval = __create_master_window(handle,
						      &master_window[ii], ii,
						      ctl, vme_addr, size,
						      phys_addr, exclusive,
						      resolution);

			write_unlock(&master_rwlock);

			return rval;
		}
	}

	write_unlock(&master_rwlock);

	return -ENOMEM;
}


/*============================================================================
 * Find an SLSI window matching the given parameters.
 */
static int find_slsi_window(vme_master_handle_t handle, uint32_t ctl,
			    uint32_t vme_addr, size_t size)
{
	uint32_t slsi, slsi_mask, ctl_mask, bound;
	int ii;

	write_lock(&master_rwlock);

	slsi = readl(universe_base + UNIV_SLSI);
	if (!(slsi & UNIV_SLSI__EN)) {
		write_unlock(&master_rwlock);
		return -ENOMEM;
	}

	/* Make sure values of PWEN and LAS match. The SLSI windows do not
	   handle BLT's, so if VCT is set in the control register, return an
	   error here.
	 */
	slsi_mask = slsi & (UNIV_SLSI__PWEN | UNIV_SLSI__LAS);
	ctl_mask = ctl & (UNIV_LSI_CTL__PWEN | UNIV_LSI_CTL__VCT |
			  UNIV_LSI_CTL__LAS);
	if (slsi_mask != ctl_mask) {
		write_unlock(&master_rwlock);
		return -ENOMEM;
	}

	/* There are four SLSI images. Each image specifies an A24 window,
	   addresses, 0x0 - 0xFEFFFF, and an A16 window addresses, 0x0 - 0xFFFF.
	 */
	for (ii = 0; ii < 4; ++ii) {
		ctl_mask = 0;

		switch (UNIV_LSI_CTL__VDW & ctl) {
		case UNIV_LSI_CTL__VDW__D16:
			ctl_mask |= UNIV_SLSI__VDW__D16(ii);
			break;
		case UNIV_LSI_CTL__VDW__D32:
			ctl_mask |= UNIV_SLSI__VDW__D32(ii);
			break;
		default:
			write_unlock(&master_rwlock);
			return -ENOMEM;
		}

		if (UNIV_LSI_CTL__PGM & ctl)
			ctl_mask |= UNIV_SLSI__PGM(ii);

		if (UNIV_LSI_CTL__SUPER & ctl)
			ctl_mask |= UNIV_SLSI__SUPER(ii);

		slsi_mask = UNIV_SLSI__VDW(ii) | UNIV_SLSI__PGM(ii) |
		    UNIV_SLSI__SUPER(ii);
		if (ctl_mask == (slsi & slsi_mask)) {
			handle->off = ii * 0x1000000;
			switch (UNIV_LSI_CTL__VAS & ctl) {
			case UNIV_LSI_CTL__VAS__A16:
				handle->off += 0xFF0000;
				bound = 0x10000;
				break;
			case UNIV_LSI_CTL__VAS__A24:
				bound = 0xFF0000;
				break;
			default:
				write_unlock(&master_rwlock);
				return -ENOMEM;
			}

			if ((vme_addr + size) > bound) {
				write_unlock(&master_rwlock);
				return -ENOMEM;
			}

			insert_master_handle(&slsi_window, handle);
			handle->off += vme_addr;

			write_unlock(&master_rwlock);

			return 0;
		}
	}

	write_unlock(&master_rwlock);

	return -ENOMEM;
}


/*============================================================================
 * Create the SLSI windows
 */
static int create_slsi_window(void)
{
	int rval;

	/* Allocate a 64MB window with 64MB resolution
	 */
	rval = allocate_pci_resource(0x4000000, 0x4000000,
				     &slsi_window.resource);
	if (rval) {
		printk(KERN_WARNING "VME: Unable to allocate memory for SLSI "
		       "window\n");
		return rval;
	}

	write_lock(&master_rwlock);

	/* Window 0=SP, 1=UP, 2=SD, 3=UD
	 */
	writel(UNIV_SLSI__EN | UNIV_SLSI__PWEN | UNIV_SLSI__VDW__D32(0) |
	       UNIV_SLSI__VDW__D32(1) | UNIV_SLSI__VDW__D32(2) |
	       UNIV_SLSI__VDW__D32(3) | UNIV_SLSI__PGM(0) |
	       UNIV_SLSI__PGM(1) | UNIV_SLSI__SUPER(0) |
	       UNIV_SLSI__SUPER(2) |
	       (UNIV_SLSI__BS & (slsi_window.resource.start >> 24)),
	       universe_base + UNIV_SLSI);

	slsi_window.phys_base = slsi_window.resource.start;
	slsi_window.size = slsi_window.resource.end -
	    slsi_window.resource.start;

	write_unlock(&master_rwlock);

	return 0;
}


/*============================================================================
 * Disable and release resources for the SLSI window
 */
static void free_slsi_window(void)
{
	if (slsi_window.vptr)
		iounmap(slsi_window.vptr);
	slsi_window.vptr = NULL;

	writel(readl(universe_base + UNIV_SLSI) & ~UNIV_SLSI__EN,
	       universe_base + UNIV_SLSI);

	if (slsi_window.resource.end - slsi_window.resource.start) {
		if (release_resource(&slsi_window.resource))
			printk(KERN_WARNING "VME: Error releasing SLSI window "
			       "resource\n");
		slsi_window.resource.start = 0;
		slsi_window.resource.end = 0;
	}
}


/*============================================================================
 * The 2.x API had support for requesting a specific window number.  This
 * functions exists to support that deprecated feature.
 */
static int legacy_window_by_number(vme_master_handle_t handle, int winnum,
				   size_t size)
{
	struct __vme_master_window *window = &master_window[winnum];

	write_lock(&master_rwlock);

	if (window->exclusive) {
		write_unlock(&master_rwlock);
		return -EPERM;
	}

	if (size > window->size) {
		write_unlock(&master_rwlock);
		return -ENOMEM;
	}

	insert_master_handle(window, handle);
	handle->off = 0;

	write_unlock(&master_rwlock);

	return 0;
}


/*============================================================================
 * Allocate a VMEbus master window handle. If a suitable window is already
 * configured within the bridge device, then that window will be used,
 * otherwise, a VMEbus window is created to access the specified address and
 * address space.  
 *
 * NOTES: Not all max data widths are available for all address modifiers.
 * The default max data width is the maximum available for the given address
 * modifier.
 */
int vme_master_window_create(vme_bus_handle_t bus_handle,
			     vme_master_handle_t * handle, uint64_t vme_addr,
			     int am, size_t size, int flags, void *phys_addr)
{
	uint32_t ctl;
	int rval;

	if (NULL == handle) {
		return -EINVAL;
	}

	/* The 2.x API had a feature to allow you to request a specific VMEbus
	   window by number. We still need to support it for the legacy API
	   wrapper.
	 */
	if (VME_CTL_LEGACY_WINNUM & flags) {
		*handle = (vme_master_handle_t)
		    kmalloc(sizeof (struct _vme_master_handle), GFP_KERNEL);
		if (NULL == *handle)
			return -ENOMEM;

		(*handle)->id = bus_handle;
		return legacy_window_by_number(*handle, vme_addr, size);
	}

	/* The VME_CTL_* flags values were carefully chosen to match up with the
	   UNIV_LSI_CTL_* flags.  If the VME_CTL_* values change be prepared for
	   heartache.  
	 */
	ctl = UNIV_LSI_CTL__EN | (flags & (UNIV_LSI_CTL__PWEN | 0x00E00000 |
					   UNIV_LSI_CTL__LAS));

	switch (am) {
	case VME_A32UMB:
	case VME_A32UB:
		ctl |= UNIV_LSI_CTL__VAS__A32 | UNIV_LSI_CTL__VCT;
		break;
	case VME_A32UD:
		ctl |= UNIV_LSI_CTL__VAS__A32;
		break;
	case VME_A32UP:
		ctl |= UNIV_LSI_CTL__VAS__A32 | UNIV_LSI_CTL__PGM;
		break;
	case VME_A32SMB:
	case VME_A32SB:
		ctl |= UNIV_LSI_CTL__VAS__A32 | UNIV_LSI_CTL__SUPER |
		    UNIV_LSI_CTL__VCT;
		break;
	case VME_A32SD:
		ctl |= UNIV_LSI_CTL__VAS__A32 | UNIV_LSI_CTL__SUPER;
		break;
	case VME_A32SP:
		ctl |= UNIV_LSI_CTL__VAS__A32 | UNIV_LSI_CTL__SUPER |
		    UNIV_LSI_CTL__PGM;
		break;
	case VME_A16U:
		ctl |= UNIV_LSI_CTL__VAS__A16;
		break;
	case VME_A16S:
		ctl |= UNIV_LSI_CTL__VAS__A16 | UNIV_LSI_CTL__SUPER;
		break;
	case VME_A24UMB:
	case VME_A24UB:
		ctl |= UNIV_LSI_CTL__VAS__A24 | UNIV_LSI_CTL__VCT;
		break;
	case VME_A24UD:
		ctl |= UNIV_LSI_CTL__VAS__A24;
		break;
	case VME_A24UP:
		ctl |= UNIV_LSI_CTL__VAS__A24 | UNIV_LSI_CTL__PGM;
		break;
	case VME_A24SMB:
	case VME_A24SB:
		ctl |= UNIV_LSI_CTL__VAS__A24 | UNIV_LSI_CTL__SUPER |
		    UNIV_LSI_CTL__VCT;
		break;
	case VME_A24SD:
		ctl |= UNIV_LSI_CTL__VAS__A24 | UNIV_LSI_CTL__SUPER;
		break;
	case VME_A24SP:
		ctl |= UNIV_LSI_CTL__VAS__A24 | UNIV_LSI_CTL__SUPER |
		    UNIV_LSI_CTL__PGM;
		break;
	default:
		DPRINTF("Invalid address modifier=%#x\n", am);
		return -EINVAL;
	}

	/* Sanity check the VMEbus address
	 */
	switch (UNIV_LSI_CTL__VAS & ctl) {
	case UNIV_LSI_CTL__VAS__A16:
		if (0x10000 < vme_addr + size)
			return -EINVAL;
	case UNIV_LSI_CTL__VAS__A24:
		if (0x1000000 < vme_addr + size)
			return -EINVAL;
	case UNIV_LSI_CTL__VAS__A32:
		if (((unsigned long)(vme_addr) + size) <
			(unsigned long)(vme_addr))
			return -EINVAL;
	}

	/* Set the max data width.  Max data width can be provided by the user
	   through the flags argument. If VDW is programmed for 64-bits then
	   the Universe tries to do MBLT's regardless of what the address
	   modifier was, so we return EINVAL if the address modifier and max
	   data width are inconsistent.  If flags were not specified, then we
	   use a default data width of 64 for MBLT address modifiers, and 32
	   for everything else.
	   NOTE: The VME_MAX_DW_* flags were carefully chosen to match up with 
	   UNIV_LSI_CTL__VDW flags, so if you mess with VME_MAX_DW_* values be
	   prepared for heartache.  This is subtle, but the VME_CTL_DW_8 flag
	   is in the reserved space so we can detect its presence and not use
	   the default values (that's where the mask 0x00F00000 comes in), but
	   if it was specified it got masked off to the correct value (0) when
	   we assigned flags to ctl.
	 */
	if (0x0 == (am & 0x03)) {	/* MBLT */
		if (0x00E00000 & flags) {
			if (UNIV_LSI_CTL__VDW__D64 != (ctl & UNIV_LSI_CTL__VDW))
				return -EINVAL;
		} else {
			ctl |= UNIV_LSI_CTL__VDW__D64;
		}
	} else {		/* Not MBLT */

		if (0x00E00000 & flags) {
			if (UNIV_LSI_CTL__VDW__D64 == (ctl & UNIV_LSI_CTL__VDW))
				return -EINVAL;
		} else {
			ctl |= UNIV_LSI_CTL__VDW__D32;
		}
	}

	*handle = (vme_master_handle_t)
	    kmalloc(sizeof (struct _vme_master_handle), GFP_KERNEL);
	if (NULL == *handle)
		return -ENOMEM;

	(*handle)->magic = VME_MASTER_MAGIC;
	(*handle)->id = bus_handle;

	DPRINTF("Allocating master window\n     ctl=%#x, vme address=%#x "
		"size=%#x\n", ctl, (uint32_t) vme_addr, size);

	if (phys_addr)
		DPRINTF("Window physical address=%#x\n", (uint32_t) phys_addr);

	/* If this is an exclusive window then don't bother trying to find an
	   existing window.
	 */
	if (!(VME_CTL_EXCLUSIVE & flags)) {
		if (0 == find_master_window(*handle, ctl, vme_addr, size,
					    phys_addr)) {
			DPRINTF("Found master window\n");
			return 0;
		}

		DPRINTF("Master window not found, trying SLSI windows\n");

		if (0 == find_slsi_window(*handle, ctl, vme_addr, size)) {
			DPRINTF("Found master window\n");
			return 0;
		}

		DPRINTF("Existing master window not found, "
			"attempting to create one\n");
	}

	rval = create_master_window(*handle, ctl, vme_addr, size, phys_addr,
				    VME_CTL_EXCLUSIVE & flags);
	if (rval) {
		kfree(*handle);
		DPRINTF("Failed to create master window\n");
		return rval;
	}

	DPRINTF("Master window succcessfully created\n");
	return 0;
}


/*============================================================================
 * Free a previously allocated VMEbus master window handle.
 */
int vme_master_window_release(vme_bus_handle_t bus_handle,
			      vme_master_handle_t handle)
{
	if ((NULL == handle) || (VME_MASTER_MAGIC != handle->magic)) {
		return -EINVAL;
	}

	write_lock(&master_rwlock);
	remove_master_handle(handle);
	write_unlock(&master_rwlock);
	kfree(handle);

	return 0;
}


/*============================================================================
 * Return the physical address of a master window
 */
void *vme_master_window_phys_addr(vme_bus_handle_t bus_handle,
				  vme_master_handle_t handle)
{
	if ((NULL == handle) || (VME_MASTER_MAGIC != handle->magic)) {
		return NULL;
	}

	return (void *) handle->window->phys_base + handle->off;
}


/*============================================================================
 * Map the VMEbus master window to local memory.
 */
void *vme_master_window_map(vme_bus_handle_t bus_handle,
			    vme_master_handle_t handle, int flags)
{
	if ((NULL == handle) || (VME_MASTER_MAGIC != handle->magic)) {
		return NULL;
	}

	write_lock(&master_rwlock);

	if (!handle->window->vptr) {
		handle->window->vptr =
		    ioremap_nocache(handle->window->phys_base,
				    handle->window->size);
		if (NULL == handle->window->vptr) {
			write_unlock(&master_rwlock);
			return NULL;
		}
	}

	write_unlock(&master_rwlock);

	return handle->window->vptr + handle->off;
}


/*============================================================================
 * Unmap the VMEbus master window.
 */
int vme_master_window_unmap(vme_bus_handle_t bus_handle,
			    vme_master_handle_t handle)
{
	if ((NULL == handle) || (VME_MASTER_MAGIC != handle->magic)) {
		return -EINVAL;
	}

	/* We don't do anything here. Since the window mapping may be shared
	   across multiple handles it's easier to let the window be unmapped
	   when the window gets freed.
	 */
	return 0;
}


/*============================================================================
 * Shift the base VMEbus address of a VME master window.
 * 
 * NOTE: The window must be created with the VME_CTL_EXCLUSIVE flag to use this
 * function. The new VMEbus address must be aligned indentically with respect
 * to a 64-kb boundary as the original VMEbus address.
 */
int vme_master_window_translate(vme_bus_handle_t bus_handle,
				vme_master_handle_t handle, uint64_t vme_addr)
{
	struct __vme_master_window *window;
	int resolution, off, num;

	if ((NULL == handle) || (VME_MASTER_MAGIC != handle->magic)) {
		return -EINVAL;
	}

	if (!handle->window->exclusive) {
		return -EPERM;
	}

	num = handle->window->number;
	window = &master_window[num];

	/* Windows 0 and 4 have a 4kb resolution, others have 64kb resolution
	 */
	resolution = (num % 4) ? 0x10000 : 0x1000;
	off = (uint32_t) vme_addr % resolution;

	/* The translated address must be aligned identical to the previous
	   address
	 */
	if (off != handle->off) {
		DPRINTF("Failed attempt to translate VMEbus address "
			"from %#x to %#x\n", window->vme_base + handle->off,
			(uint32_t) vme_addr);
		return -EINVAL;
	}

	write_lock(&master_rwlock);

	window->vme_base = vme_addr - off;
	writel(window->vme_base - window->phys_base,
	       universe_base + UNIV_LSI_TO(num));

	write_unlock(&master_rwlock);

	return 0;
}


/*============================================================================
 * Perform a read modify write cycle on the VMEbus
 * 
 * Read modify write is performed by reading a value from the VMEbus, the
 * masked values are bitwise compared with the contents of cmp.  All bits that
 * compare true are are swapped with the contents of the swap register.
 *
 * NOTE: To prevent side effects, the window must be created with the
 * VME_CTL_EXCLUSIVE flag to use this function.  If your application is
 * multi-threaded you should take measures to protect againsts other threads
 * attempting to make access to the same VMEbus address.  Read modify write
 * only works for PCI memory mapped windows.
 */
int vme_read_modify_write(vme_bus_handle_t bus_handle,
			  vme_master_handle_t handle, size_t offset, int dw,
			  uint64_t mask, uint64_t cmp, uint64_t swap)
{
	uint32_t ctl, max_dw, las;
	void *addr;

	if ((NULL == handle) || (VME_MASTER_MAGIC != handle->magic)) {
		return -EINVAL;
	}

	if (!handle->window->exclusive) {
		return -EPERM;
	}

	ctl = readl(universe_base + UNIV_LSI_CTL(handle->window->number));
	max_dw = ctl & UNIV_LSI_CTL__VDW;
	las = ctl & UNIV_LSI_CTL__LAS;

	/* Make sure the data width and alignment are valid before we proceed
	 */
	switch (dw) {
	case VME_D8:
		if ((handle->off + offset) % 4) { 
			return -EINVAL;
		}
		break;
	case VME_D16:
		if (((handle->off + offset) % 4) ||
		    (UNIV_LSI_CTL__VDW__D16 > max_dw)) {
			return -EINVAL;
		}
		break;
	case VME_D32:
		if (((handle->off + offset) % 4) ||
		    (UNIV_LSI_CTL__VDW__D32 > max_dw)) {
			return -EINVAL;
		}
		break;
	default:
		return -EINVAL;
	}

	addr = vme_master_window_map(bus_handle, handle, 0);
	if (NULL == addr) {
		return -ENOMEM;
	}

	write_lock(&master_rwlock);

	writel(handle->window->phys_base + handle->off + offset,
	       universe_base + UNIV_SCYC_ADDR);
	writel(mask, universe_base + UNIV_SCYC_EN);
	writel(cmp, universe_base + UNIV_SCYC_CMP);
	writel(swap, universe_base + UNIV_SCYC_SWP);
	writel(UNIV_SCYC_CTL__SCYC__RMW | ((las) ? UNIV_SCYC_CTL__LAS : 0),
	       universe_base + UNIV_SCYC_CTL);

	/* The read-modify-write cycle is initiated when we do a read.
	 */
	switch (dw) {
	case VME_D8:
		*(volatile uint8_t *) (addr + offset);
		break;
	case VME_D16:
		*(volatile uint16_t *) (addr + offset);
		break;
	case VME_D32:
		*(volatile uint32_t *) (addr + offset);
		break;
	}

	writel(0, universe_base + UNIV_SCYC_CTL);

	write_unlock(&master_rwlock);

	vme_master_window_unmap(bus_handle, handle);

	return 0;
}


/*============================================================================
 * Initialize the VMEbus master windows
 */
int master_init(void)
{
	struct window_module_parameter *mwind[VME_MASTER_WINDOWS];
	int ii, resolution;

	/* Disable all windows initially
	 */
	for (ii = 0; ii < VME_MASTER_WINDOWS; ++ii)
		writel(0, universe_base + UNIV_LSI_CTL(ii));

	mwind[0] = (struct window_module_parameter *) master_window0;
	mwind[1] = (struct window_module_parameter *) master_window1;
	mwind[2] = (struct window_module_parameter *) master_window2;
	mwind[3] = (struct window_module_parameter *) master_window3;
	mwind[4] = (struct window_module_parameter *) master_window4;
	mwind[5] = (struct window_module_parameter *) master_window5;
	mwind[6] = (struct window_module_parameter *) master_window6;
	mwind[7] = (struct window_module_parameter *) master_window7;

	for (ii = 0; ii < VME_MASTER_WINDOWS; ++ii) {
		if (mwind[ii]->ctl) {
			/* FIXME - problem specifying a physical address of 0
			   not that you would want to here.
			 */
			if (1 == mwind[ii]->pci_addr)
				mwind[ii]->pci_addr = 0;

			resolution = (ii % 4) ? 0x10000 : 0x1000;
			write_lock(&master_rwlock);
			if (__create_master_window(NULL,
						   &master_window[ii], ii,
						   mwind[ii]->ctl, 
						   mwind[ii]->vme_addr, 
						   mwind[ii]->size,
						   (void *)mwind[ii]->pci_addr, 
						   0,
						   resolution)) {


				printk(KERN_ERR "VME: Error configuring "
				       "master window %x\n", ii);
				/* Not a fatal error */
			}
			master_window[ii].hardcoded = 1;
			write_unlock(&master_rwlock);
		}
	}

	if (create_slsi_window()) {
		printk(KERN_ERR "VME: Error configuring SLSI window\n");
		/* Not a fatal error */
	}

	master_proc_entry = create_proc_entry("vme/master", S_IRUGO | S_IWUSR,
					      NULL);
	if (NULL == master_proc_entry) {
		printk(KERN_WARNING "VME: Failed to register master proc "
		       "page\n");
		/* Not a fatal error */
	} else {
		master_proc_entry->read_proc = read_master_proc_page;
	}

	return 0;
}


/*============================================================================
 * Cleanup the VMEbus master windows for exit
 */
int master_term(void)
{
	int ii;

	remove_proc_entry("vme/master", NULL);

	for (ii = 0; ii < VME_MASTER_WINDOWS; ++ii) {
		while (master_window[ii].handles)
			vme_master_window_release(NULL,
						  master_window[ii].handles);

		free_master_window(ii);
	}

	while (slsi_window.handles)
		vme_master_window_release(NULL, slsi_window.handles);

	free_slsi_window();

	return 0;
}
