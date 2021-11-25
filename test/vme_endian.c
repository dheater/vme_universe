
/*
===============================================================================
                            COPYRIGHT NOTICE

    Copyright (C) 2002-2003 GE Fanuc
    International Copyright Secured.  All Rights Reserved.

-------------------------------------------------------------------------------
    Description: VMEbus example to set endian conversion mode
-------------------------------------------------------------------------------

===============================================================================
*/


#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <vme/vme_api.h>
#include "vme_test.h"


static int print_status(vme_bus_handle_t bus_handle)
{
	int endian;

	if (vme_get_master_endian_conversion(bus_handle, &endian)) {
		if (ENOSYS == errno) {
			fprintf(stdout, "Master endian conversion is not "
				"implemented\n");
		} else {
			perror("vme_get_master_endian_conversion");
			return -1;
		}
	}

	fprintf(stdout, "Master endian conversion is %s\n",
		(endian) ? "on" : "off");

	if (vme_get_slave_endian_conversion(bus_handle, &endian)) {
		if (ENOSYS == errno) {
			fprintf(stdout, "Slave endian conversion is not "
				"implemented\n");
		} else {
			perror("vme_get_slave_endian_conversion");
			return -1;
		}
	}

	fprintf(stdout, "Slave endian conversion is %s\n",
		(endian) ? "on" : "off");

	if (vme_get_endian_conversion_bypass(bus_handle, &endian)) {
		if (ENOSYS == errno) {
			fprintf(stdout, "Endain conversion cannot be "
				"bypassed\n");
		} else {
			perror("vme_get_endian_conversion_bypass");
			return -1;
		}
	}

	fprintf(stdout, "Endian conversion is %s\n",
		(endian) ? "bypassed" : "enabled");

	return 0;
}


static int set_status(vme_bus_handle_t bus_handle, int argc, char **argv)
{
	int c, endian;

	while (-1 != (c = getopt(argc, argv, "b:m:s:"))) {
		switch (c) {
		case 'b':	/* Endian conversion bypass */
			endian = strtol(optarg, NULL, 0);
			if (vme_set_endian_conversion_bypass(bus_handle,
							     endian)) {
				perror("vme_set_endian_conversion_bypass");
				return -1;
			}
			break;
		case 'm':	/* Master endian conversion */
			endian = strtol(optarg, NULL, 0);
			if (vme_set_master_endian_conversion
			    (bus_handle, endian)) {
				perror("vme_set_master_endian_conversion");
				return -1;
			}
			break;
		case 's':	/* Slave endian conversion */
			endian = strtol(optarg, NULL, 0);
			if (vme_set_slave_endian_conversion(bus_handle, endian)) {
				perror("vme_set_slave_endian_conversion");
				return -1;
			}
			break;
		default:
			fprintf(stderr, "USAGE: vme_endian [-b value]"
				"[-m value][-s value]");
			return -1;
		}
	}

	return 0;
}


int main(int argc, char **argv)
{
	vme_bus_handle_t bus_handle;
	int rval;

	if (vme_init(&bus_handle)) {
		perror("vme_init");
		return -1;
	}

	/* If there are no arguments, print the current endian conversion
	   hardware setup, else set the requested values.
	 */
	if (1 == argc) {
		rval = print_status(bus_handle);
	} else {
		rval = set_status(bus_handle, argc, argv);
	}

	if (vme_term(bus_handle)) {
		perror("vme_term");
		rval = -1;
	}

	return rval;
}
