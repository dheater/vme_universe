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

#ifdef CONFIG_SMP
#define __SMP__
#endif

#ifndef ARCH

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/version.h>

#else

#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif

#include <linux/types.h>   
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/ioport.h>

#endif

#include <linux/module.h>     
#include <linux/pci.h>
#include "vme/universe.h"
#include "vme/vmivme.h"

#ifdef DEBUG
#define DPRINTF(x...)  printk(KERN_DEBUG "VME: "x)
#else
#define DPRINTF(x...)
#endif


void *vmic_base;
static void *plx_base;
static unsigned int comm = 0;
static uint16_t board_id = 0;
int vmic_reg_type = VMIVME_NONE;
extern uint32_t pci_lo_bound;
extern uint32_t pci_hi_bound;


MODULE_PARM(comm, "i");


/*============================================================================
 * Find the PLX device if present, and configure the VME registers
 */
static int vmic_init_plx(void)
{
	struct pci_dev *vmic_pci_dev;
	uint32_t base_addr;

	if ((vmic_pci_dev = pci_find_device(PCI_VENDOR_ID_VMIC, 0x0001, NULL))) {
		pci_read_config_dword(vmic_pci_dev, 0x18, &base_addr);

		if (NULL == (vmic_base = ioremap_nocache(base_addr, 12))) {
			printk(KERN_ERR
			       "VME: Failure mapping the VMIC registers\n");
			return -1;
		}

		pci_read_config_dword(vmic_pci_dev, 0x10, &base_addr);

		if (NULL == (plx_base = ioremap_nocache(base_addr, 0x80))) {
			printk(KERN_ERR
			       "VME: Failure mapping the VMIC registers\n");
			return -1;
		}

		/* Set VMIC comm register
		 */
		if (comm)
			writew(comm, vmic_base + VMIVMEP_COMM);
		else		/* default */
			writew(VMIVMEP_COMM__MEC | VMIVMEP_COMM__SEC |
			       VMIVMEP_COMM__ABLE | VMIVMEP_COMM__VME_EN,
			       vmic_base + VMIVMEP_COMM);

		/* Clear any pending BERR
		 */
		writel(readl(vmic_base + VMIVMEP_B_INT_STATUS) |
		       VMIVMEP_B_INT_STATUS__BERR_S,
		       vmic_base + VMIVMEP_B_INT_STATUS);

		/* Enable the BERR interrupt
		 */
		writew(VMIVMEP_B_INT_MASK__BERR_M,
		       vmic_base + VMIVMEP_B_INT_MASK);
		writel(readl(plx_base + VMIVMEP_CSR) | VMIVMEP_CSR__INTR_EN,
		       plx_base + VMIVMEP_CSR);

		/* Set the hi and lo available addresses for dynamic window
		   creation if they have not already been specified.
		 */
		if (0 == pci_lo_bound)
			pci_lo_bound = 0x04000000;
		if (~0 == pci_hi_bound)
			pci_hi_bound = 0xfffeffff;

		return 0;
	}

	return -1;
}


/*============================================================================
 * Clean up the VMIC PLX registers prior to exit
 */
static void vmic_term_plx(void)
{
	/* Disable the BERR interrupt
	 */
	writew(readl(vmic_base + VMIVMEP_B_INT_MASK) &
	       ~VMIVMEP_B_INT_MASK__BERR_M, vmic_base + VMIVMEP_B_INT_MASK);
	writel(readl(plx_base + VMIVMEP_CSR) & ~VMIVMEP_CSR__INTR_EN,
	       plx_base + VMIVMEP_CSR);

	iounmap(plx_base);
}


/*============================================================================
 * Find the FPGA device if present, and configure the VME registers
 */
static int vmic_init_fpga(void)
{
	struct pci_dev *vmic_pci_dev;
	uint32_t base_addr;

	if ((vmic_pci_dev = pci_find_device(PCI_VENDOR_ID_VMIC, 0x0004, NULL))
	    || (vmic_pci_dev =
		pci_find_device(PCI_VENDOR_ID_VMIC, 0x0005, NULL))
	    || (vmic_pci_dev =
		pci_find_device(PCI_VENDOR_ID_VMIC, 0x0006, NULL))) {
		pci_read_config_dword(vmic_pci_dev, 0x10, &base_addr);

		if (NULL == (vmic_base = ioremap_nocache(base_addr, 12))) {
			printk(KERN_ERR
			       "VME: Failure mapping the VMIC registers\n");
			return -1;
		}

		pci_read_config_word(vmic_pci_dev, PCI_SUBSYSTEM_ID,
				     (uint16_t *) & board_id);
		DPRINTF("Board is VMIVME-%x\n", board_id);

		/* Clear any pending BERR
		 */
		writel(readl(vmic_base + VMIVMEF_COMM) | VMIVMEF_COMM__BERRST,
		       vmic_base + VMIVMEF_COMM);

		/* Set VMIC comm register
		 */
		if (comm)
			writel(comm, vmic_base + VMIVMEF_COMM);
		else		/* default */
			writel(VMIVMEF_COMM__BERRI | VMIVMEF_COMM__MEC |
			       VMIVMEF_COMM__SEC | VMIVMEF_COMM__ABLE |
			       VMIVMEF_COMM__VME_EN, vmic_base + VMIVMEF_COMM);

		/* Set the hi and lo available addresses for dynamic window
		   creation if they have not already been specified.
		 */
		if (0 == pci_lo_bound)
			pci_lo_bound = 0x04000000;
		if (~0 == pci_hi_bound)
			pci_hi_bound = 0xfffeffff;

		return 0;
	}

	return -1;
}


/*============================================================================
 * Clean up the VMIC FPGA registers prior to exit
 */
static void vmic_term_fpga(void)
{
	/* Disable the BERR interrupt
	 */
	writel(readl(vmic_base + VMIVMEF_COMM) & ~VMIVMEF_COMM__BERRI,
	       vmic_base + VMIVMEF_COMM);
}


/*============================================================================
 * Find the ISA device if present, and configure the VME registers
 */
static int vmic_init_isa(void)
{
	if (NULL == (vmic_base = ioremap_nocache(VMIVMEI_BASE, 0x18))) {
		printk(KERN_ERR "VME: Failure mapping the VMIC registers\n");
		return -1;
	}

	/* Test to see if this board has extension registers in I/O space by
	   looking for a valid board id.  Starting with the 7750, these
	   registers have moved to the FPGA part, so hopefully, this list will
	   remain complete.
	 */
	board_id = readw(vmic_base + VMIVMEI_BID);
	switch (board_id) {
	case 0x7591:
	case 0x7592:
	case 0x7695:
	case 0x7696:
	case 0x7697:
	case 0x7698:
	case 0x7740:
		DPRINTF("Board is VMIVME-%x\n", board_id);
		break;
	default:
		iounmap(vmic_base);

		/* Make sure vmic_base is NULL so the cleanup function doesn't
		   try to unmap memory that it shouldn't.
		 */
		vmic_base = NULL;
		return -1;
		break;
	}

	/* Clear any pending BERR
	 */
	writew(readw(vmic_base + VMIVMEI_COMM) | VMIVMEI_COMM__BERRST,
	       vmic_base + VMIVMEI_COMM);

	/* Set VMIC comm register
	 */
	if (comm)
		writew(comm, vmic_base + VMIVMEI_COMM);
	else			/* default */
		writew(VMIVMEI_COMM__BERRI | VMIVMEI_COMM__MEC |
		       VMIVMEI_COMM__SEC | VMIVMEI_COMM__ABLE |
		       VMIVMEI_COMM__VME_EN, vmic_base + VMIVMEI_COMM);

	/* Set the hi and lo available addresses for dynamic window creation if
	   they have not already been specified.
	 */
	if (0 == pci_lo_bound)
		pci_lo_bound = 0x04000000;
	if (~0 == pci_hi_bound)
		pci_hi_bound = 0xfffeffff;

	return 0;
}


/*============================================================================
 * Clean up the VMIC ISA registers prior to exit
 */
static void vmic_term_isa(void)
{
	/* Disable the BERR interrupt
	 */
	writew(readw(vmic_base + VMIVMEI_COMM) & ~VMIVMEI_COMM__BERRI,
	       vmic_base + VMIVMEI_COMM);
}


/*===========================================================================
 * Determine if the filter registers settings need to be fixed up.
 * In days long ago a Universe errata triggered the addition of a filter
 * circuit on DS. That errata was fixed by Tundra by adding a filter
 * internal to the Universe chip. However, for boards that still have the
 * external filter, we do not turn on the DS filter to prevent any ill
 * effects from double-filtering.
 */
void vmic_filter_fixup(int *filter)
{
	switch (board_id) {
	case 0x7589:
	case 0x7591:
	case 0x7592:
	case 0x7695:
	case 0x7696:
	case 0x7697:
	case 0x7698:
	case 0x7740:
	case 0x7765:
	case 0x7766:
		*filter &= ~UNIV_U2SPEC__DS;
		break;
	}
}


/*===========================================================================
 * Locate the VMIC registers if present and initialize them.
 */
int vmic_init(void)
{

	if (0 == vmic_init_plx()) {
		vmic_reg_type = VMIVME_PLX;
		DPRINTF("VMIC PLX device found and initialized\n");
		return 0;
	}

	if (0 == vmic_init_fpga()) {
		vmic_reg_type = VMIVME_FPGA;
		DPRINTF("VMIC FPGA device found and initialized\n");
		return 0;
	}

#ifdef ARCH
#ifdef CONFIG_ISA
	if (0 == vmic_init_isa()) {
		vmic_reg_type = VMIVME_ISA;
		DPRINTF("VMIC ISA device found and initialized\n");
		return 0;
	}
#endif
#else
	if (0 == vmic_init_isa()) {
		vmic_reg_type = VMIVME_ISA;
		DPRINTF("VMIC ISA device found and initialized\n");
		return 0;
	}
#endif

	/* Not finding a VMIC board is OK; Theoretically, this driver should
	   support any Tundra Universe II based board.  There are a couple of
	   features (like endian conversion), that won't work.
	 */
	DPRINTF("Not a supported VMIC board.\n");

	return 0;
}


/*===========================================================================
 * Cleanup the VMIC registers
 */
void vmic_term(void)
{

	if (vmic_base) {
		switch (vmic_reg_type) {
		case VMIVME_PLX:
			vmic_term_plx();
			DPRINTF("VMIC PLX device termination complete\n");
			break;
		case VMIVME_FPGA:
			vmic_term_fpga();
			DPRINTF("VMIC FPGA device termination complete\n");
			break;
		case VMIVME_ISA:
			vmic_term_isa();
			DPRINTF("VMIC ISA device termination complete\n");
			break;
		}
		iounmap(vmic_base);
	}
}
