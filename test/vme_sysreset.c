
/*
===============================================================================
                            COPYRIGHT NOTICE

    Copyright (C) 2001-2003 GE Fanuc
    International Copyright Secured.  All Rights Reserved.

-------------------------------------------------------------------------------
    Description: VMEbus reset example and utility
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

	if (vme_sysreset(handle)) {
		perror("vmeSysreset");
		vme_term(handle);
		return -1;
	}

	if (vme_term(handle)) {
		perror("vme_term");
		return -1;
	}

	return 0;
}
