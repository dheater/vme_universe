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
#include <string.h>
#include <sys/ioctl.h>
#include "vme/vme.h"
#include "vme/vme_api.h"

#define VME_INTERRUPT_MAGIC 0x114a0001
#define VME_INTERRUPT_MAGIC_NULL 0x0

struct _vme_interrupt_handle {
	struct vmectl_interrupt_t ctl;	/* ioctl structure, must be first */
	int magic;
};


/*============================================================================
 * Associate this process with a VME interrupt level.
 *
 * NOTES: For some interrupts, a data value is returned either by way of the
 * reply parameter for blocking interrupts, or the signal value structure
 * member if POSIX real-time signals are used. In the case of VIRQ1-7, the
 * value is the interrupt vector; in the case of a DMA interrupt, the value i
 * the status of the DGCS register; in the case of BERR, the value is the
 * address of the bus error; in the case of a mailbox interrupt the value is
 * the mailbox data.
 */
int vme_interrupt_attach(vme_bus_handle_t bus_handle,
			 vme_interrupt_handle_t * handle, int level, int vector,
			 int flags, void *data)
{
	if (NULL == handle) {
		errno = EINVAL;
		return -1;
	}

	/* Allocate the handle
	 */
	*handle = (vme_interrupt_handle_t)
	    malloc(sizeof (struct _vme_interrupt_handle));
	if (NULL == *handle)
		return -1;

	/* Populate the control structure
	 */
	(*handle)->magic = VME_INTERRUPT_MAGIC;
	(*handle)->ctl.level = level;
	(*handle)->ctl.vector = vector;
	(*handle)->ctl.flags = flags;
	if (VME_INTERRUPT_SIGEVENT == flags) {
		memcpy(&(*handle)->ctl.int_data.event, data,
		       sizeof (struct sigevent));
	}

	/* Attach the interrupt
	 */
	if (ioctl((int) bus_handle, VMECTL_INTERRUPT_ATTACH, &(*handle)->ctl)) {
		(*handle)->magic = VME_INTERRUPT_MAGIC_NULL;
		free(*handle);
		*handle = NULL;
		return -1;
	}

	/* Copy the data returned by the handler 
	 */
	if ((flags == VME_INTERRUPT_BLOCKING) && data)
		*(int *) data = (*handle)->ctl.int_data.data;

	return 0;
}


/*============================================================================
 * Uninstall an interrupt handler.
 */
int vme_interrupt_release(vme_bus_handle_t bus_handle,
			  vme_interrupt_handle_t handle)
{
	int rval;

	rval = ioctl((int) bus_handle, VMECTL_INTERRUPT_RELEASE, &handle->ctl);
	handle->magic = VME_INTERRUPT_MAGIC_NULL;
	free(handle);

	return rval;
}


/*============================================================================
 * Generate a VMEbus interrupt.
 */
int vme_interrupt_generate(vme_bus_handle_t bus_handle, int level, int vector)
{
	struct vmectl_interrupt_t interrupt;

	/* Populate the control structure
	 */
	interrupt.level = level;
	interrupt.vector = vector;

	/* Generate the interrupt
	 */
	return ioctl((int) bus_handle, VMECTL_INTERRUPT_GENERATE, &interrupt);
}
