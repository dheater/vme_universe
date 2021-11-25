/*
===============================================================================
                            COPYRIGHT NOTICE

    Copyright (C) 2001-2003 GE Fanuc
    International Copyright Secured.  All Rights Reserved.

-------------------------------------------------------------------------------
    Description: Acquire and hold the VMEbus utility and example
-------------------------------------------------------------------------------

===============================================================================
*/


#include <vme/vme_api.h>
#include "vme_test.h"


int main(int argc, char **argv)
{
	vme_bus_handle_t handle;

	if (vme_init(&handle)) {
		perror("vme_init");
		return -1;
	}

	/* Acquire and hold the VMEbus. Be careful to make sure that the bus
	   gets released.
	 */
	if (vme_acquire_bus_ownership(handle)) {
		perror("vme_acquire_bus_ownership");
		vme_term(handle);
		return -1;
	}

	if (vme_term(handle)) {
		perror("vme_term");
		return -1;
	}

	return 0;
}
