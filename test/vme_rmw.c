/*
===============================================================================
                            COPYRIGHT NOTICE

    Copyright (C) 2001-2003 GE Fanuc
    International Copyright Secured.  All Rights Reserved.

-------------------------------------------------------------------------------
    Description: Perform a VMEbus read-modify-write cycle
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
	vme_master_handle_t handle;
	uint64_t vme_addr, mask, cmp, swap;
	int am, dw, flags, c, rval;

	/* Set up default values
	 */
	vme_addr = 0;
	am = VME_A32SD;
	dw = VME_D32;
	flags = VME_CTL_PWEN | VME_CTL_EXCLUSIVE;
	rval = 0;

	/* Parse the argument list
	 */
	while (-1 != (c = getopt(argc, argv, "a:A:d:f:")))
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
				fprintf(stderr, "Invalid access data width\n");
				return -1;
			}
			break;
		case 'f':	/* Flags */
			if (strtoflags(optarg, &flags)) {
				fprintf(stderr, "Invalid flags\n");
				return -1;
			}
			break;
		default:
			fprintf(stderr, "USAGE: vme_rmw [-a addr_mod]"
				"[-A vme_addr][-d dwidth][-f flags] mask cmp "
				"swap");
			return -1;
		}

	/* We should still have the mask, cmp, and swap parameters to parse
	 */
	if (3 != argc - optind) {
		fprintf(stderr, "USAGE: vme_rtp [-a addr_mod][-A vme_addr]"
			"[-d dwidth][-f flags] mask cmp swap");
		return -1;
	}

	mask = strtoul(argv[optind++], NULL, 0);
	cmp = strtoul(argv[optind++], NULL, 0);
	swap = strtoul(argv[optind++], NULL, 0);

	if (vme_init(&bus_handle)) {
		perror("vme_init");
		return -1;
	}

	DPRINTF("vme_addr=0x%lx\n", (unsigned long) vme_addr);
	DPRINTF("am=0x%x\n", am);
	DPRINTF("dw=0x%x\n", dw);
	DPRINTF("flags=0x%x\n", flags);
	DPRINTF("mask=0x%lx\n", (unsigned long) mask);
	DPRINTF("cmp=0x%lx\n", (unsigned long) cmp);
	DPRINTF("swap=0x%lx\n", (unsigned long) swap);

	if (vme_master_window_create(bus_handle, &handle, vme_addr, am, dw,
				     flags, NULL)) {
		perror("vme_master_window_create");
		rval = -1;
		goto error_create;
	}

	if (vme_read_modify_write(bus_handle, handle, 0, dw, mask, cmp, swap)) {
		perror("vme_read_modify_write");
		rval = -1;
	}

	if (vme_master_window_release(bus_handle, handle)) {
		perror("vme_master_window_release");
		rval = -1;
	}

      error_create:
	if (vme_term(bus_handle)) {
		perror("vme_term");
		rval = -1;
	}

	return rval;
}
