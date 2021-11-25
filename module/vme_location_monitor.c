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
#include <linux/pci.h>
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


EXPORT_SYMBOL_NOVERS(vme_location_monitor_create);
EXPORT_SYMBOL_NOVERS(vme_location_monitor_release);

#define VME_LM_MAGIC 0x10e30003
#define VME_LM_MAGIC_NULL 0x0

struct _vme_lm_handle {
	int magic;
	void *id;
	vme_lm_handle_t next;	/* Pointer to the next handle */
	vme_lm_handle_t prev;	/* Pointer to the previous handle */
};

struct lm_module_parameter {
	unsigned int ctl;
	unsigned int vme_addr;
};


static rwlock_t lm_rwlock = RW_LOCK_UNLOCKED;
static vme_lm_handle_t handles;
static int hardcoded;		/* Flag to indicate that this window was setup
				   by module parameters, so don't muck with it!
				 */
static struct lm_module_parameter location_monitor = { 0, 0 };
extern void *universe_base;


/* Format is:
   Control register, VMEbus address
 */
MODULE_PARM(location_monitor, "2-2i");


/*============================================================================
 * Insert a location monitor handle into the location monitor handles list
 * NOTE: The calling process should already have acquired any necessary locks
 */
static inline void insert_lm_handle(vme_lm_handle_t handle)
{
	handle->next = handles;
	handle->prev = NULL;
	if (handle->next)
		handle->next->prev = handle;

	handles = handle;
}


/*============================================================================
 * Remove a location mointor handle from the location monitor handles list
 * NOTE: The calling process should already have acquired any necessary locks
 */
static inline void remove_lm_handle(vme_lm_handle_t handle)
{
	handle->magic = VME_LM_MAGIC_NULL;

	if (handle->prev)	/* This is not the first node */
		handle->prev->next = handle->next;
	else			/* This was the first node */
		handles = handle->next;

	if (handle->next)	/* If this is not the last node */
		handle->next->prev = handle->prev;

	/* If there are no handles and the window is not hardcoded we can
	   disable it.
	 */
	if (!hardcoded && (NULL == handles)) {
		DPRINTF("Disabling LM window\n");
		writel(0, universe_base + UNIV_LM_CTL);
	}
}


/*===========================================================================
 * Create the location monitor image
 */
int vme_location_monitor_create(vme_bus_handle_t bus_handle,
				vme_lm_handle_t * lm_handle, uint64_t vme_addr,
				int as, int reserved, int flags)
{
	uint32_t ctl, win_ctl;

	if (NULL == lm_handle) {
		return -EINVAL;
	}

	/* The VME_CTL_* values were carefully chosen to correspond with
	   UNIV_LM_CTL__* values. If VME_CTL_* values are changed be prepared
	   for heartache here.
	 */
	ctl = UNIV_LM_CTL__EN | (flags & (UNIV_LM_CTL__PGM__BOTH |
					  UNIV_LM_CTL__SUPER__BOTH));

	/* Default to both USER and SUPER and PROGRAM and DATA
	 */
	if (!(UNIV_LM_CTL__PGM__BOTH & ctl))
		ctl |= UNIV_LM_CTL__PGM__BOTH;

	if (!(UNIV_LM_CTL__SUPER__BOTH & ctl))
		ctl |= UNIV_LM_CTL__SUPER__BOTH;

	switch (as) {
	case VME_A32:
		ctl |= UNIV_LM_CTL__VAS__A32;
		break;
	case VME_A24:
		ctl |= UNIV_LM_CTL__VAS__A24;
		break;
	case VME_A16:
		ctl |= UNIV_LM_CTL__VAS__A16;
		break;
	default:
		DPRINTF("Invalid address space=%#x\n", as);
		return -EINVAL;
	}

	if (NULL == (*lm_handle =
		     (vme_lm_handle_t) kmalloc(sizeof (struct _vme_lm_handle),
					       GFP_KERNEL)))
		return -ENOMEM;

	(*lm_handle)->magic = VME_LM_MAGIC;
	(*lm_handle)->id = bus_handle;

	write_lock(&lm_rwlock);

	win_ctl = readl(universe_base + UNIV_LM_CTL);

	if (UNIV_LM_CTL__EN & win_ctl) {
		/* If a window is set up already, make sure we don't refuse to
		   create the window just becuase the super/user and
		   data/program modes are different. We'll jsut set the window
		   to respond to BOTH modes.
		 */
		ctl |= win_ctl & (UNIV_LM_CTL__PGM__BOTH |
				  UNIV_LM_CTL__SUPER__BOTH);
		win_ctl |= ctl & (UNIV_LM_CTL__PGM__BOTH |
				  UNIV_LM_CTL__SUPER__BOTH);

		if ((win_ctl != ctl) ||
		    (vme_addr != readl(universe_base + UNIV_LM_BS)))
        {
	        write_unlock(&lm_rwlock);
			return -EBUSY;
        }
	}

	writel(vme_addr, universe_base + UNIV_LM_BS);
	writel(ctl, universe_base + UNIV_LM_CTL);

	insert_lm_handle(*lm_handle);

	write_unlock(&lm_rwlock);

	return 0;
}


/*===========================================================================
 * Release the location monitor image
 */
int vme_location_monitor_release(vme_bus_handle_t handle,
				 vme_lm_handle_t lm_handle)
{
	if ((NULL == lm_handle) || (VME_LM_MAGIC != lm_handle->magic)) {
		return -EINVAL;
	}

	write_lock(&lm_rwlock);

	remove_lm_handle(lm_handle);

	kfree(lm_handle);

	write_unlock(&lm_rwlock);

	return 0;
}


/*============================================================================
 * Recover all vrai handles owned by the current process.
 */
void reclaim_location_monitor_handles(vme_bus_handle_t bus_handle)
{
	vme_lm_handle_t ptr, tptr;

	write_lock(&lm_rwlock);

	ptr = handles;
	while (ptr) {
		if (ptr->id == bus_handle) {
			tptr = ptr;
			ptr = ptr->next;

			remove_lm_handle(tptr);

			kfree(tptr);
		} else {
			ptr = ptr->next;
		}
	}

	write_unlock(&lm_rwlock);
}


/*===========================================================================
 * Initialize the VMEbus location monitor image
 */
int location_monitor_init(void)
{
	if (location_monitor.ctl) {
		writel(location_monitor.vme_addr, universe_base + UNIV_LM_BS);
		writel(location_monitor.ctl, universe_base + UNIV_LM_CTL);
		hardcoded = 1;
	} else {
		writel(0, universe_base + UNIV_LM_CTL);
	}

	return 0;
}


/*===========================================================================
 * Cleanup and disable the VMEbus location monitor image
 */
void location_monitor_term(void)
{
	writel(0, universe_base + UNIV_LM_CTL);
}
