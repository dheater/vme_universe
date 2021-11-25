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
#include <asm/io.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include "vme/universe.h"
#include "vme/vme.h"
#include "vme/vme_api.h"
#include "vme/vmivme.h"


#ifdef DEBUG
#define DPRINTF(x...)  printk(KERN_DEBUG "VME: "x)
#else
#define DPRINTF(x...)
#endif


EXPORT_SYMBOL_NOVERS(vme_set_max_retry);
EXPORT_SYMBOL_NOVERS(vme_get_max_retry);
EXPORT_SYMBOL_NOVERS(vme_set_posted_write_count);
EXPORT_SYMBOL_NOVERS(vme_get_posted_write_count);
EXPORT_SYMBOL_NOVERS(vme_set_bus_request_level);
EXPORT_SYMBOL_NOVERS(vme_get_bus_request_level);
EXPORT_SYMBOL_NOVERS(vme_set_bus_request_mode);
EXPORT_SYMBOL_NOVERS(vme_get_bus_request_mode);
EXPORT_SYMBOL_NOVERS(vme_set_bus_release_mode);
EXPORT_SYMBOL_NOVERS(vme_get_bus_release_mode);
EXPORT_SYMBOL_NOVERS(vme_acquire_bus_ownership);
EXPORT_SYMBOL_NOVERS(vme_release_bus_ownership);
EXPORT_SYMBOL_NOVERS(vme_get_bus_ownership);
EXPORT_SYMBOL_NOVERS(vme_set_bus_timeout);
EXPORT_SYMBOL_NOVERS(vme_get_bus_timeout);
EXPORT_SYMBOL_NOVERS(vme_set_arbitration_mode);
EXPORT_SYMBOL_NOVERS(vme_get_arbitration_mode);
EXPORT_SYMBOL_NOVERS(vme_set_arbitration_timeout);
EXPORT_SYMBOL_NOVERS(vme_get_arbitration_timeout);
EXPORT_SYMBOL_NOVERS(vme_set_master_endian_conversion);
EXPORT_SYMBOL_NOVERS(vme_get_master_endian_conversion);
EXPORT_SYMBOL_NOVERS(vme_set_slave_endian_conversion);
EXPORT_SYMBOL_NOVERS(vme_get_slave_endian_conversion);
EXPORT_SYMBOL_NOVERS(vme_set_endian_conversion_bypass);
EXPORT_SYMBOL_NOVERS(vme_get_endian_conversion_bypass);
EXPORT_SYMBOL_NOVERS(vme_sysreset);


DECLARE_MUTEX(vown_lock);
rwlock_t ctl_rwlock = RW_LOCK_UNLOCKED;
extern wait_queue_head_t interrupt_wq[];
static struct proc_dir_entry *ctl_proc_entry;
static int vown_count = 0;
extern void *universe_base;
extern void *vmic_base;
extern int vmic_reg_type;


/*============================================================================
 * Hook for display proc page info
 * WARNING: If the amount of data displayed exceeds a page, then we need to
 * change how this page gets registered.
 */
static int
read_ctl_proc_page(char *buf, char **start, off_t offset, int len,
		   int *eof_unused, void *data_unused)
{
	uint32_t ctl, base;
	int nbytes = 0;

	read_lock(&ctl_rwlock);

	switch (vmic_reg_type) {
	case VMIVME_PLX:
		nbytes += sprintf(buf + nbytes, "comm=%#x\n\n",
				  readw(vmic_base + VMIVMEP_COMM));
		break;
	case VMIVME_ISA:
		nbytes += sprintf(buf + nbytes, "comm=%#x\n\n",
				  readw(vmic_base + VMIVMEI_COMM));
		break;
	case VMIVME_FPGA:
		nbytes += sprintf(buf + nbytes, "comm=%#x\n\n",
				  readl(vmic_base + VMIVMEF_COMM));
		break;
	}

	nbytes += sprintf(buf + nbytes, "master control=%#x\n\n",
			  readl(universe_base + UNIV_MAST_CTL));

	nbytes += sprintf(buf + nbytes, "miscellaneous control=%#x\n\n",
			  readl(universe_base + UNIV_MISC_CTL));

	nbytes += sprintf(buf + nbytes, "filter=%#x\n\n",
			  readl(universe_base + UNIV_U2SPEC));

	read_unlock(&ctl_rwlock);

	/* These windows are static, so no locking is required
	 */
	ctl = readl(universe_base + UNIV_VRAI_CTL);
	base = readl(universe_base + UNIV_VRAI_BS);
	if (ctl & UNIV_VRAI_CTL__EN)
		nbytes += sprintf(buf + nbytes,
				  "vrai:\n  control=%#x\n  base=%#x\n\n", ctl,
				  base);

	ctl = readl(universe_base + UNIV_LM_CTL);
	base = readl(universe_base + UNIV_LM_BS);
	if (ctl & UNIV_LM_CTL__EN)
		nbytes += sprintf(buf + nbytes, "location monitor:\n"
				  "  control=%#x\n  base=%#x\n\n", ctl, base);

	return nbytes;
}


/*============================================================================
 * Set the number of retries before the PCI master interface signals an error.
 */
int vme_set_max_retry(vme_bus_handle_t handle, int maxrtry)
{
	if (maxrtry & ~0x3c0)
		return -EINVAL;

	write_lock(&ctl_rwlock);
	writel((readl(universe_base + UNIV_MAST_CTL) & ~UNIV_MAST_CTL__MAXRTRY)
	       | (maxrtry << 22), universe_base + UNIV_MAST_CTL);
	write_unlock(&ctl_rwlock);

	return 0;
}


/*============================================================================
 * Get the number of retries before the PCI master interface signals an error.
 */
int vme_get_max_retry(vme_bus_handle_t bus_handle, int *maxrtry)
{
	read_lock(&ctl_rwlock);
	*maxrtry = (readl(universe_base + UNIV_MAST_CTL) &
		    UNIV_MAST_CTL__MAXRTRY) >> 22;
	read_unlock(&ctl_rwlock);

	return 0;
}


/*============================================================================
 * Set the transfer count at which the PCI slave channel posted write FIFO
 * gives up the VMEbus master interface.
 */
int vme_set_posted_write_count(vme_bus_handle_t handle, int pwon)
{
	switch (pwon) {
	case 0:
		pwon = UNIV_MAST_CTL__PWON__EARLY_RELEASE;
		break;
	case 4096:
		pwon = UNIV_MAST_CTL__PWON__4096;
		break;
	case 2048:
		pwon = UNIV_MAST_CTL__PWON__2048;
		break;
	case 1024:
		pwon = UNIV_MAST_CTL__PWON__1024;
		break;
	case 512:
		pwon = UNIV_MAST_CTL__PWON__512;
		break;
	case 256:
		pwon = UNIV_MAST_CTL__PWON__256;
		break;
	case 128:
		pwon = UNIV_MAST_CTL__PWON__128;
		break;
	default:
		return -EINVAL;
	}

	write_lock(&ctl_rwlock);
	writel((readl(universe_base + UNIV_MAST_CTL) & ~UNIV_MAST_CTL__PWON) |
	       pwon, universe_base + UNIV_MAST_CTL);
	write_unlock(&ctl_rwlock);

	return 0;
}


/*============================================================================
 * Return the transfer count at which the PCI slave channel posted write FIFO
 * gives up the VMEbus master interface.
 */
int vme_get_posted_write_count(vme_bus_handle_t handle, int *pwon)
{
	read_lock(&ctl_rwlock);
	*pwon = readl(universe_base + UNIV_MAST_CTL) & UNIV_MAST_CTL__PWON;
	read_unlock(&ctl_rwlock);

	switch (*pwon) {
	case UNIV_MAST_CTL__PWON__EARLY_RELEASE:
		*pwon = 0;
		break;
	case UNIV_MAST_CTL__PWON__4096:
		*pwon = 4096;
		break;
	case UNIV_MAST_CTL__PWON__2048:
		*pwon = 2048;
		break;
	case UNIV_MAST_CTL__PWON__1024:
		*pwon = 1024;
		break;
	case UNIV_MAST_CTL__PWON__512:
		*pwon = 512;
		break;
	case UNIV_MAST_CTL__PWON__256:
		*pwon = 256;
		break;
	case UNIV_MAST_CTL__PWON__128:
		*pwon = 128;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}


/*============================================================================
 * Set the VMEbus request level.
 */
int vme_set_bus_request_level(vme_bus_handle_t handle, int br_level)
{
	if (br_level & ~0x3)
		return -EINVAL;

	write_lock(&ctl_rwlock);
	writel((readl(universe_base + UNIV_MAST_CTL) & ~UNIV_MAST_CTL__VRL) |
	       (br_level << 22), universe_base + UNIV_MAST_CTL);
	write_unlock(&ctl_rwlock);

	return 0;
}


/*============================================================================
 * Return the current VMEbus request level.
 */
int vme_get_bus_request_level(vme_bus_handle_t handle, int *br_level)
{
	read_lock(&ctl_rwlock);
	*br_level = (readl(universe_base + UNIV_MAST_CTL) &
		     UNIV_MAST_CTL__VRL) >> 22;
	read_unlock(&ctl_rwlock);

	return 0;
}


/*============================================================================
 * Set the VMEbus request mode.
 */
int vme_set_bus_request_mode(vme_bus_handle_t handle, int br_mode)
{
	switch (br_mode) {
	case VME_DEMAND:
		write_lock(&ctl_rwlock);
		writel(readl(universe_base + UNIV_MAST_CTL) &
		       ~UNIV_MAST_CTL__VRM, universe_base + UNIV_MAST_CTL);
		write_unlock(&ctl_rwlock);
		break;
	case VME_FAIR:
		write_lock(&ctl_rwlock);
		writel(readl(universe_base + UNIV_MAST_CTL) |
		       UNIV_MAST_CTL__VRM, universe_base + UNIV_MAST_CTL);
		write_unlock(&ctl_rwlock);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}


/*============================================================================
 * Return the current VMEbus request mode.
 */
int vme_get_bus_request_mode(vme_bus_handle_t handle, int *br_mode)
{
	read_lock(&ctl_rwlock);
	*br_mode = (readl(universe_base + UNIV_MAST_CTL) & UNIV_MAST_CTL__VRM)
	    ? VME_FAIR : VME_DEMAND;
	read_unlock(&ctl_rwlock);

	return 0;
}


/*============================================================================
 * Set the VMEbus release mode.
 */
int vme_set_bus_release_mode(vme_bus_handle_t handle, int br_mode)
{
	switch (br_mode) {
	case VME_RELEASE_WHEN_DONE:
		write_lock(&ctl_rwlock);
		writel(readl(universe_base + UNIV_MAST_CTL) &
		       ~UNIV_MAST_CTL__VREL, universe_base + UNIV_MAST_CTL);
		write_unlock(&ctl_rwlock);
		break;
	case VME_RELEASE_ON_REQUEST:
		write_lock(&ctl_rwlock);
		writel(readl(universe_base + UNIV_MAST_CTL) |
		       UNIV_MAST_CTL__VREL, universe_base + UNIV_MAST_CTL);
		write_unlock(&ctl_rwlock);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}


/*============================================================================
 * Return the current VMEbus release mode.
 */
int vme_get_bus_release_mode(vme_bus_handle_t handle, int *br_mode)
{
	read_lock(&ctl_rwlock);
	*br_mode = (readl(universe_base + UNIV_MAST_CTL) & UNIV_MAST_CTL__VREL)
	    ? VME_RELEASE_ON_REQUEST : VME_RELEASE_WHEN_DONE;
	read_unlock(&ctl_rwlock);

	return 0;
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
	DECLARE_WAITQUEUE(wait, current);
	static int index = UNIV_INTERRUPT_INDEX(VME_INTERRUPT_VOWN, 0);

	if (down_interruptible(&vown_lock))
		return -EINTR;

	if (0 == vown_count) {
		/* Put this task on the wait queue BEFORE we set vown. When
		   the VOWN interrupt occurs, this task gets awakened.
		 */
		set_current_state(TASK_INTERRUPTIBLE);
		add_wait_queue(&interrupt_wq[index], &wait);

		write_lock(&ctl_rwlock);
		writel(readl(universe_base + UNIV_MAST_CTL) |
		       UNIV_MAST_CTL__VOWN, universe_base + UNIV_MAST_CTL);
		write_unlock(&ctl_rwlock);

		/* Go to sleep
		 */
		schedule();

		/* OK, we've been woken up and now we're ready to run. 
		 */
		set_current_state(TASK_RUNNING);
		remove_wait_queue(&interrupt_wq[index], &wait);

		/* Let's check VOWN to make sure that it is set.  If it's not,
		   then some kind of error occurred.
		 */
		read_lock(&ctl_rwlock);
		if (!(readl(universe_base + UNIV_MAST_CTL) &
		      UNIV_MAST_CTL__VOWN_ACK)) {
			read_unlock(&ctl_rwlock);
			up(&vown_lock);
			return -EIO;
		}
		read_unlock(&ctl_rwlock);
	}

	++vown_count;
	up(&vown_lock);

	return 0;
}


/*============================================================================
 * Relinquish ownership of the VMEbus.
 */
int vme_release_bus_ownership(vme_bus_handle_t handle)
{
	if (down_interruptible(&vown_lock))
		return -EINTR;

	if (0 >= --vown_count) {
		write_lock(&ctl_rwlock);
		writel(readl(universe_base + UNIV_MAST_CTL) &
		       ~UNIV_MAST_CTL__VOWN, universe_base + UNIV_MAST_CTL);
		write_unlock(&ctl_rwlock);

		/* Wait until VOWN_ACK is a value of 0 before writing a value
		   of 1 to the VOWN bit.
		 */
		read_lock(&ctl_rwlock);
		while (readl(universe_base + UNIV_MAST_CTL) &
		       UNIV_MAST_CTL__VOWN_ACK) ;
		read_unlock(&ctl_rwlock);
		vown_count = 0;
	}

	up(&vown_lock);

	return 0;
}


/*============================================================================
 * Return the current VMEbus ownership status.
 */
int vme_get_bus_ownership(vme_bus_handle_t handle, int *vown)
{
	read_lock(&ctl_rwlock);
	*vown = (readl(universe_base + UNIV_MAST_CTL) &
		 UNIV_MAST_CTL__VOWN_ACK) ? 1 : 0;
	read_unlock(&ctl_rwlock);

	return 0;
}


/*============================================================================
 * Set the VMEbus timeout value.
 */
int vme_set_bus_timeout(vme_bus_handle_t handle, int to)
{
	switch (to) {
	case 0:
		to = UNIV_MISC_CTL__VBTO__DISABLE;
		break;
	case 1024:
		to = UNIV_MISC_CTL__VBTO__1024;
		break;
	case 512:
		to = UNIV_MISC_CTL__VBTO__512;
		break;
	case 256:
		to = UNIV_MISC_CTL__VBTO__256;
		break;
	case 128:
		to = UNIV_MISC_CTL__VBTO__128;
		break;
	case 64:
		to = UNIV_MISC_CTL__VBTO__64;
		break;
	case 32:
		to = UNIV_MISC_CTL__VBTO__32;
		break;
	case 16:
		to = UNIV_MISC_CTL__VBTO__16;
		break;
	default:
		return -EINVAL;
	}

	write_lock(&ctl_rwlock);
	writel((readl(universe_base + UNIV_MISC_CTL) & ~UNIV_MISC_CTL__VBTO) |
	       to, universe_base + UNIV_MISC_CTL);
	write_unlock(&ctl_rwlock);

	return 0;
}


/*============================================================================
 * Return the current VMEbus timeout value.
 */
int vme_get_bus_timeout(vme_bus_handle_t handle, int *to)
{
	read_lock(&ctl_rwlock);
	*to = readl(universe_base + UNIV_MISC_CTL) & UNIV_MISC_CTL__VBTO;
	read_unlock(&ctl_rwlock);

	switch (*to) {
	case UNIV_MISC_CTL__VBTO__DISABLE:
		*to = 0;
		break;
	case UNIV_MISC_CTL__VBTO__1024:
		*to = 1024;
		break;
	case UNIV_MISC_CTL__VBTO__512:
		*to = 512;
		break;
	case UNIV_MISC_CTL__VBTO__256:
		*to = 256;
		break;
	case UNIV_MISC_CTL__VBTO__128:
		*to = 128;
		break;
	case UNIV_MISC_CTL__VBTO__64:
		*to = 64;
		break;
	case UNIV_MISC_CTL__VBTO__32:
		*to = 32;
		break;
	case UNIV_MISC_CTL__VBTO__16:
		*to = 16;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}


/*============================================================================
 * Set the VMEbus arbitration mode.
 */
int vme_set_arbitration_mode(vme_bus_handle_t handle, int arb_mode)
{
	switch (arb_mode) {
	case VME_ROUND_ROBIN:
		write_lock(&ctl_rwlock);
		writel(readl(universe_base + UNIV_MISC_CTL) &
		       ~UNIV_MISC_CTL__VARB, universe_base + UNIV_MISC_CTL);
		write_unlock(&ctl_rwlock);
		break;
	case VME_PRIORITY:
		write_lock(&ctl_rwlock);
		writel(readl(universe_base + UNIV_MISC_CTL) |
		       UNIV_MISC_CTL__VARB, universe_base + UNIV_MISC_CTL);
		write_unlock(&ctl_rwlock);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}


/*============================================================================
 * Return the current VMEbus arbitration mode.
 */
int vme_get_arbitration_mode(vme_bus_handle_t handle, int *arb_mode)
{
	read_lock(&ctl_rwlock);
	*arb_mode = (readl(universe_base + UNIV_MISC_CTL) &
		     UNIV_MISC_CTL__VARB) ? VME_PRIORITY : VME_ROUND_ROBIN;
	read_unlock(&ctl_rwlock);

	return 0;
}


/*============================================================================
 * Set the VMEbus arbitration timeout value.
 */
int vme_set_arbitration_timeout(vme_bus_handle_t handle, int to)
{
	switch (to) {
	case 0:
		to = UNIV_MISC_CTL__VARBTO__DISABLE;
		break;
	case 16:
		to = UNIV_MISC_CTL__VARBTO__16;
		break;
	case 256:
		to = UNIV_MISC_CTL__VARBTO__256;
		break;
	default:
		return -EINVAL;
	}

	write_lock(&ctl_rwlock);
	writel((readl(universe_base + UNIV_MISC_CTL) & ~UNIV_MISC_CTL__VARBTO) |
	       to, universe_base + UNIV_MISC_CTL);
	write_unlock(&ctl_rwlock);

	return 0;
}


/*============================================================================
 * Return the current VMEbus arbitration timeout value.
 */
int vme_get_arbitration_timeout(vme_bus_handle_t handle, int *to)
{
	read_lock(&ctl_rwlock);
	*to = readl(universe_base + UNIV_MISC_CTL) & UNIV_MISC_CTL__VARBTO;
	read_unlock(&ctl_rwlock);

	switch (*to) {
	case UNIV_MISC_CTL__VARBTO__DISABLE:
		*to = 0;
		break;
	case UNIV_MISC_CTL__VARBTO__16:
		*to = 16;
		break;
	case UNIV_MISC_CTL__VARBTO__256:
		*to = 256;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}


/*============================================================================
 * Set master window endian conversion feature on or off.
 */
int vme_set_master_endian_conversion(vme_bus_handle_t handle, int endian)
{

	switch (endian) {
	case 0:
		switch (vmic_reg_type) {
		case VMIVME_PLX:
			write_lock(&ctl_rwlock);
			writew(readw(vmic_base + VMIVMEP_COMM) &
			       ~VMIVMEP_COMM__MEC, vmic_base + VMIVMEP_COMM);
			write_unlock(&ctl_rwlock);
			break;
		case VMIVME_ISA:
			write_lock(&ctl_rwlock);
			writew(readw(vmic_base + VMIVMEI_COMM) &
			       ~VMIVMEI_COMM__MEC, vmic_base + VMIVMEI_COMM);
			write_unlock(&ctl_rwlock);
			break;
		case VMIVME_FPGA:
			write_lock(&ctl_rwlock);
			writel(readl(vmic_base + VMIVMEF_COMM) &
			       ~VMIVMEF_COMM__MEC, vmic_base + VMIVMEF_COMM);
			write_unlock(&ctl_rwlock);
			break;
		default:
			return -ENOSYS;
		}
		break;
	case 1:
		switch (vmic_reg_type) {
		case VMIVME_PLX:
			write_lock(&ctl_rwlock);
			writew(readw(vmic_base + VMIVMEP_COMM) |
			       VMIVMEP_COMM__MEC, vmic_base + VMIVMEP_COMM);
			write_unlock(&ctl_rwlock);
			break;
		case VMIVME_ISA:
			write_lock(&ctl_rwlock);
			writew(readw(vmic_base + VMIVMEI_COMM) |
			       VMIVMEI_COMM__MEC, vmic_base + VMIVMEI_COMM);
			write_unlock(&ctl_rwlock);
			break;
		case VMIVME_FPGA:
			write_lock(&ctl_rwlock);
			writel(readl(vmic_base + VMIVMEF_COMM) |
			       VMIVMEF_COMM__MEC, vmic_base + VMIVMEF_COMM);
			write_unlock(&ctl_rwlock);
			break;
		default:
			return -ENOSYS;
		}
		break;
	default:
		return -EINVAL;
	}

	return 0;
}


/*============================================================================
 * Return master window endian conversion feature status.
 */
int vme_get_master_endian_conversion(vme_bus_handle_t handle, int *endian)
{
	switch (vmic_reg_type) {
	case VMIVME_PLX:
		read_lock(&ctl_rwlock);
		*endian = (readw(vmic_base + VMIVMEP_COMM) & VMIVMEP_COMM__MEC)
		    ? 1 : 0;
		read_unlock(&ctl_rwlock);
		break;
	case VMIVME_ISA:
		read_lock(&ctl_rwlock);
		*endian = (readw(vmic_base + VMIVMEI_COMM) & VMIVMEI_COMM__MEC)
		    ? 1 : 0;
		read_unlock(&ctl_rwlock);
		break;
	case VMIVME_FPGA:
		read_lock(&ctl_rwlock);
		*endian = (readl(vmic_base + VMIVMEF_COMM) & VMIVMEF_COMM__MEC)
		    ? 1 : 0;
		read_unlock(&ctl_rwlock);
		break;
	default:
		return -ENOSYS;
	}

	return 0;
}


/*============================================================================
 * Set the slave window endian conversion feature on or off.
 */
int vme_set_slave_endian_conversion(vme_bus_handle_t handle, int endian)
{
	switch (endian) {
	case 0:
		switch (vmic_reg_type) {
		case VMIVME_PLX:
			write_lock(&ctl_rwlock);
			writew(readw(vmic_base + VMIVMEP_COMM) &
			       ~VMIVMEP_COMM__SEC, vmic_base + VMIVMEP_COMM);
			write_unlock(&ctl_rwlock);
			break;
		case VMIVME_ISA:
			write_lock(&ctl_rwlock);
			writew(readw(vmic_base + VMIVMEI_COMM) &
			       ~VMIVMEI_COMM__SEC, vmic_base + VMIVMEI_COMM);
			write_unlock(&ctl_rwlock);
			break;
		case VMIVME_FPGA:
			write_lock(&ctl_rwlock);
			writel(readl(vmic_base + VMIVMEF_COMM) &
			       ~VMIVMEF_COMM__SEC, vmic_base + VMIVMEF_COMM);
			write_unlock(&ctl_rwlock);
			break;
		default:
			return -ENOSYS;
		}
		break;
	case 1:
		switch (vmic_reg_type) {
		case VMIVME_PLX:
			write_lock(&ctl_rwlock);
			writew(readw(vmic_base + VMIVMEP_COMM) |
			       VMIVMEP_COMM__SEC, vmic_base + VMIVMEP_COMM);
			write_unlock(&ctl_rwlock);
			break;
		case VMIVME_ISA:
			write_lock(&ctl_rwlock);
			writew(readw(vmic_base + VMIVMEI_COMM) |
			       VMIVMEI_COMM__SEC, vmic_base + VMIVMEI_COMM);
			write_unlock(&ctl_rwlock);
			break;
		case VMIVME_FPGA:
			write_lock(&ctl_rwlock);
			writel(readl(vmic_base + VMIVMEF_COMM) |
			       VMIVMEF_COMM__SEC, vmic_base + VMIVMEF_COMM);
			write_unlock(&ctl_rwlock);
			break;
		default:
			return -ENOSYS;
		}
		break;
	default:
		return -EINVAL;
	}

	return 0;
}


/*============================================================================
 * Return slave window endian conversion feature status.
 */
int vme_get_slave_endian_conversion(vme_bus_handle_t handle, int *endian)
{
	switch (vmic_reg_type) {
	case VMIVME_PLX:
		read_lock(&ctl_rwlock);
		*endian = (readw(vmic_base + VMIVMEP_COMM) & VMIVMEP_COMM__SEC)
		    ? 1 : 0;
		read_unlock(&ctl_rwlock);
		break;
	case VMIVME_ISA:
		read_lock(&ctl_rwlock);
		*endian = (readw(vmic_base + VMIVMEI_COMM) & VMIVMEI_COMM__SEC)
		    ? 1 : 0;
		read_unlock(&ctl_rwlock);
		break;
	case VMIVME_FPGA:
		read_lock(&ctl_rwlock);
		*endian = (readl(vmic_base + VMIVMEF_COMM) & VMIVMEF_COMM__SEC)
		    ? 1 : 0;
		read_unlock(&ctl_rwlock);
		break;
	default:
		return -ENOSYS;
	}

	return 0;
}


/*============================================================================
 * Set the endian conversion feature bypass on or off
 */
int vme_set_endian_conversion_bypass(vme_bus_handle_t handle, int bypass)
{
	switch (bypass) {
	case 0:
		switch (vmic_reg_type) {
			/* The PLX device does not have endian conversion bypass
			 */
		case VMIVME_ISA:
			write_lock(&ctl_rwlock);
			writew(readw(vmic_base + VMIVMEI_COMM) &
			       ~VMIVMEI_COMM__BYPASS, vmic_base + VMIVMEI_COMM);
			write_unlock(&ctl_rwlock);
			break;
		case VMIVME_FPGA:
			write_lock(&ctl_rwlock);
			writel(readl(vmic_base + VMIVMEF_COMM) &
			       ~VMIVMEF_COMM__BYPASS, vmic_base + VMIVMEF_COMM);
			write_unlock(&ctl_rwlock);
			break;
		default:
			return -ENOSYS;
		}
		break;
	case 1:
		switch (vmic_reg_type) {
		case VMIVME_ISA:
			write_lock(&ctl_rwlock);
			writew(readw(vmic_base + VMIVMEI_COMM) |
			       VMIVMEI_COMM__BYPASS, vmic_base + VMIVMEI_COMM);
			write_unlock(&ctl_rwlock);
			break;
		case VMIVME_FPGA:
			write_lock(&ctl_rwlock);
			writel(readl(vmic_base + VMIVMEF_COMM) |
			       VMIVMEF_COMM__BYPASS, vmic_base + VMIVMEF_COMM);
			write_unlock(&ctl_rwlock);
			break;
		default:
			return -ENOSYS;
		}
		break;
	default:
		return -EINVAL;
	}

	return 0;
}


/*============================================================================
 * Return the endian conversion feature bypass status.
 */
int vme_get_endian_conversion_bypass(vme_bus_handle_t handle, int *bypass)
{
	switch (vmic_reg_type) {
		/* The PLX device does not have endian conversion bypass
		 */
	case VMIVME_ISA:
		read_lock(&ctl_rwlock);
		*bypass = (readw(vmic_base + VMIVMEI_COMM) &
			   VMIVMEI_COMM__BYPASS) ? 1 : 0;
		read_unlock(&ctl_rwlock);
		break;
	case VMIVME_FPGA:
		read_lock(&ctl_rwlock);
		*bypass = (readl(vmic_base + VMIVMEF_COMM) &
			   VMIVMEF_COMM__BYPASS) ? 1 : 0;
		read_unlock(&ctl_rwlock);
		break;
	default:
		return -ENOSYS;
	}

	return 0;
}


/*============================================================================
 * Case a VMEbus sysreset to be asserted.
 */
int vme_sysreset(vme_bus_handle_t handle)
{
	writel(readl(universe_base + UNIV_MISC_CTL) | UNIV_MISC_CTL__SW_SRST,
	       universe_base + UNIV_MISC_CTL);
	return 0;
}


/*===========================================================================
 * Locate the VMIC registers if present and initialize them.
 */
int ctl_init(void)
{

	if (NULL == (ctl_proc_entry = create_proc_entry("vme/ctl",
							S_IRUGO | S_IWUSR,
							NULL))) {
		printk(KERN_WARNING "VME: Failed to register ctl proc page\n");
		/* Not a fatal error */
	} else {
		ctl_proc_entry->read_proc = read_ctl_proc_page;
	}

	return 0;
}


/*===========================================================================
 * Cleanup the VMIC registers
 */
void ctl_term(void)
{
	remove_proc_entry("vme/ctl", NULL);
}
