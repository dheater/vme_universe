/*
===============================================================================
                            COPYRIGHT NOTICE

    Copyright (C) 2002-2003 GE Fanuc
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


#include <stdlib.h>
#include <sys/ioctl.h>
#include "vme/vme.h"
#include "vme/vme_api.h"


/*============================================================================
 * Set the number of retries before the PCI master interface signals an error.
 */
int vme_set_max_retry(vme_bus_handle_t handle, int maxrtry)
{
	return ioctl((int) handle, VMECTL_SET_MAX_RETRY, &maxrtry);
}


/*============================================================================
 * Get the number of retries before the PCI master interface signals an error.
 */
int vme_get_max_retry(vme_bus_handle_t handle, int *maxrtry)
{
	return ioctl((int) handle, VMECTL_GET_MAX_RETRY, maxrtry);
}


/*============================================================================
 * Set the transfer count at which the PCI slave channel posted write FIFO
 * gives up the VMEbus master interface.
 */
int vme_set_posted_write_count(vme_bus_handle_t handle, int pwon)
{
	return ioctl((int) handle, VMECTL_SET_POSTED_WRITE_COUNT, &pwon);
}


/*============================================================================
 * Return the transfer count at which the PCI slave channel posted write FIFO
 * gives up the VMEbus master interface.
 */
int vme_get_posted_write_count(vme_bus_handle_t handle, int *pwon)
{
	return ioctl((int) handle, VMECTL_GET_POSTED_WRITE_COUNT, pwon);
}


/*============================================================================
 * Set the VMEbus request level.
 */
int vme_set_bus_request_level(vme_bus_handle_t handle, int br_level)
{
	return ioctl((int) handle, VMECTL_SET_BUS_REQUEST_LEVEL, &br_level);
}


/*============================================================================
 * Return the current VMEbus request level.
 */
int vme_get_bus_request_level(vme_bus_handle_t handle, int *br_level)
{
	return ioctl((int) handle, VMECTL_GET_BUS_REQUEST_LEVEL, br_level);
}


/*============================================================================
 * Set the VMEbus request mode.
 */
int vme_set_bus_request_mode(vme_bus_handle_t handle, int br_mode)
{
	return ioctl((int) handle, VMECTL_SET_BUS_REQUEST_MODE, &br_mode);
}


/*============================================================================
 * Return the current VMEbus request mode.
 */
int vme_get_bus_request_mode(vme_bus_handle_t handle, int *br_mode)
{
	return ioctl((int) handle, VMECTL_GET_BUS_REQUEST_MODE, br_mode);
}


/*============================================================================
 * Set the VMEbus release mode.
 */
int vme_set_bus_release_mode(vme_bus_handle_t handle, int br_mode)
{
	return ioctl((int) handle, VMECTL_SET_BUS_RELEASE_MODE, &br_mode);
}


/*============================================================================
 * Return the current VMEbus release mode.
 */
int vme_get_bus_release_mode(vme_bus_handle_t handle, int *br_mode)
{
	return ioctl((int) handle, VMECTL_GET_BUS_RELEASE_MODE, br_mode);
}


/*============================================================================
 * Acquire ownership of the VMEbus.
 *
 * WARNING: Aquiring ownership is implemented with a counting semaphore. Be
 * absolutely sure to make a vme_release_bus_ownership call for every
 * vme_acquire_bus_ownership call, otherwise, the VMEbus will remain held.
 */
int vme_acquire_bus_ownership(vme_bus_handle_t handle)
{
	return ioctl((int) handle, VMECTL_ACQUIRE_BUS_OWNERSHIP, NULL);
}


/*============================================================================
 * Relinquish ownership of the VMEbus.
 */
int vme_release_bus_ownership(vme_bus_handle_t handle)
{
	return ioctl((int) handle, VMECTL_RELEASE_BUS_OWNERSHIP, NULL);
}


/*============================================================================
 * Return the current VMEbus ownership status.
 */
int vme_get_bus_ownership(vme_bus_handle_t handle, int *vown)
{
	return ioctl((int) handle, VMECTL_GET_BUS_OWNERSHIP, vown);
}


/*============================================================================
 * Set the VMEbus timeout value.
 */
int vme_set_bus_timeout(vme_bus_handle_t handle, int to)
{
	return ioctl((int) handle, VMECTL_SET_BUS_TIMEOUT, &to);
}


/*============================================================================
 * Return the current VMEbus timeout value.
 */
int vme_get_bus_timeout(vme_bus_handle_t handle, int *to)
{
	return ioctl((int) handle, VMECTL_GET_BUS_TIMEOUT, to);
}


/*============================================================================
 * Set the VMEbus arbitration mode.
 */
int vme_set_arbitration_mode(vme_bus_handle_t handle, int arb_mode)
{
	return ioctl((int) handle, VMECTL_SET_BUS_ARB_MODE, &arb_mode);
}


/*============================================================================
 * Return the current VMEbus arbitration mode.
 */
int vme_get_arbitration_mode(vme_bus_handle_t handle, int *arb_mode)
{
	return ioctl((int) handle, VMECTL_GET_BUS_ARB_MODE, arb_mode);
}


/*============================================================================
 * Set the VMEbus arbitration timeout value.
 */
int vme_set_arbitration_timeout(vme_bus_handle_t handle, int to)
{
	return ioctl((int) handle, VMECTL_SET_BUS_ARB_TIMEOUT, &to);
}


/*============================================================================
 * Return the current VMEbus arbitration timeout value.
 */
int vme_get_arbitration_timeout(vme_bus_handle_t handle, int *to)
{
	return ioctl((int) handle, VMECTL_GET_BUS_ARB_TIMEOUT, to);
}


/*============================================================================
 * Set master window endian conversion feature on or off.
 */
int vme_set_master_endian_conversion(vme_bus_handle_t handle, int endian)
{
	return ioctl((int) handle, VMECTL_SET_MEC, &endian);
}


/*============================================================================
 * Return master window endian conversion feature status.
 */
int vme_get_master_endian_conversion(vme_bus_handle_t handle, int *endian)
{
	return ioctl((int) handle, VMECTL_GET_MEC, endian);
}


/*============================================================================
 * Set the slave window endian conversion feature on or off.
 */
int vme_set_slave_endian_conversion(vme_bus_handle_t handle, int endian)
{
	return ioctl((int) handle, VMECTL_SET_SEC, &endian);
}


/*============================================================================
 * Return slave window endian conversion feature status.
 */
int vme_get_slave_endian_conversion(vme_bus_handle_t handle, int *endian)
{
	return ioctl((int) handle, VMECTL_GET_SEC, endian);
}


/*============================================================================
 * Set the endian conversion feature bypass on or off.
 */
int vme_set_endian_conversion_bypass(vme_bus_handle_t handle, int bypass)
{
	return ioctl((int) handle, VMECTL_SET_ENDIAN_BYPASS, &bypass);
}


/*============================================================================
 * Return endian conversion feature bypass status.
 */
int vme_get_endian_conversion_bypass(vme_bus_handle_t handle, int *bypass)
{
	return ioctl((int) handle, VMECTL_GET_ENDIAN_BYPASS, bypass);
}


/*============================================================================
 * Case a VMEbus sysreset to be asserted.
 */
int vme_sysreset(vme_bus_handle_t handle)
{
	return ioctl((int) handle, VMECTL_ASSERT_SYSRESET);
}
