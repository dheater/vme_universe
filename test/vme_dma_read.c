/*
===============================================================================
                            COPYRIGHT NOTICE

    Copyright (C) 2001-2003 GE Fanuc
    International Copyright Secured.  All Rights Reserved.

-------------------------------------------------------------------------------
    Description: VMEbus DMA read example and utility
-------------------------------------------------------------------------------

===============================================================================
*/


#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>
#include "vme_test.h"


#ifdef DEBUG
#define DPRINTF(x...)  fprintf(stderr, x)
#else
#define DPRINTF(x...)
#endif


int main(int argc, char **argv)
{
	vme_bus_handle_t bus_handle;
	vme_dma_handle_t handle;
	uint64_t vme_addr;
	int am, dw, flags, c, rval;
	void *phys_addr, *addr;
	size_t nelem, size;

	/* Set up default values
	 */
	vme_addr = 0;
	am = VME_A32SD;
	dw = VME_D8;
	flags = 0;
	nelem = 1;
	phys_addr = NULL;
	rval = 0;

	/* Parse the argument list
	 */
	while (-1 != (c = getopt(argc, argv, "a:A:d:e:f:p:")))
		switch (c) {
		case 'a':	/* Address modifier */
			if (strtoam(optarg, &am)) {
				fprintf(stderr, "Invalid address modifier\n");
				return -1;
			}
			break;
		case 'A':	/* VMEbus address */
			vme_addr = strtoul(optarg, NULL, 0);
			break;
		case 'd':	/* VMEbus access data width */
			if (strtodw(optarg, &dw)) {
				fprintf(stderr, "Invalid data width\n");
				return -1;
			}
			break;
		case 'e':	/* Number of elements */
			nelem = strtoul(optarg, NULL, 0);
			break;
		case 'f':	/* Flags */
			if (strtoflags(optarg, &flags)) {
				fprintf(stderr, "Invalid flags\n");
				return -1;
			}
			break;
		case 'p':	/* Physical address */
			phys_addr = (void *) strtoul(optarg, NULL, 0);
			break;
			break;
		default:
			fprintf(stderr, "USAGE: vme_dma_read [-a addr_mod] "
				"[-A vme_addr][-d dwidth][-e num_elem]"
				"[-f flags][-p phys_addr]");
			return -1;
		}

	/* Use number of elements and data width to calculate the size.
	 */
	size = nelem * dw;

	/* Initialize
	 */
	if (vme_init(&bus_handle)) {
		perror("vme_init");
		return -1;
	}

	DPRINTF("vme_addr=0x%lx\n", (unsigned long) vme_addr);
	DPRINTF("am=0x%x\n", am);
	DPRINTF("dw=0x%x\n", dw);
	DPRINTF("size=0x%x\n", size);
	DPRINTF("flags=0x%x\n", flags);
	DPRINTF("phys_addr=0x%lx\n", (unsigned long) phys_addr);

	/* Create DMA buffer
	 */
	if (vme_dma_buffer_create(bus_handle, &handle, size, 0, phys_addr)) {
		perror("vme_dma_buffer_create");
		rval = -1;
		goto error_create;
	}

	/* This is not necessary, I just put this function here to test it and
	   demonstrate it's use.
	 */
	phys_addr = vme_dma_buffer_phys_addr(bus_handle, handle);
	if (NULL == phys_addr) {
		perror("vme_dma_buffer_phys_addr");
	}

	DPRINTF("Buffer physical address = 0x%lx\n", (unsigned long) phys_addr);

	/* Map in the window
	 */
	addr = vme_dma_buffer_map(bus_handle, handle, 0);
	if (NULL == addr) {
		perror("vme_dma_buffer_map");
		rval = -1;
		goto error_map;
	}

	/* Do the transfer
	 */
	if (vme_dma_read(bus_handle, handle, 0, vme_addr, am, size, flags)) {
		perror("vme_dma_read");
		rval = -1;
		goto error_read;
	}

	/* Print data to stdout
	 */
	if (vdump(addr, nelem, dw)) {
		perror("vdump");
		rval = -1;
	}

      error_read:
	if (vme_dma_buffer_unmap(bus_handle, handle)) {
		perror("vme_dma_buffer_unmap");
		rval = -1;
	}

      error_map:
	if (vme_dma_buffer_release(bus_handle, handle)) {
		perror("vme_dma_buffer_release");
		rval = -1;
	}

      error_create:
	if (vme_term(bus_handle)) {
		perror("vme_term");
		rval = -1;
	}

	return rval;
}
