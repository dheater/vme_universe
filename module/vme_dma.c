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
   o Neither the name of GE Fanuc nor the names of its contributors may be used      to endorse or promote products derived from this software without specific
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

#ifdef ARCH
#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif
#endif

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/sched.h>
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


EXPORT_SYMBOL_NOVERS(vme_dma_buffer_create);
EXPORT_SYMBOL_NOVERS(vme_dma_buffer_release);
EXPORT_SYMBOL_NOVERS(vme_dma_buffer_phys_addr);
EXPORT_SYMBOL_NOVERS(vme_dma_buffer_map);
EXPORT_SYMBOL_NOVERS(vme_dma_buffer_unmap);
EXPORT_SYMBOL_NOVERS(vme_dma_read);
EXPORT_SYMBOL_NOVERS(vme_dma_write);

#define VME_DMA_MAGIC 0x10e30001
#define VME_DMA_MAGIC_NULL 0x0

struct __vme_dma {
	vme_dma_handle_t handles;
};

struct _vme_dma_handle {
	int magic;
	struct __vme_dma *dma;
	dma_addr_t resource;
	void *vptr;
	void *phys_addr;
	void *pci_addr;
	size_t size;
	void *id;
	int allocated_memory;
	vme_dma_handle_t next;	/* Pointer to the next handle */
	vme_dma_handle_t prev;	/* Pointer to the previous handle */
};


static struct __vme_dma dma;
DECLARE_MUTEX(dma_lock);
static rwlock_t dma_rwlock = RW_LOCK_UNLOCKED;
extern void *universe_base;
extern wait_queue_head_t interrupt_wq[];
extern struct pci_dev *universe_pci_dev;
extern struct semaphore vown_lock;


/*============================================================================
 * Insert a dma window handle into the dma window handles list
 * NOTE: The calling process should already have acquired any necessary locks
 */
static inline void insert_dma_handle(struct __vme_dma *_dma,
				     vme_dma_handle_t handle)
{
	handle->dma = _dma;
	handle->next = _dma->handles;
	handle->prev = NULL;
	if (handle->next)
		handle->next->prev = handle;

	_dma->handles = handle;
}


/*============================================================================
 * Remove a dma window handle from the dma window handles list
 * NOTE: The calling process should already have acquired any necessary locks
 */
static inline void remove_dma_handle(vme_dma_handle_t handle)
{
	handle->magic = VME_DMA_MAGIC_NULL;

	if (handle->prev)	/* This is not the first node */
		handle->prev->next = handle->next;
	else			/* This was the first node */
		handle->dma->handles = handle->next;

	if (handle->next)	/* If this is not the last node */
		handle->next->prev = handle->prev;
}


/*============================================================================
 * Recover all dma buffer handles owned by the current process.
 */
void reclaim_dma_handles(vme_bus_handle_t bus_handle)
{
	vme_dma_handle_t ptr, tptr;
#ifndef ARCH
	struct page *page;
#endif

	write_lock(&dma_rwlock);

	ptr = dma.handles;
	while (ptr) {
		if (ptr->id == bus_handle) {
			tptr = ptr;
			ptr = ptr->next;

			remove_dma_handle(tptr);
#ifndef ARCH
			for (page = virt_to_page(tptr->vptr);
			     page <= virt_to_page(tptr->vptr + tptr->size - 1);
			     ++page) {
			     
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,68)
				mem_map_unreserve(page);
#else
				ClearPageReserved(page);
#endif				
			}
#endif
			pci_free_consistent(universe_pci_dev, tptr->size,
					    tptr->vptr, tptr->resource);

			kfree(tptr);
		} else {
			ptr = ptr->next;
		}
	}

	write_unlock(&dma_rwlock);
}


/*============================================================================
 * Allocate a DMA safe memory buffer and associated handle.
 */
int vme_dma_buffer_create(vme_bus_handle_t bus_handle,
			  vme_dma_handle_t * handle, size_t size, int flags,
			  void *phys_addr)
{
#ifndef ARCH
	struct page *page;
#endif

	if (NULL == handle) {
		return -EINVAL;
	}

	*handle = (vme_dma_handle_t) kmalloc(sizeof (struct _vme_dma_handle),
					     GFP_KERNEL);
	if (NULL == *handle) {
		return -ENOMEM;
	}

	if (phys_addr) {
		(*handle)->phys_addr = phys_addr;
		(*handle)->vptr = NULL;

		DPRINTF("%#x bytes at physical address %#lx, assigned to "
			"DMA buffer\n", size, (unsigned long) phys_addr);
	} else {
		(*handle)->vptr = pci_alloc_consistent(universe_pci_dev, size,
						       &(*handle)->resource);
		if ((NULL == (*handle)->vptr)) {
#ifdef ARCH
            printk(KERN_DEBUG "pci_alloc_consistent failed\n");
#endif
			return -ENOMEM;
		}

#ifdef ARCH
		(*handle)->phys_addr = (*handle)->vptr;
                }
		(*handle)->pci_addr = (vme_dma_handle_t *)(*handle)->resource;
#else
		for (page = virt_to_page((*handle)->vptr);
		     page <= virt_to_page((*handle)->vptr + size - 1); ++page)
		{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,68)
			mem_map_reserve(page);
#else
			SetPageReserved(page);
#endif			
		}

		(*handle)->phys_addr = (void *) virt_to_phys((*handle)->vptr);
		(*handle)->pci_addr = (*handle)->phys_addr;

		/* Physical address and pci address are not the same thing on
		   PPC */

		DPRINTF("%#x byte DMA buffer allocated with physical "
			"address %#lx\n", size,
			(unsigned long) (*handle)->phys_addr);
	}
#endif
	(*handle)->magic = VME_DMA_MAGIC;
	(*handle)->size = size;
	(*handle)->id = bus_handle;

	/* Set a flag to indicate whether we allocated memory using
	   pci_alloc_consistant or were given the physical address.
	 */
	(*handle)->allocated_memory = (phys_addr) ? 0 : 1;

	write_lock(&dma_rwlock);

	insert_dma_handle(&dma, *handle);

	write_unlock(&dma_rwlock);

	return 0;
}


/*============================================================================
 * Free a DMA buffer and it's associated handle.
 */
int vme_dma_buffer_release(vme_bus_handle_t bus_handle, vme_dma_handle_t handle)
{
#ifndef ARCH
	struct page *page;
#endif

	if ((NULL == handle) || (VME_DMA_MAGIC != handle->magic)) {
		return -EINVAL;
	}

	vme_dma_buffer_unmap(bus_handle, handle);

	if (handle->allocated_memory) {
#ifndef ARCH
		for (page = virt_to_page(handle->vptr);
		     page <= virt_to_page(handle->vptr + handle->size - 1);
		     ++page) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,68)
			mem_map_unreserve(page);
#else
			ClearPageReserved(page);
#endif			
		}
#endif
		pci_free_consistent(universe_pci_dev, handle->size,
				    handle->vptr, handle->resource);
	}

	write_lock(&dma_rwlock);

	remove_dma_handle(handle);

	write_unlock(&dma_rwlock);

	kfree(handle);

	return 0;
}


/*============================================================================
 * Return the physical address of a dma buffer
 */
void *vme_dma_buffer_phys_addr(vme_bus_handle_t bus_handle,
			       vme_dma_handle_t handle)
{
	if ((NULL == handle) || (VME_DMA_MAGIC != handle->magic)) {
		return NULL;
	}

	return handle->phys_addr;
}


/*============================================================================
 * Map a DMA buffer
 */
void *vme_dma_buffer_map(vme_bus_handle_t bus_handle, vme_dma_handle_t handle,
			 int flags)
{
	if ((NULL == handle) || (VME_DMA_MAGIC != handle->magic)) {
		return NULL;
	}

	/* If we were given the physical address for the buffer, and we
	   haven't done so already, map the buffer.
	 */
	if (!handle->allocated_memory && !handle->vptr) {
		return handle->vptr = ioremap_nocache((int) handle->phys_addr,
						      handle->size);
	}

	return handle->vptr;
}


/*============================================================================
 * Unmap a DMA buffer
 */
int vme_dma_buffer_unmap(vme_bus_handle_t bus_handle, vme_dma_handle_t handle)
{
	if ((NULL == handle) || (VME_DMA_MAGIC != handle->magic)) {
		return -EINVAL;
	}

	/* If we were given the physical address and the buffer is mapped,
	   unmap it.
	 */
	if (!handle->allocated_memory && handle->vptr) {
		iounmap(handle->vptr);
		handle->vptr = NULL;
	}

	return 0;
}


/*============================================================================
 * Transfer data using the DMA engine.
 */
static int dma_transfer(vme_dma_handle_t handle, unsigned long offset,
			uint64_t vme_addr, int am, size_t nbytes, int flags)
{
	uint32_t ctl, dgcs, count, off;
	DECLARE_WAITQUEUE(wait, current);
	int index = UNIV_INTERRUPT_INDEX(VME_INTERRUPT_DMA, 0);

	/* The VME and PCI addresses must be 8-byte aligned
	 */
	if ((vme_addr % 8) != ((long) (handle->pci_addr + offset) % 8)) {
		DPRINTF("DMA transfer request addresses are not aligned\n");
		return -EINVAL;
	}

	/* NOTE: The VME_CTL_* values were chosen to match UNIV_DCTL_* values,
	   if the VME_CTL_* values changes, be prepared for heartache.
	 */
	ctl = (UNIV_DCTL__L2V | UNIV_DCTL__LD64EN | 0x00E00000) & flags;

	switch (am) {
	case VME_A32UMB:
	case VME_A32UB:
		ctl |= UNIV_DCTL__VAS__A32 | UNIV_DCTL__VCT;
		break;
	case VME_A32UD:
		ctl |= UNIV_DCTL__VAS__A32;
		break;
	case VME_A32UP:
		ctl |= UNIV_DCTL__VAS__A32 | UNIV_DCTL__PGM;
		break;
	case VME_A32SMB:
	case VME_A32SB:
		ctl |= UNIV_DCTL__VAS__A32 | UNIV_DCTL__SUPER | UNIV_DCTL__VCT;
		break;
	case VME_A32SD:
		ctl |= UNIV_DCTL__VAS__A32 | UNIV_DCTL__SUPER;
		break;
	case VME_A32SP:
		ctl |= UNIV_DCTL__VAS__A32 | UNIV_DCTL__SUPER | UNIV_DCTL__PGM;
		break;
	case VME_A16U:
		ctl |= UNIV_DCTL__VAS__A16;
		break;
	case VME_A16S:
		ctl |= UNIV_DCTL__VAS__A16 | UNIV_DCTL__SUPER;
		break;
	case VME_A24UMB:
	case VME_A24UB:
		ctl |= UNIV_DCTL__VAS__A24 | UNIV_DCTL__VCT;
		break;
	case VME_A24UD:
		ctl |= UNIV_DCTL__VAS__A24;
		break;
	case VME_A24UP:
		ctl |= UNIV_DCTL__VAS__A24 | UNIV_DCTL__PGM;
		break;
	case VME_A24SMB:
	case VME_A24SB:
		ctl |= UNIV_DCTL__VAS__A24 | UNIV_DCTL__SUPER | UNIV_DCTL__VCT;
		break;
	case VME_A24SD:
		ctl |= UNIV_DCTL__VAS__A24 | UNIV_DCTL__SUPER;
		break;
	case VME_A24SP:
		ctl |= UNIV_DCTL__VAS__A24 | UNIV_DCTL__SUPER | UNIV_DCTL__PGM;
		break;
	default:
		DPRINTF("Invalid DMA address modifier=%#x\n", am);
		return -EINVAL;
	}

	/* Sanity check the VMEbus address
	 */
	switch (UNIV_LSI_CTL__VAS & ctl) {
	case UNIV_DCTL__VAS__A16:
		if (0x10000 < vme_addr + nbytes)
			return -EINVAL;
	case UNIV_DCTL__VAS__A24:
		if (0x1000000 < vme_addr + nbytes)
			return -EINVAL;
	case UNIV_DCTL__VAS__A32:
		if (((unsigned long) (vme_addr) + nbytes) <
		    (unsigned long) (vme_addr))
			return -EINVAL;
	}

	/* Set the data width.  Data width can be provided by the user through
	   the flags argument. If VDW is programmed for 64-bits then the
	   Universe tries to do MBLT's regardless of what the address modifier
	   was, so we return EINVAL if the address modifier and data width are
	   inconsistant.  If flags we're not specified, then we use a default
	   data width of 64 for MBLT
	   address modifiers, and 32 for everything else.
	   NOTE: The VME_DMA_DW_* flags were carefully chosen to match up with 
	   UNIV_DCTL__VDW flags so if you mess with VME_DMA_DW_* values be
	   prepared for heartache.
	 */
	if (0x0 == (am & 0x03)) {	/* MBLT */
		if (0x00E00000 & flags) {
			if (UNIV_DCTL__VDW__D64 != (ctl & UNIV_DCTL__VDW))
				return -EINVAL;
		} else {
			ctl |= UNIV_DCTL__VDW__D64;
		}
	} else {		/* Not MBLT */

		if (0x00E00000 & flags) {
			if (UNIV_DCTL__VDW__D64 == (ctl & UNIV_DCTL__VDW))
				return -EINVAL;
		} else {
			ctl |= UNIV_DCTL__VDW__D32;
		}

		/* Do not allow BLT's with 8-bit transfers.
		   They return bad data.
		 */
		if ((VME_DMA_DW_8 & flags) && (UNIV_DCTL__VCT & ctl))
			return -EINVAL;
	}

	/* Determine the DGCS register value from the given flags.
	   When specifying the VME_DMA_VON* values, if we left them as defined
	   by the Universe chip they would have been in conflict with the
	   VME_DMA_DW* values, so I shifted them 8 bits to the right. The
	   VME_DMA_VOFF* values were defined to match UNIV_DGCS__VOFF values,
	   so just 'or' them in.
	 */
	dgcs = (((UNIV_DGCS__VON >> 8) & flags) << 8) |
	    (UNIV_DGCS__VOFF & flags);

	if (down_interruptible(&dma_lock))
		return -EINTR;

	/* The DMA transfer cannot take place if VOWN is set. We lock the vown
	   mutex here to prevent a race condition.  One might think that we
	   could turn off VOWN during the transfer, and turn it back on at the
	   end, but that could also result in race conditions.
	 */
	if (down_interruptible(&vown_lock)) {
		up(&dma_lock);
		return -EINTR;
	}

	if (readl(universe_base + UNIV_MAST_CTL) & UNIV_MAST_CTL__VOWN_ACK) {
		up(&vown_lock);
		up(&dma_lock);
		return -EPERM;
	}

	/* If the last DMA is still active wait for it to complete
	 */
	while (readl(universe_base + UNIV_DGCS) & UNIV_DGCS__ACT) ;

	off = 0;
	while (nbytes) {
		count = (0x1000000 > nbytes) ? nbytes : 0xfff000;

		writel(count, universe_base + UNIV_DTBC);
#ifdef ARCH
		writel((uint32_t) handle->resource + offset + off,
		       universe_base + UNIV_DLA);
#else
		writel((uint32_t) handle->pci_addr + offset + off,
		       universe_base + UNIV_DLA);
#endif
		writel(vme_addr + off, universe_base + UNIV_DVA);
		writel(ctl, universe_base + UNIV_DCTL);

		/* Put this task on the wait queue BEFORE we start the transfer.
		   When the DMA interrupt occurs, this task gets awakened.
		 */
		set_current_state(TASK_INTERRUPTIBLE);
		add_wait_queue(&interrupt_wq[index], &wait);

		/* Kick off the transfer. Weeee!!!
		 */
		writel(dgcs | UNIV_DGCS__GO | UNIV_DGCS__STOP | UNIV_DGCS__HALT
		       | UNIV_DGCS__DONE | UNIV_DGCS__LERR | UNIV_DGCS__VERR |
		       UNIV_DGCS__P_ERR | UNIV_DGCS__INT_STOP |
		       UNIV_DGCS__INT_HALT | UNIV_DGCS__INT_DONE |
		       UNIV_DGCS__INT_LERR | UNIV_DGCS__INT_VERR |
		       UNIV_DGCS__INT_P_ERR, universe_base + UNIV_DGCS);

		/* Go to sleep. Zzzzz!
		 */
		schedule();

		/* OK, we've been woken up and now we're ready to run. 
		 */
		set_current_state(TASK_RUNNING);
		remove_wait_queue(&interrupt_wq[index], &wait);

		/* Check DGCS to find out the status of the transfer
		 */
		dgcs = readl(universe_base + UNIV_DGCS);

		/* Testing for signal_pending() outside of the check for DMA
		   done is not an option since the signal may have been
		   generated by the DMA done interrupt, in which case returning 
		   -EINTR does not make sense. This does present a small race
		   condition. If a signal not related to a DMA complete
		   interrupt occurs and the DMA is complete at the same time,
		   then we will return a successful DMA status instead of
		   -ERESTARTSYS. That's generally a good thing. The only time
		   it is a problem is if we are early in a REALLY big DMA
		   transfer (note the while loop that this code is in).
		   In that case, the signal delivery will get delayed until
		   the full DMA is complete. 
		 */
		if (!(dgcs & UNIV_DGCS__DONE)) {
			/* Halt the pending transfer
			 */
			writel(dgcs | UNIV_DGCS__STOP,
			       universe_base + UNIV_DGCS);

			up(&vown_lock);
			up(&dma_lock);

			/* Returning -ERESTARTSYS will allow the signal to
			   be processed immediately, then the DMA will be
			   retried.
			 */
			if (signal_pending(current)) {
				DPRINTF("DMA interrupted by a signal\n");
				return -ERESTARTSYS;
			}

			DPRINTF("DMA did not complete sucessfully\n");
			return -EIO;
		}

		off += count;
		nbytes -= count;
	}

	up(&vown_lock);
	up(&dma_lock);

	DPRINTF("DMA completed sucessfully\n");

	return 0;
}


/*============================================================================
 * Read data from the VMEbus using DMA.
 *
 * NOTES: Not all data widths are available for all address modifiers.
 * The default data width is the maximum available for the given address
 * modifier.
 */
int vme_dma_read(vme_bus_handle_t bus_handle, vme_dma_handle_t handle,
		 unsigned long offset, uint64_t vme_addr, int am, size_t nbytes,
		 int flags)
{
	if ((NULL == handle) || (VME_DMA_MAGIC != handle->magic)) 
	{
	        DPRINTF("VME:DMA read - ERROR 1\n");
		return -EINVAL;
	}

	flags &= ~(UNIV_DCTL__L2V);

	DPRINTF("VME: DMA transfer requested from %#lx VME, %#x AM, to %#lx\n",
		(unsigned long) vme_addr, am,
		(unsigned long) handle->phys_addr);

	return dma_transfer(handle, offset, vme_addr, am, nbytes, flags);
}

/*============================================================================
 * Write data to the VMEbus using DMA.
 *
 * NOTES: Not all data widths are available for all address modifiers.
 * The default data width is the maximum available for the given address
 * modifier.
 */
int vme_dma_write(vme_bus_handle_t bus_handle, vme_dma_handle_t handle,
		  unsigned long offset, uint64_t vme_addr, int am,
		  size_t nbytes, int flags)
{
	if ((NULL == handle) || (VME_DMA_MAGIC != handle->magic))
        {
#ifdef ARCH
	        DPRINTF("VME:DMA write - ERROR 1\n");
#endif
		return -EINVAL;
	}

	flags |= UNIV_DCTL__L2V;

#ifdef ARCH
	DPRINTF("VME: DMA transfer requested from %#lx to %#lx VME, %#x AM\n",
		(unsigned long) handle->vptr, (unsigned long) vme_addr,
		am);
#else
	DPRINTF("VME:DMA transfer requested from %#lx to %#lx VME, %#x AM\n",
		(unsigned long) handle->phys_addr, (unsigned long) vme_addr,
		am);
#endif
	return dma_transfer(handle, offset, vme_addr, am, nbytes, flags);
}
