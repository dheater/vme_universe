
/*
===============================================================================
                            COPYRIGHT NOTICE

    Copyright (C) 2002-2003 GE Fanuc
    International Copyright Secured.  All Rights Reserved.

-------------------------------------------------------------------------------
    Description: VME common test utilty header
-------------------------------------------------------------------------------

===============================================================================
*/

#ifndef __VME_TEST_H
#define __VME_TEST_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


	extern int strtoam(const char *str, int *am);
	extern int strtoas(const char *str, int *as);
	extern int strtodw(const char *str, int *dw);
	extern int strtolvl(const char *str, int *level);
	extern int strtoflags(char *str, int *flags);
	extern int vdump(void *data, int nelem, int dw);
	extern int vmemcpy(void *dest, const void *src, int nelem, int dw);
	extern int vconvert_hexvals(void **data, int dw, FILE * fileptr);
	extern int vconvert_hexargs(void **data, int nelem, int dw,
				    char **argv);


#ifdef __cplusplus
}
#endif
#endif				/* VME_TEST_H */
