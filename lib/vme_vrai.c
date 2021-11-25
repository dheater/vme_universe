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


/*===========================================================================
 * Create the VMEbus register access image
 */
int vme_register_image_create(vme_bus_handle_t bus_handle,
			      vme_vrai_handle_t * window_handle,
			      uint64_t vme_addr, int as, int flags)
{
	struct vmectl_window_t window;
	int rval;

	if (NULL == window_handle) {
		errno = EINVAL;
		return -1;
	}

	window.vaddr = vme_addr;
	window.am = as;
	window.flags = flags;

	rval = ioctl((int) bus_handle, VMECTL_VRAI_REQUEST, &window);
	*window_handle = window.id;

	return rval;
}


/*===========================================================================
 * Release the VMEbus register access image
 */
int vme_register_image_release(vme_bus_handle_t bus_handle,
			       vme_vrai_handle_t window_handle)
{
	return ioctl((int) bus_handle, VMECTL_VRAI_RELEASE, &window_handle);
}
