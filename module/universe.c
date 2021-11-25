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


extern void vmic_filter_fixup(int *filter);


void *universe_base;
struct pci_dev *universe_pci_dev;
static unsigned int master_control = UNIV_MAST_CTL__MAXRTRY__512 |
    UNIV_MAST_CTL__PWON__128 | UNIV_MAST_CTL__VRL__3 |
    UNIV_MAST_CTL__VRM__DEMAND | UNIV_MAST_CTL__VREL__ON_REQ |
    UNIV_MAST_CTL__PABS__32 | UNIV_MAST_CTL__BUS_NO(0);
static unsigned int miscellaneous_control =
    UNIV_MISC_CTL__VBTO__64 | UNIV_MISC_CTL__VARB | UNIV_MISC_CTL__VARBTO__16 |
    UNIV_MISC_CTL__RESCIND;
static unsigned int filter = UNIV_U2SPEC__DS;
static int system_controller = -1;

MODULE_PARM(master_control, "i");
MODULE_PARM(miscellaneous_control, "i");
MODULE_PARM(filter, "i");
MODULE_PARM(system_controller,"i");


/*===========================================================================
 * Locate the Universe registers and initialize it
 */
int universe_init(void)
{
	uint32_t base_addr, csr, syscon;

	universe_pci_dev = pci_find_device(PCI_VENDOR_ID_TUNDRA,
					   PCI_DEVICE_ID_TUNDRA_CA91C042, NULL);
	if (NULL == universe_pci_dev) {
		printk(KERN_ERR "VME: Universe device not found\n");
		return -1;
	}

	pci_read_config_dword(universe_pci_dev, 0x10, &base_addr);
	if (base_addr & 1) {
		pci_read_config_dword(universe_pci_dev, 0x14, &base_addr);
		printk(KERN_WARNING
		       "VME: Using alternate Universe base address\n");
	}

	universe_base = ioremap_nocache(base_addr, 4096);
	if (NULL == universe_base) {
		printk(KERN_ERR
		       "VME: Failure mapping the Universe registers\n");
		return -1;
	}

	/* Initialize PCI CSR register
	 */
	csr = readl(universe_base + UNIV_PCI_CSR);
	writel(csr | UNIV_PCI_CSR__BM, universe_base + UNIV_PCI_CSR);

	/* Initialize the master control register
	 */
	writel(master_control, universe_base + UNIV_MAST_CTL);

	/* The Universe chip determines system controller as a power-up option.
	   The user has the option to request the initial state.
	 */
	switch (system_controller) {
	case 0:
		syscon = 0;
		break;
	case 1:
		syscon = UNIV_MISC_CTL__SYSCON;
		break;
	default:
		syscon = readl(universe_base + UNIV_MISC_CTL) & 
				UNIV_MISC_CTL__SYSCON;
	}

	/* Initialize the miscellaneous control register
	 */
	miscellaneous_control &= ~UNIV_MISC_CTL__SYSCON;
	writel(miscellaneous_control | syscon, universe_base + UNIV_MISC_CTL);

	syscon = readl(universe_base + UNIV_MISC_CTL) & 
				UNIV_MISC_CTL__SYSCON;
	printk(KERN_NOTICE "VME: Board is %ssystem controller\n",
	       (syscon) ? "" : "not ");

	/* Initialize the Universe II specific filters
	 */
	vmic_filter_fixup(&filter);
	writel(filter, universe_base + UNIV_U2SPEC);

	/* Disable all interrupts
	 */
	writel(0, universe_base + UNIV_LINT_EN);
	writel(0, universe_base + UNIV_VINT_EN);

	/* Map all interrupts to INTA
	 */
	writel(0, universe_base + UNIV_LINT_MAP0);
	writel(0, universe_base + UNIV_LINT_MAP1);
	writel(0, universe_base + UNIV_LINT_MAP2);

	/* Clear all pending interrupts
	 */
	writel(~0, universe_base + UNIV_LINT_STAT);
	writel(~0, universe_base + UNIV_VINT_STAT);

	return 0;
}


/*===========================================================================
 * Do any necessary cleanup on the Universe device
 */
void universe_term(void)
{
	/* Disable and clear all pending interrupts
	 */
	writel(0, universe_base + UNIV_LINT_EN);
	writel(~0, universe_base + UNIV_LINT_STAT);
	writel(0, universe_base + UNIV_VINT_EN);
	writel(~0, universe_base + UNIV_VINT_STAT);

	iounmap(universe_base);
}
