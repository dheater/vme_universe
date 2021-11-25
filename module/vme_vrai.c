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


EXPORT_SYMBOL_NOVERS(vme_register_image_create);
EXPORT_SYMBOL_NOVERS(vme_register_image_release);

#define VME_VRAI_MAGIC 0x10e30006
#define VME_VRAI_MAGIC_NULL 0x0


struct _vme_vrai_handle {
	int magic;
	void *id;
	vme_vrai_handle_t next;	/* Pointer to the next handle */
	vme_vrai_handle_t prev;	/* Pointer to the previous handle */
};

struct vrai_module_parameter {
	unsigned int ctl;
	unsigned int vme_addr;
};


static rwlock_t vrai_rwlock = RW_LOCK_UNLOCKED;
static vme_vrai_handle_t handles;
static int hardcoded;		/* Flag to indicate that this window was setup
				   by module parameters, so don't muck with it!
				 */
static struct vrai_module_parameter vrai = { 0, 0 };
extern void *universe_base;


/* Format is:
   Control register, VMEbus address
 */
MODULE_PARM(vrai, "2-2i");


/*============================================================================
 * Insert a vrai window handle into the vrai window handles list
 * NOTE: The calling process should already have acquired any necessary locks
 */
static inline void insert_vrai_handle(vme_vrai_handle_t handle)
{
	handle->next = handles;
	handle->prev = NULL;
	if (handle->next)
		handle->next->prev = handle;

	handles = handle;
}


/*============================================================================
 * Remove a vrai window handle from the vrai window handles list
 * NOTE: The calling process should already have acquired any necessary locks
 */
static inline void remove_vrai_handle(vme_vrai_handle_t handle)
{
	handle->magic = VME_VRAI_MAGIC_NULL;

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
		DPRINTF("Disabling VRAI window\n");
		writel(0, universe_base + UNIV_VRAI_CTL);
	}
}


/*===========================================================================
 * Create the VMEbus register access image
 */
int vme_register_image_create(vme_bus_handle_t bus_handle,
			      vme_vrai_handle_t * window_handle,
			      uint64_t vme_addr, int as, int flags)
{
	uint32_t ctl, win_ctl;

	if (NULL == window_handle) {
		return -EINVAL;
	}

	/* The VME_CTL_* values were carefully chosen to correspond with
	   UNIV_VRAI_CTL__* values.  If VME_CTL_* values are changed be
	   prepared for heartache here.
	 */
	ctl = UNIV_VRAI_CTL__EN | (flags & (UNIV_VRAI_CTL__PGM__BOTH |
					    UNIV_VRAI_CTL__SUPER__BOTH));

	/* Default to both USER and SUPER and PROGRAM and DATA
	 */
	if (!(UNIV_VRAI_CTL__PGM__BOTH & ctl))
		ctl |= UNIV_VRAI_CTL__PGM__BOTH;

	if (!(UNIV_VRAI_CTL__SUPER__BOTH & ctl))
		ctl |= UNIV_VRAI_CTL__SUPER__BOTH;

	switch (as) {
	case VME_A32:
		ctl |= UNIV_VRAI_CTL__VAS__A32;
		break;
	case VME_A24:
		ctl |= UNIV_VRAI_CTL__VAS__A24;
		break;
	case VME_A16:
		ctl |= UNIV_VRAI_CTL__VAS__A16;
		break;
	default:
		DPRINTF("Invalid address space=%#x\n", as);
		return -EINVAL;
	}

	*window_handle =
	    (vme_vrai_handle_t) kmalloc(sizeof (struct _vme_vrai_handle),
					GFP_KERNEL);
	if (NULL == *window_handle)
		return -ENOMEM;

	(*window_handle)->magic = VME_VRAI_MAGIC;
	(*window_handle)->id = bus_handle;

	write_lock(&vrai_rwlock);

	win_ctl = readl(universe_base + UNIV_VRAI_CTL);

	if (UNIV_VRAI_CTL__EN & win_ctl) {
		/* If a window is set up already, make sure we don't refuse to
		   create the window just becuase the super/user and
		   data/program modes are different. We'll just set the window
		   to respond to BOTH modes.
		 */
		ctl |= win_ctl & (UNIV_VRAI_CTL__PGM__BOTH |
				  UNIV_VRAI_CTL__SUPER__BOTH);
		win_ctl |= ctl & (UNIV_VRAI_CTL__PGM__BOTH |
				  UNIV_VRAI_CTL__SUPER__BOTH);

		if ((win_ctl != ctl) ||
		    (vme_addr != readl(universe_base + UNIV_VRAI_BS)))
        {
	        write_unlock(&vrai_rwlock);
			return -EBUSY;
        }
	}

	writel(vme_addr, universe_base + UNIV_VRAI_BS);
	writel(ctl, universe_base + UNIV_VRAI_CTL);

	insert_vrai_handle(*window_handle);

	write_unlock(&vrai_rwlock);

	return 0;
}


/*===========================================================================
 * Release the VMEbus register access image
 */
int vme_register_image_release(vme_bus_handle_t handle,
			       vme_vrai_handle_t window_handle)
{
	if ((NULL == window_handle) || (VME_VRAI_MAGIC != window_handle->magic)) {
		return -EINVAL;
	}

	write_lock(&vrai_rwlock);

	remove_vrai_handle(window_handle);
	kfree(window_handle);

	write_unlock(&vrai_rwlock);

	return 0;
}


/*============================================================================
 * Recover all vrai handles owned by the current process.
 */
void reclaim_vrai_handles(vme_bus_handle_t bus_handle)
{
	vme_vrai_handle_t ptr, tptr;

	write_lock(&vrai_rwlock);

	ptr = handles;
	while (ptr) {
		if (ptr->id == bus_handle) {
			tptr = ptr;
			ptr = ptr->next;

			remove_vrai_handle(tptr);

			kfree(tptr);
		} else {
			ptr = ptr->next;
		}
	}

	write_unlock(&vrai_rwlock);
}


/*===========================================================================
 * Initialize the VMEbus register access image
 */
int vrai_init(void)
{
	if (vrai.ctl) {
		writel(vrai.vme_addr, universe_base + UNIV_VRAI_BS);
		writel(vrai.ctl, universe_base + UNIV_VRAI_CTL);
		hardcoded = 1;
	} else {
		writel(0, universe_base + UNIV_VRAI_CTL);
	}

	return 0;
}


/*===========================================================================
 * Cleanup and disable the VMEbus register access image
 */
void vrai_term(void)
{
	writel(0, universe_base + UNIV_VRAI_CTL);
}
