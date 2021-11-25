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
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "vme/vme.h"
#include "vme/vme_api.h"

#define VME_MASTER_MAGIC 0x114a0002
#define VME_MASTER_MAGIC_NULL 0x0


struct _vme_master_handle {
	struct vmectl_window_t ctl;	/* ioctl structure, must be first */
	int magic;
	void *vptr;
};


extern void *vme_mmap_phys(int fd, unsigned long paddr, size_t size);
extern int vme_unmap_phys(unsigned long vaddr, size_t size);


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
#if 0 /* This code is for eventual implementation of read/write functions/ */
	char *path = "/dev/vme/mx";
#endif

	if (NULL == handle) {
		errno = EINVAL;
		return -1;
	}

	/* Allocate the handle
	 */
	*handle = (vme_master_handle_t)
	    malloc(sizeof (struct _vme_master_handle));
	if (NULL == *handle) {
		return -1;
	}

	/* Populate the control structure
	 */
	(*handle)->ctl.vaddr = vme_addr;
	(*handle)->ctl.size = size;
	(*handle)->ctl.am = am;
	(*handle)->ctl.flags = flags;
	(*handle)->ctl.paddr = (phys_addr) ? phys_addr : NULL;

	/* Request the window from the driver. The physical address field gets
	   set by the driver if was not requested by the user.
	 */
	if (ioctl((int) bus_handle, VMECTL_MASTER_WINDOW_REQUEST, *handle)) {
		free(*handle);
		*handle = NULL;
		return -1;
	}

	(*handle)->magic = VME_MASTER_MAGIC;

#if 0 /* This code is for eventual implementation of read/write functions/ */
	/* Open a file descriptor for read and write functions */
	path[10] = '0' + (*handle)->ctl.fd;

	(*handle)->ctl.fd = open(path, O_RDWR);
	if (-1 == (*handle)->ctl.fd) {
		vme_master_window_release(bus_handle, *handle);
		*handle = NULL;
		return -1;
	}

	return (*handle)->ctl.fd;
#endif

	return 0;
}


/*============================================================================
 * Free a previously allocated VMEbus master window handle.
 */
int vme_master_window_release(vme_bus_handle_t bus_handle,
			      vme_master_handle_t handle)
{
	int rval = 0;

	if ((NULL == handle) || (VME_MASTER_MAGIC != handle->magic)) {
		errno = EINVAL;
		return -1;
	}

#if 0 /* This code is for eventual implementation of read/write functions/ */
	close(handle->ctl.fd);
#endif

	/* Let the driver know that we are finished with this window
	 */
	rval = ioctl((int) bus_handle, VMECTL_MASTER_WINDOW_RELEASE,
		     &(handle->ctl));
	handle->magic = VME_MASTER_MAGIC_NULL;
	free(handle);

	return rval;
}


/*============================================================================
 * Return the physical address of the VMEbus master window
 */
void *vme_master_window_phys_addr(vme_bus_handle_t bus_handle,
				  vme_master_handle_t handle)
{
	if ((NULL == handle) || (VME_MASTER_MAGIC != handle->magic)) {
		errno = EINVAL;
		return NULL;
	}

	return handle->ctl.paddr;
}


/*============================================================================
 * Map the VMEbus master window to local memory.
 */
void *vme_master_window_map(vme_bus_handle_t bus_handle,
			    vme_master_handle_t handle, int flags)
{
	if ((NULL == handle) || (VME_MASTER_MAGIC != handle->magic)) {
		errno = EINVAL;
		return NULL;
	}

	return handle->vptr = vme_mmap_phys((int) bus_handle,
					    (unsigned long) handle->ctl.paddr,
					    handle->ctl.size);
}


/*============================================================================
 * Unmap the VMEbus master window.
 */
int
vme_master_window_unmap(vme_bus_handle_t bus_handle, vme_master_handle_t handle)
{
	if ((NULL == handle) || (VME_MASTER_MAGIC != handle->magic)) {
		errno = EINVAL;
		return -1;
	}

	return vme_unmap_phys((unsigned long) handle->vptr, handle->ctl.size);
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
	if ((NULL == handle) || (VME_MASTER_MAGIC != handle->magic)) {
		errno = EINVAL;
		return -1;
	}

	handle->ctl.vaddr = vme_addr;

	return ioctl((int) bus_handle, VMECTL_MASTER_WINDOW_TRANSLATE,
		     &(handle)->ctl);
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
	struct vmectl_rmw_t rmw;

	if ((NULL == handle) || (VME_MASTER_MAGIC != handle->magic)) {
		errno = EINVAL;
		return -1;
	}

	rmw.id = handle->ctl.id;
	rmw.offset = offset;
	rmw.mask = mask;
	rmw.cmp = cmp;
	rmw.swap = swap;
	rmw.dw = dw;

	return ioctl((int) bus_handle, VMECTL_RMW, &rmw);
}
