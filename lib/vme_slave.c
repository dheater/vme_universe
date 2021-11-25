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

#define VME_SLAVE_MAGIC 0x114a0003
#define VME_SLAVE_MAGIC_NULL 0x0


struct _vme_slave_handle {
	struct vmectl_window_t ctl;	/* ioctl structure, must be first */
	int magic;
	void *vptr;
};


extern void *vme_mmap_phys(int fd, unsigned long paddr, size_t size);
extern int vme_unmap_phys(unsigned long vaddr, size_t size);


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
			    int address_space, size_t size, int flags,
			    void *phys_addr)
{
	if (NULL == handle) {
		errno = EINVAL;
		return -1;
	}

	/* Allocate the handle
	 */
	*handle = (vme_slave_handle_t)
	    malloc(sizeof (struct _vme_slave_handle));
	if (NULL == *handle)
		return -1;

	/* Populate the control structure
	 */
	(*handle)->magic = VME_SLAVE_MAGIC;
	(*handle)->ctl.vaddr = vme_addr;
	(*handle)->ctl.size = size;
	(*handle)->ctl.am = address_space;
	(*handle)->ctl.flags = flags;
	(*handle)->ctl.paddr = (phys_addr) ? phys_addr : NULL;

	/* Request the window from the driver. The physical address field gets
	   set by the driver if was not requested by the user.
	 */
	if (ioctl((int) bus_handle, VMECTL_SLAVE_WINDOW_REQUEST,
		  &((*handle)->ctl))) {
		(*handle)->magic = VME_SLAVE_MAGIC_NULL;
		free(*handle);
		*handle = NULL;
		return -1;
	}

	return 0;
}


/*============================================================================
 * Free a previously allocated VMEbus slave window handle.
 */
int vme_slave_window_release(vme_bus_handle_t bus_handle,
			     vme_slave_handle_t handle)
{
	int rval = 0;

	if ((NULL == handle) || (VME_SLAVE_MAGIC != handle->magic)) {
		errno = EINVAL;
		return -1;
	}

	/* Let the driver know that we are finished with this window
	 */
	rval = ioctl((int) bus_handle, VMECTL_SLAVE_WINDOW_RELEASE,
		     &(handle->ctl));
	handle->magic = VME_SLAVE_MAGIC_NULL;
	free(handle);

	return rval;
}


/*============================================================================
 * Return the physical address of the VMEbus slave window
 */
void *vme_slave_window_phys_addr(vme_bus_handle_t bus_handle,
				 vme_slave_handle_t handle)
{
	if ((NULL == handle) || (VME_SLAVE_MAGIC != handle->magic)) {
		errno = EINVAL;
		return NULL;
	}

	return handle->ctl.paddr;
}


/*============================================================================
 * Map the VMEbus slave window to local memory.
 */
void *vme_slave_window_map(vme_bus_handle_t bus_handle,
			   vme_slave_handle_t handle, int flags)
{
	if ((NULL == handle) || (VME_SLAVE_MAGIC != handle->magic)) {
		errno = EINVAL;
		return NULL;
	}

	return handle->vptr = vme_mmap_phys((int) bus_handle,
					    (unsigned long) handle->ctl.paddr,
					    handle->ctl.size);
}


/*============================================================================
 * Unmap the VMEbus slave window.
 */
int vme_slave_window_unmap(vme_bus_handle_t bus_handle,
			   vme_slave_handle_t handle)
{
	if ((NULL == handle) || (VME_SLAVE_MAGIC != handle->magic)) {
		errno = EINVAL;
		return -1;
	}

	return vme_unmap_phys((unsigned long) handle->vptr, handle->ctl.size);
}
