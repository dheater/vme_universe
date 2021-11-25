
/*
===============================================================================
                            COPYRIGHT NOTICE

    Copyright (C) 2001-2003 GE Fanuc
    International Copyright Secured.  All Rights Reserved.

-------------------------------------------------------------------------------
    Description: VMEbus interrupt generation example
-------------------------------------------------------------------------------

===============================================================================
*/


#include <stdlib.h>
#include <unistd.h>
#include <vme/universe.h>
#include <vme/vme.h>
#include <vme/vme_api.h>
#include "vme_test.h"


int main(int argc, char **argv)
{
	vme_bus_handle_t bus_handle;
	int level;
	int vector;
	int c;

	/* Default values
	 */
	level = VME_INTERRUPT_VIRQ1;
	vector = 0;

	/* Parse the argument list
	 */
	while (-1 != (c = getopt(argc, argv, "l:v:")))
		switch (c) {
		case 'l':	/* Interrupt level */
			if (strtolvl(optarg, &level)) {
				fprintf(stderr, "Invalid interrupt level\n");
				return -1;
			}
			break;
		case 'v':	/* Interrupt vector */
			vector = strtol(optarg, NULL, 0);
			if (vector > 0xff) {
				fprintf(stderr, "Invalid vector");
				return -1;
			}
			break;
			break;
		default:
			fprintf(stderr, "USAGE: vme_generate_interrupt "
				"[-l level] [-v vector]");
			return -1;
		}

	/* Initialize
	 */
	if (vme_init(&bus_handle)) {
		perror("vme_init");
		return -1;
	}

	/* Generate an interrupt on the VMEbus
	 */
	if (vme_interrupt_generate(bus_handle, level, vector)) {
		perror("vme_interrupt_generate");
		vme_term(bus_handle);
		return -1;
	}

	/* Clean up and exit
	 */
	if (vme_term(bus_handle)) {
		perror("vme_term");
		return -1;
	}

	return 0;
}
