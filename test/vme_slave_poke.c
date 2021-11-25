/*
===============================================================================
                            COPYRIGHT NOTICE

    Copyright (C) 2001-2003 GE Fanuc
    International Copyright Secured.  All Rights Reserved.

-------------------------------------------------------------------------------
    Description: VMEbus slave window write using memory-mapped registers
                 example and utility
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
	vme_slave_handle_t handle;
	uint64_t vme_addr;
	int as, dw, flags, use_memcpy, c, rval;
	void *phys_addr, *buffer, *addr;
	size_t nelem, size;
	FILE *datafile;

	/* Set up default values
	 */
	vme_addr = 0;
	as = VME_A32;
	dw = VME_D8;
	flags = VME_CTL_PWEN | VME_CTL_PREN;
	nelem = 1;
	phys_addr = NULL;
	use_memcpy = 1;
	datafile = stdin;
	rval = 0;

	/* Parse the argument list
	 */
	while (-1 != (c = getopt(argc, argv, "a:A:d:f:F:p:")))
		switch (c) {
		case 'a':	/* Address modifier */
			if (strtoas(optarg, &as)) {
				fprintf(stderr, "Invalid address space\n");
				return -1;
			}
			break;
		case 'A':	/* VMEbus address */
			vme_addr = strtoul(optarg, NULL, 0);
			break;
		case 'd':	/* VMEbus access data width */
			if (strtodw(optarg, &dw)) {
				fprintf(stderr, "Invalid access data width\n");
				return -1;
			}
			/* If a specific data width was requested, then don't
			   use memcpy. */
			use_memcpy = 0;
			break;
		case 'f':	/* Flags */
			if (strtoflags(optarg, &flags)) {
				fprintf(stderr, "Invalid flags\n");
				return -1;
			}
			break;
		case 'F':	/* Get data from a file */
			if (NULL == (datafile = fopen(optarg, "r"))) {
				perror("fopen");
				return -1;
			}
			break;
		case 'p':	/* Physical address */
			phys_addr = (void *) strtoul(optarg, NULL, 0);
			break;
		default:
			fprintf(stderr, "USAGE: vme_slave_poke [-a addr_mod]"
				"[-A vme_addr][-d dwidth][-f flags][-F file]"
				"[-p phys_addr][hexdata...]");
			return -1;
		}

	/* Get the data to be transfered
	 */
	if (argc > optind) {	/* Get argument from the command line */
		nelem = argc - optind;
		if (vconvert_hexargs(&buffer, nelem, dw, argv + optind)) {
			return -1;
		}
	} else {		/* Read data from file or stdin */
		nelem = vconvert_hexvals(&buffer, dw, datafile);
		if (0 == nelem) {
			fprintf(stderr, "No data to transfer\n");
			return -1;
		}
	}


	/* Use number of elements and data width to calculate the size.
	 */
	size = nelem * dw;

	/* Initialize
	 */
	if (vme_init(&bus_handle)) {
		perror("vme_init");
		rval = -1;
		goto error_init;
	}

	DPRINTF("vme_addr=0x%lx\n", (unsigned long) vme_addr);
	DPRINTF("as=0x%x\n", as);
	DPRINTF("dw=0x%x\n", dw);
	DPRINTF("size=0x%x\n", size);
	DPRINTF("flags=0x%x\n", flags);
	DPRINTF("phys_addr=0x%lx\n", (unsigned long) phys_addr);

	/* Create a handle to the necessary window
	 */
	if (vme_slave_window_create(bus_handle, &handle, vme_addr, as, size,
				    flags, phys_addr)) {
		perror("vme_slave_window_create");
		rval = -1;
		goto error_create;
	}

	/* This is not necessary, I just put this function here to test it and
	   demonstrate it's use.
	 */
	phys_addr = vme_slave_window_phys_addr(bus_handle, handle);
	if (NULL == phys_addr) {
		perror("vme_slave_window_phys_addr");
	}

	DPRINTF("Window physical address = 0x%lx\n", (unsigned long) phys_addr);

	/* Map in the window
	 */
	addr = vme_slave_window_map(bus_handle, handle, 0);
	if (NULL == addr) {
		perror("vme_slave_window_map");
		rval = -1;
		goto error_map;
	}

	/* If no data width is specified, the use memcpy to transfer the data.
	   This may result in byte-swapping though.
	 */
	if (use_memcpy) {
		memcpy(addr, buffer, size);
	} else {
		if (vmemcpy(addr, buffer, nelem, dw)) {
			perror("vmemcpy");
			rval = -1;
		}
	}

	if (vme_slave_window_unmap(bus_handle, handle)) {
		perror("vme_slave_window_unmap");
		rval = -1;
	}

      error_map:
	if (vme_slave_window_release(bus_handle, handle)) {
		perror("vme_slave_window_release");
		rval = -1;
	}

      error_create:
	if (vme_term(bus_handle)) {
		perror("vme_term");
		rval = -1;
	}

      error_init:
	free(buffer);

	return rval;
}
