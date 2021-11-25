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

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,68)
#include <linux/wrapper.h>
#endif




#ifdef DEBUG
#define DPRINTF(x...)  printk(KERN_DEBUG "VME: "x)
#else
#define DPRINTF(x...)
#endif


EXPORT_SYMBOL_NOVERS(vme_slave_window_create);
EXPORT_SYMBOL_NOVERS(vme_slave_window_release);
EXPORT_SYMBOL_NOVERS(vme_slave_window_phys_addr);
EXPORT_SYMBOL_NOVERS(vme_slave_window_map);
EXPORT_SYMBOL_NOVERS(vme_slave_window_unmap);

#define VME_SLAVE_MAGIC 0x10e30005
#define VME_SLAVE_MAGIC_NULL 0x0
#define VME_SLAVE_WINDOWS   8


struct __vme_slave_window {
	dma_addr_t resource;
	int hardcoded;		/* Flag to indicate that this window was setup
				   by module parameters, so don't muck with it!
				 */
	uint32_t phys_base;
	uint32_t vme_base;
	uint32_t size;
	void *vptr;
	int number;
	vme_slave_handle_t handles;
};

struct _vme_slave_handle {
	int magic;
	struct __vme_slave_window *window;
	uint32_t off;
	void *id;
	vme_slave_handle_t next;	/* Pointer to the next handle */
	vme_slave_handle_t prev;	/* Pointer to the previous handle */
};

struct window_module_parameter {
	unsigned int ctl;
	unsigned int size;
	unsigned int vme_addr;
	unsigned int pci_addr;
};


static struct __vme_slave_window slave_window[VME_SLAVE_WINDOWS] = {
	{0, 0, 0, 0, 0, NULL, 0, NULL},
	{0, 0, 0, 0, 0, NULL, 1, NULL},
	{0, 0, 0, 0, 0, NULL, 2, NULL},
	{0, 0, 0, 0, 0, NULL, 3, NULL},
	{0, 0, 0, 0, 0, NULL, 4, NULL},
	{0, 0, 0, 0, 0, NULL, 5, NULL},
	{0, 0, 0, 0, 0, NULL, 6, NULL},
	{0, 0, 0, 0, 0, NULL, 7, NULL}
};

static rwlock_t slave_rwlock = RW_LOCK_UNLOCKED;
static struct proc_dir_entry *slave_proc_entry;
extern void *universe_base;
extern struct pci_dev *universe_pci_dev;


/* Module parameters - yes, an array of structs for the windows would be nice,
   but I don't know how to incorporate an array of structs into module
   parameters.
 */
static unsigned int slave_window0[4] = { 0, 0, 0, 1 };
static unsigned int slave_window1[4] = { 0, 0, 0, 1 };
static unsigned int slave_window2[4] = { 0, 0, 0, 1 };
static unsigned int slave_window3[4] = { 0, 0, 0, 1 };
static unsigned int slave_window4[4] = { 0, 0, 0, 1 };
static unsigned int slave_window5[4] = { 0, 0, 0, 1 };
static unsigned int slave_window6[4] = { 0, 0, 0, 1 };
static unsigned int slave_window7[4] = { 0, 0, 0, 1 };


/* Format is:
   Control register, Size, VMEbus address, optional PCI address
 */
MODULE_PARM(slave_window0, "3-4i");
MODULE_PARM(slave_window1, "3-4i");
MODULE_PARM(slave_window2, "3-4i");
MODULE_PARM(slave_window3, "3-4i");
MODULE_PARM(slave_window4, "3-4i");
MODULE_PARM(slave_window5, "3-4i");
MODULE_PARM(slave_window6, "3-4i");
MODULE_PARM(slave_window7, "3-4i");


/*============================================================================
 * Hook for display proc page info
 * WARNING: If the amount of data displayed exceeds a page, then we need to
 * change how this page gets registered.
 */
static int read_slave_proc_page(char *buf, char **start, off_t offset, int len,
				int *eof_unused, void *data_unused)
{
	uint32_t ctl, base, bound, to;
	int nbytes = 0;
	int ii;

	read_lock(&slave_rwlock);

	for (ii = 0; ii < VME_SLAVE_WINDOWS; ++ii) {
		ctl = readl(universe_base + UNIV_VSI_CTL(ii));
		base = readl(universe_base + UNIV_VSI_BS(ii));
		bound = readl(universe_base + UNIV_VSI_BD(ii));
		to = readl(universe_base + UNIV_VSI_TO(ii));
		if (ctl & UNIV_VSI_CTL__EN) {
			nbytes += sprintf(buf + nbytes, "slave window %d:\n  "
					  "control=%#x\n  base=%#x\n  "
					  "bound=%#x\n  "
					  "translation offset=%#x\n\n",
					  ii, ctl, base, bound, to);
		}
	}

	nbytes += sprintf(buf + nbytes, "\n");

	read_unlock(&slave_rwlock);

	return nbytes;
}


/*============================================================================
 * Disable and release resources for a slave window
 */
static void free_slave_window(int num)
{

	writel(readl(universe_base + UNIV_VSI_CTL(num)) &
	       ~UNIV_VSI_CTL__EN, universe_base + UNIV_VSI_CTL(num));

	if (slave_window[num].resource) {
		pci_free_consistent(universe_pci_dev, slave_window[num].size,
				    slave_window[num].vptr,
				    slave_window[num].resource);
		slave_window[num].vptr = NULL;
		slave_window[num].resource = 0;
	} else if (slave_window[num].vptr) {
		iounmap(slave_window[num].vptr);
		slave_window[num].vptr = NULL;
	}
}


/*============================================================================
 * Insert a slave window handle into the slave window handles list
 * NOTE: The calling process should already have acquired any necessary locks
 */
static inline void insert_slave_handle(struct __vme_slave_window *window,
				       vme_slave_handle_t handle)
{
	handle->window = window;
	handle->next = window->handles;
	handle->prev = NULL;
	if (handle->next)
		handle->next->prev = handle;

	window->handles = handle;
}


/*============================================================================
 * Remove a slave window handle from the slave window handles list
 * NOTE: The calling process should already have acquired any necessary locks
 */
static inline void remove_slave_handle(vme_slave_handle_t handle)
{
	handle->magic = VME_SLAVE_MAGIC_NULL;

	if (handle->prev)	/* This is not the first node */
		handle->prev->next = handle->next;
	else			/* This was the first node */
		handle->window->handles = handle->next;

	if (handle->next)	/* If this is not the last node */
		handle->next->prev = handle->prev;

	/* If there are no handles and the window is not hardcoded, then we can
	   unmap the window and mark it available.
	 */
	if (!handle->window->hardcoded && (NULL == handle->window->handles)) {
		DPRINTF("Reclaiming slave window %d\n", handle->window->number);
		free_slave_window(handle->window->number);
	}
}


/*============================================================================
 * Recover all slave window handles owned by the current process.
 */
void reclaim_slave_handles(vme_bus_handle_t bus_handle)
{
	vme_slave_handle_t ptr, tptr;
	int ii;

	write_lock(&slave_rwlock);

	for (ii = 0; ii < VME_SLAVE_WINDOWS; ++ii) {
		ptr = slave_window[ii].handles;
		while (ptr) {
			if (ptr->id == bus_handle) {
				tptr = ptr;
				ptr = ptr->next;
				remove_slave_handle(tptr);
				kfree(tptr);
			} else {
				ptr = ptr->next;
			}
		}
	}

	write_unlock(&slave_rwlock);
}


/*============================================================================
 * Find a slave window matching the given parameters
 */
static int find_slave_window(vme_slave_handle_t handle, uint32_t ctl,
			     uint32_t vme_addr, size_t size, void *phys_addr)
{
	struct __vme_slave_window *window;
	uint32_t base, bound, to;
	int ii;

	write_lock(&slave_rwlock);

	for (ii = 0; ii < VME_SLAVE_WINDOWS; ++ii) {
		window = &slave_window[ii];

		if (ctl == readl(universe_base + UNIV_VSI_CTL(ii))) {
			base = readl(universe_base + UNIV_VSI_BS(ii));
			bound = readl(universe_base + UNIV_VSI_BD(ii));
			to = readl(universe_base + UNIV_VSI_TO(ii));

			/* If this check fails, then either the window handle
			   has gotten corrupted somewhere, or someone has been
			   mucking with this window behind this driver's back.
			 */
			if ((base != window->vme_base)
			    || ((base + to) != window->phys_base) ||
			    ((bound - base) != window->size)) {
				printk(KERN_ERR
				       "VME: Slave window %d fails consistancy "
				       "check\n", ii);
				DPRINTF("Expected base=%#x bound=%#x to=%#x\n",
					window->vme_base,
					window->vme_base + window->size,
					window->phys_base - window->vme_base);
				DPRINTF("Found base=%#x bound=%#x to=%#x\n",
					base, bound, to);
				continue;
			}

			if (phys_addr &&
			    (vme_addr != (uint32_t) (phys_addr - to)))
				continue;

			if ((vme_addr >= base) && ((vme_addr + size) <= bound)) {
				insert_slave_handle(window, handle);
				handle->off = vme_addr - base;
				write_unlock(&slave_rwlock);
				return 0;
			}
		}
	}

	write_unlock(&slave_rwlock);

	return -ENOMEM;
}


/*============================================================================
 * Do window creation here
 */
static int __create_slave_window(vme_slave_handle_t handle,
				 struct __vme_slave_window *window, int num,
				 uint32_t ctl, uint32_t vme_addr, size_t size,
				 void *phys_addr)
{
	uint32_t base, bound, to, off;
#ifndef ARCH
	struct page *page;
#endif
	int resolution;

	/* Windows 0 and 4 have a 4kb resolution, others have
	   64kb resolution
	 */
	resolution = (num % 4) ? 0x10000 : 0x1000;
	off = vme_addr % resolution;
	vme_addr -= off;
	size += off;
	size += (size % resolution) ? resolution - (size % resolution) : 0;

	/* If we're given the physical address, then use it,
	   otherwise, let the kernel allocate the memory
	   wherever it wants to.
	 */
	if (phys_addr) {
		phys_addr -= off;
		if ((uint32_t) phys_addr % resolution) {
			write_unlock(&slave_rwlock);
			printk(KERN_ERR "VME: Invalid physical address for "
			       "slave window %d\n", num);
			return -EINVAL;
		}
	} else {
		window->vptr = pci_alloc_consistent(universe_pci_dev, size,
						    &window->resource);
		if (NULL == window->vptr) {
			window->resource = 0;
			window->vptr = NULL;
			write_unlock(&slave_rwlock);
			printk(KERN_ERR "VME: Failed to allocate memory for "
			       "slave window %d\n", num);
			return -ENOMEM;
		}
#ifdef ARCH
	    memset(window->vptr, 0, size);
	}
#else
		/* The memory manager wants to remove the
		   allocated pages from main memory.  We don't
		   want that because the user ends up seeing
		   all zero's so we set the PG_RESERVED bit
		   on each page.
		 */
		for (page = virt_to_page(window->vptr);
		     page < virt_to_page(window->vptr + size); ++page)
		{		     
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,68)
			mem_map_reserve(page);
#else			
			SetPageReserved(page);
#endif			
		}

		phys_addr = (void *) virt_to_phys(window->vptr);
#endif
		base = vme_addr;
		bound = base + size;
#ifdef ARCH
	to = (uint32_t) window->resource - base;
	window->phys_base = (uint32_t) window->vptr;
#else
		to = (uint32_t) phys_addr - base;
	}

	base = vme_addr;
	bound = base + size;
	to = (uint32_t) phys_addr - base;

	window->phys_base = (uint32_t) phys_addr;
#endif

	window->vme_base = base;
	window->size = size;

	writel(ctl, universe_base + UNIV_VSI_CTL(num));
	writel(base, universe_base + UNIV_VSI_BS(num));
	writel(bound, universe_base + UNIV_VSI_BD(num));
	writel(to, universe_base + UNIV_VSI_TO(num));

#ifndef ARCH
	/* Double check that the window setup is consistant
	   with what we expect. If this check fails, then I've
	   probably screwed up something in the driver.
	 */
	base = readl(universe_base + UNIV_VSI_BS(num));
	bound = readl(universe_base + UNIV_VSI_BD(num));
	to = readl(universe_base + UNIV_VSI_TO(num));

	if ((base != window->vme_base) || ((base + to) != window->phys_base) ||
	    ((bound - base) != window->size)) {
		write_unlock(&slave_rwlock);
		printk(KERN_ERR "VME: Slave window %d fails consistancy "
		       "check\n", num);
		DPRINTF("Expected base=%#x bound=%#x to=%#x\n",
			window->vme_base,
			window->vme_base + window->size,
			window->phys_base - window->vme_base);
		DPRINTF("Found base=%#x bound=%#x to=%#x\n", base, bound, to);
		return -EIO;
	}
#endif

	if (handle) {
		insert_slave_handle(window, handle);
		handle->off = vme_addr - base;
	}

	return 0;
}


/*============================================================================
 * Create a new slave window
 */
static int create_slave_window(vme_slave_handle_t handle, uint32_t ctl,
			       uint32_t vme_addr, size_t size, void *phys_addr)
{
	int ii, rval;

	write_lock(&slave_rwlock);

	for (ii = 0; ii < VME_SLAVE_WINDOWS; ++ii) {
		/* Look for a window that is not already enabled
		 */
		if (!(readl(universe_base + UNIV_VSI_CTL(ii)) &
		      UNIV_VSI_CTL__EN)) {

			/* Only windows 0 and 4 support A16 address space
			 */
			if ((UNIV_VSI_CTL__VAS__A16 & ctl) && (ii % 4))
				continue;

			rval = __create_slave_window(handle, &slave_window[ii],
						     ii, ctl, vme_addr, size,
						     phys_addr);

			write_unlock(&slave_rwlock);

			return rval;
		}
	}

	write_unlock(&slave_rwlock);

	return -ENOMEM;
}


/*============================================================================
 * The 2.x API had support for requesting a specific window number.  This
 * functions exists to support that deprecated feature.
 */
static int legacy_window_by_number(vme_slave_handle_t handle, int winnum,
				   size_t size)
{

	write_lock(&slave_rwlock);

	if (size > slave_window[winnum].size) {
		write_unlock(&slave_rwlock);
		return -ENOMEM;
	}

	insert_slave_handle(&slave_window[winnum], handle);
	handle->off = 0;

	write_unlock(&slave_rwlock);

	return 0;
}


/*============================================================================
 * Allocate a VMEbus slave window handle. If a suitable window is already
 * configured within the bridge device, then that window will be used,
 * otherwise, a VMEbus window is created to access the specified address and
 * address space.  
 *
 * NOTES: Setting VME_CTL_64_BIT will result in a performance degradation
 * when accessing 32-bit targets. Setting VME_CTL_64_BIT will also result in
 * performance degradation when accessing 64-bit targets if less than 64-bits
 * of data are being transferred. Enabling VME_CTL_RMW for a slave window
 * may reduce performance for all transactions through that slave window.
 */
int vme_slave_window_create(vme_bus_handle_t bus_handle,
			    vme_slave_handle_t * handle, uint64_t vme_addr,
			    int as, size_t size, int flags, void *phys_addr)
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
		*handle = (vme_slave_handle_t)
		    kmalloc(sizeof (struct _vme_slave_handle), GFP_KERNEL);
		if (!(*handle))
			return -ENOMEM;

		(*handle)->magic = VME_SLAVE_MAGIC;
		(*handle)->id = bus_handle;
		return legacy_window_by_number(*handle, vme_addr, size);
	}

	/* The VME_CTL_* values were carefully chosen to correspond with
	   UNIV_VSI_CTL__* values.  If VME_CTL_* values are changed be prepared
	   for heartache here.
	 */
	ctl = UNIV_VSI_CTL__EN | (flags &
				  (UNIV_VSI_CTL__PWEN | UNIV_VSI_CTL__PREN |
				   UNIV_VSI_CTL__PGM__BOTH |
				   UNIV_VSI_CTL__SUPER__BOTH |
				   UNIV_VSI_CTL__LLRMW | UNIV_VSI_CTL__LAS));

	/* Default to both USER and SUPER and PROGRAM and DATA
	 */
	if (!(UNIV_VSI_CTL__PGM__BOTH & ctl))
		ctl |= UNIV_VSI_CTL__PGM__BOTH;

	if (!(UNIV_VSI_CTL__SUPER__BOTH & ctl))
		ctl |= UNIV_VSI_CTL__SUPER__BOTH;

	switch (as) {
	case VME_A32:
		ctl |= UNIV_VSI_CTL__VAS__A32;
		if (((unsigned long)(vme_addr) + size) <
			(unsigned long)(vme_addr))
			return -EINVAL;
		break;
	case VME_A24:
		ctl |= UNIV_VSI_CTL__VAS__A24;
		if (0x1000000 < vme_addr + size)
			return -EINVAL;
		break;
	case VME_A16:
		ctl |= UNIV_VSI_CTL__VAS__A16;
		if (0x10000 < vme_addr + size)
			return -EINVAL;
		break;
	default:
		DPRINTF("Invalid address space=%#x\n", as);
		return -EINVAL;
	}

	*handle =
	    (vme_slave_handle_t) kmalloc(sizeof (struct _vme_slave_handle),
					 GFP_KERNEL);
	if (NULL == *handle) {
		return -ENOMEM;
	}

	(*handle)->id = bus_handle;
	(*handle)->magic = VME_SLAVE_MAGIC;

	DPRINTF("Creating handle to slave window\n     ctl=%#x, "
		"vme address=%#x size=%#x\n", ctl, (uint32_t) vme_addr, size);
	if (phys_addr)
		DPRINTF("Window physical address=%#x\n", (uint32_t) phys_addr);

	if (!find_slave_window(*handle, ctl, vme_addr, size, phys_addr)) {
		DPRINTF("Found slave window\n");
		return 0;
	}

	DPRINTF("Existing slave window not found, attempting to create one\n");

	rval = create_slave_window(*handle, ctl, vme_addr, size, phys_addr);
	if (rval) {
		kfree(*handle);
		DPRINTF("Failed to create slave window\n");
		return rval;
	}

	DPRINTF("Slave window succcessfully created\n");
	return 0;
}


/*============================================================================
 * Free a previously allocated VMEbus slave window handle.
 */
int vme_slave_window_release(vme_bus_handle_t bus_handle,
			     vme_slave_handle_t handle)
{
	if ((NULL == handle) || (VME_SLAVE_MAGIC != handle->magic)) {
		return -EINVAL;
	}

	write_lock(&slave_rwlock);

	remove_slave_handle(handle);

	write_unlock(&slave_rwlock);

	kfree(handle);

	return 0;
}


/*============================================================================
 * Return the physical address of a slave window
 */
void *vme_slave_window_phys_addr(vme_bus_handle_t bus_handle,
				 vme_slave_handle_t handle)
{
	if ((NULL == handle) || (VME_SLAVE_MAGIC != handle->magic)) {
		return NULL;
	}

	return (void *) handle->window->phys_base + handle->off;
}


/*============================================================================
 * Map the VMEbus slave window to local memory.
 */
void *vme_slave_window_map(vme_bus_handle_t bus_handle,
			   vme_slave_handle_t handle, int flags)
{
	if ((NULL == handle) || (VME_SLAVE_MAGIC != handle->magic)) {
		return NULL;
	}

	write_lock(&slave_rwlock);

	if (!handle->window->vptr) {
		handle->window->vptr = ioremap_nocache(handle->window->size,
						       handle->window->
						       phys_base);
		if (!handle->window->vptr) {
			write_unlock(&slave_rwlock);
			return NULL;
		}
	}

	write_unlock(&slave_rwlock);

	return handle->window->vptr + handle->off;
}


/*============================================================================
 * Unmap the VMEbus slave window.
 */
int vme_slave_window_unmap(vme_bus_handle_t bus_handle,
			   vme_slave_handle_t handle)
{
	if ((NULL == handle) || (VME_SLAVE_MAGIC != handle->magic)) {
		return -EINVAL;
	}

	/* We don't do anything here. Since the window mapping may be shared
	   across multiple handles it's easier to let the window be unmapped
	   when the window gets freed.
	 */
	return 0;
}


/*============================================================================
 * Initialize the VMEbus slave windows
 */
int slave_init(void)
{
	struct window_module_parameter *swind[VME_SLAVE_WINDOWS];
	void *pci_addr;
	int ii;

	/* Disable all windows initially
	 */
	for (ii = 0; ii < VME_SLAVE_WINDOWS; ++ii)
		writel(0, universe_base + UNIV_VSI_CTL(ii));

	swind[0] = (struct window_module_parameter *) slave_window0;
	swind[1] = (struct window_module_parameter *) slave_window1;
	swind[2] = (struct window_module_parameter *) slave_window2;
	swind[3] = (struct window_module_parameter *) slave_window3;
	swind[4] = (struct window_module_parameter *) slave_window4;
	swind[5] = (struct window_module_parameter *) slave_window5;
	swind[6] = (struct window_module_parameter *) slave_window6;
	swind[7] = (struct window_module_parameter *) slave_window7;

	for (ii = 0; ii < VME_SLAVE_WINDOWS; ++ii) {
		if (swind[ii]->ctl) {
			pci_addr = (void *) ((1 == swind[ii]->pci_addr) ?
					     0 : swind[ii]->pci_addr);

			/* FIXME - problem specifying a physical address of 0
			 */
			write_lock(&slave_rwlock);
			if (__create_slave_window(NULL, &slave_window[ii],
						  ii, swind[ii]->ctl, 
						  swind[ii]->vme_addr, 
						  swind[ii]->size,
						  pci_addr)) {


				printk(KERN_ERR "VME: Error configuring slave "
				       "window %x\n", ii);
				/* Not a fatal error */
			}
			slave_window[ii].hardcoded = 1;
			write_unlock(&slave_rwlock);
		}
	}

	slave_proc_entry = create_proc_entry("vme/slave", S_IRUGO | S_IWUSR,
					     NULL);
	if (!slave_proc_entry) {
		printk(KERN_WARNING
		       "VME: Failed to register slave proc page\n");
		/* Not a fatal error */
	} else {
		slave_proc_entry->read_proc = read_slave_proc_page;
	}

	return 0;
}


/*============================================================================
 * Cleanup the VMEbus slave windows for exit
 */
int slave_term(void)
{
	int ii;

	remove_proc_entry("vme/slave", NULL);

	for (ii = 0; ii < VME_SLAVE_WINDOWS; ++ii) {
		while (slave_window[ii].handles)
			vme_slave_window_release(NULL,
						 slave_window[ii].handles);

		free_slave_window(ii);
	}

	return 0;
}
