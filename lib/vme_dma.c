/*
===============================================================================
                            COPYRIGHT NOTICE

    Copyright (C) 2002-2004 GE Fanuc
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
     to endorse or promote products derived from this software without specific
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


#include <errno.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "vme/vme.h"
#include "vme/vme_api.h"

#define VME_DMA_MAGIC 0x114a0000
#define VME_DMA_MAGIC_NULL 0x0

struct _vme_dma_handle {
	struct vmectl_dma_t ctl;	/* ioctl structure, must be first */
	int magic;
	size_t size;
	void *vptr;
};


extern void *vme_mmap_phys(int fd, unsigned long paddr, size_t size);
extern int vme_unmap_phys(unsigned long vaddr, size_t size);


/*============================================================================
 * Allocate a DMA safe memory buffer and associated handle.
 */
int vme_dma_buffer_create(vme_bus_handle_t bus_handle,
			  vme_dma_handle_t * handle, size_t size, int flags,
			  void *phys_addr)
{
	if (NULL == handle) {
		errno = EINVAL;
		return -1;
	}

	/* Allocate the handle
	 */
	*handle = (vme_dma_handle_t) malloc(sizeof (struct _vme_dma_handle));
	if (NULL == *handle)
		return -1;

	/* Populate the ctl structure. We populate the size fields in both the
	   control structure and the handle. The size field in the ctl structure
	   is used to make the driver allocate space, but later this will be
	   overwritten when we do a read or write. The handle size field is
	   persistant.
	 */
	(*handle)->magic = VME_DMA_MAGIC;
	(*handle)->size = (*handle)->ctl.size = size;
	(*handle)->ctl.paddr = phys_addr;

	if (ioctl((int) bus_handle, VMECTL_DMA_BUFFER_ALLOC, *handle)) {
		(*handle)->magic = VME_DMA_MAGIC_NULL;
		free(*handle);
		*handle = NULL;
		return -1;
	}

	return 0;
}


/*============================================================================
 * Free a DMA buffer and it's associated handle.
 */
int vme_dma_buffer_release(vme_bus_handle_t bus_handle, vme_dma_handle_t handle)
{
	int rval;

	if ((NULL == handle) || (VME_DMA_MAGIC != handle->magic)) {
		errno = EINVAL;
		return -1;
	}

	rval = ioctl((int) bus_handle, VMECTL_DMA_BUFFER_FREE, handle);
	handle->magic = VME_DMA_MAGIC_NULL;
	free(handle);

	return rval;
}


/*============================================================================
 * Return the physical address of the DMA buffer
 */
void *vme_dma_buffer_phys_addr(vme_bus_handle_t bus_handle,
			       vme_dma_handle_t handle)
{
	if ((NULL == handle) || (VME_DMA_MAGIC != handle->magic)) {
		errno = EINVAL;
		return NULL;
	}

	return handle->ctl.paddr;
}


/*============================================================================
 * Map a DMA buffer
 */
void *vme_dma_buffer_map(vme_bus_handle_t bus_handle, vme_dma_handle_t handle,
			 int flags)
{
	if ((NULL == handle) || (VME_DMA_MAGIC != handle->magic)) {
		errno = EINVAL;
		return NULL;
	}

	return handle->vptr = vme_mmap_phys((int) bus_handle,
					    (unsigned long) handle->ctl.paddr,
					    handle->ctl.size);
}


/*============================================================================
 * Unmap a DMA buffer
 */
int vme_dma_buffer_unmap(vme_bus_handle_t bus_handle, vme_dma_handle_t handle)
{
	if ((NULL == handle) || (VME_DMA_MAGIC != handle->magic)) {
		errno = EINVAL;
		return -1;
	}

	return vme_unmap_phys((unsigned long) handle->vptr, handle->ctl.size);
}


/*============================================================================
 * Read data from the VMEbus using DMA.
 *
 * NOTES: Not all max data widths are available for all address modifiers.
 * The default max data width is the maximum available for the given address
 * modifier.
 */
int vme_dma_read(vme_bus_handle_t bus_handle, vme_dma_handle_t handle,
		 unsigned long offset, uint64_t vme_addr, int am, size_t nbytes,
		 int flags)
{
	if ((NULL == handle) || (VME_DMA_MAGIC != handle->magic)) {
		errno = EINVAL;
		return -1;
	}

	/* Populate the control structure
	 */
	handle->ctl.vaddr = vme_addr;
	handle->ctl.size = nbytes;
	handle->ctl.offset = offset;
	handle->ctl.am = am;
	handle->ctl.flags = flags;

	/* Do the transfer
	 */
	return ioctl((int) bus_handle, VMECTL_DMA_READ, handle);
}


/*============================================================================
 * Write data to the VMEbus using DMA.
 *
 * NOTES: Not all max data widths are available for all address modifiers.
 * The default max data width is the maximum available for the given address
 * modifier.
 */
int vme_dma_write(vme_bus_handle_t bus_handle, vme_dma_handle_t handle,
		  unsigned long offset, uint64_t vme_addr, int am,
		  size_t nbytes, int flags)
{
	if ((NULL == handle) || (VME_DMA_MAGIC != handle->magic)) {
		errno = EINVAL;
		return -1;
	}
	/* Populate the control structure
	 */
	handle->ctl.vaddr = vme_addr;
	handle->ctl.size = nbytes;
	handle->ctl.offset = offset;
	handle->ctl.am = am;
	handle->ctl.flags = flags;

	/* Do the transfer
	 */
	return ioctl((int) bus_handle, VMECTL_DMA_WRITE, handle);
}
