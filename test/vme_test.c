/*
===============================================================================
                            Copyright NOTICE

    Copyright (C) 2000-2003 GE Fanuc
    International Copyright Secured.  All Rights Reserved.

-------------------------------------------------------------------------------
    Description: Common functions for the Universe II example shell utilities.
-------------------------------------------------------------------------------

===============================================================================
*/

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/universe.h>
#include <vme/vme_api.h>
#include "vme_test.h"


/*===========================================================================
 * Convert an address modifier string to it's integer value
 * Returns: 0 or -1
 */
int strtoam(const char *str, int *am)
{
	char *table[] = {
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "VME_A32UMB",
		"VME_A32UD", "VME_A32UP", "VME_A32UB", "VME_A32SMB",
		"VME_A32SD", "VME_A32SP", "VME_A32SB", NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, "VME_A16U", NULL, NULL, NULL, "VME_A16S", NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		"VME_A24UMB", "VME_A24UD", "VME_A24UP", "VME_A24UB",
		"VME_A24SMB", "VME_A24SD", "VME_A24SP", "VME_A24SB"
	};
	int ii;

	for (ii = 0; ii <= VME_A24SB; ++ii)
		if (table[ii]) {
			if (0 == strcmp(str, table[ii])) {
				*am = ii;
				return 0;
			}
		}

	return -1;
}


/*===========================================================================
 * Convert an address space string to it's integer value
 * Returns: 0 or -1
 */
int strtoas(const char *str, int *as)
{
	char *table[] = {
		"VME_A16", "VME_A24", "VME_A32"
	};
	int ii;

	for (ii = 0; ii <= VME_A32; ++ii)
		if (table[ii]) {
			if (0 == strcmp(str, table[ii])) {
				*as = ii;
				return 0;
			}
		}

	return -1;
}


/*===========================================================================
 * Convert a data width string argument to a data width value
 * Returns: 0 or -1
 */
int strtodw(const char *str, int *dw)
{

	char *table[] = {
		NULL, "VME_D8", "VME_D16", NULL, "VME_D32", NULL, NULL, NULL,
		"VME_D64"
	};
	int ii;

	for (ii = 0; ii <= VME_D64; ++ii)
		if (table[ii]) {
			if (0 == strcmp(str, table[ii])) {
				*dw = ii;
				return 0;
			}
		}

	return -1;
}


/*===========================================================================
 * Convert a data width string argument to an interrupt level value
 * Returns: 0 or -1
 */
int strtolvl(const char *str, int *level)
{

	char *table[] = {
		"VME_INTERRUPT_VOWN", "VME_INTERRUPT_VIRQ1",
		"VME_INTERRUPT_VIRQ2", "VME_INTERRUPT_VIRQ3",
		"VME_INTERRUPT_VIRQ4", "VME_INTERRUPT_VIRQ5",
		"VME_INTERRUPT_VIRQ6", "VME_INTERRUPT_VIRQ7",
		"VME_INTERRUPT_DMA", "VME_INTERRUPT_LERR",
		"VME_INTERRUPT_BERR", NULL, "VME_INTERRUPT_SW_IACK",
		"VME_INTERRUPT_SW_INT", "VME_INTERRUPT_SYSFAIL",
		"VME_INTERRUPT_ACFAIL", "VME_INTERRUPT_MBOX0",
		"VME_INTERRUPT_MBOX1", "VME_INTERRUPT_MBOX2",
		"VME_INTERRUPT_MBOX3", "VME_INTERRUPT_LM0",
		"VME_INTERRUPT_LM1", "VME_INTERRUPT_LM2",
		"VME_INTERRUPT_LM3"
	};
	int ii;

	for (ii = 0; ii <= VME_INTERRUPT_LM3; ++ii)
		if (table[ii]) {
			if (0 == strcmp(str, table[ii])) {
				*level = ii;
				return 0;
			}
		}

	return -1;
}


/*===========================================================================
 * Convert a flags string to an integer value
 * Returns: 0 or -1
 */
int strtoflags(char *str, int *flags)
{
	struct __flag_table {
		char *name;
		int value;
	};

	struct __flag_table flag_table[] = {
		{"VME_INTERRUPT_BLOCKING", VME_INTERRUPT_BLOCKING},
		{"VME_INTERRUPT_SIGEVENT", VME_INTERRUPT_SIGEVENT},
		{"VME_INTERRUPT_RESERVE", VME_INTERRUPT_RESERVE},
		{"VME_CTL_PCI_IO_SPACE", VME_CTL_PCI_IO_SPACE},
		{"VME_CTL_MAX_DW_8", VME_CTL_MAX_DW_8},
		{"VME_CTL_MAX_DW_16", VME_CTL_MAX_DW_16},
		{"VME_CTL_MAX_DW_32", VME_CTL_MAX_DW_32},
		{"VME_CTL_MAX_DW_64", VME_CTL_MAX_DW_64},
		{"VME_CTL_PWEN", VME_CTL_PWEN},
		{"VME_CTL_PCI_CONFIG", VME_CTL_PCI_CONFIG},
		{"VME_CTL_RMW", VME_CTL_RMW},
		{"VME_CTL_64_BIT", VME_CTL_64_BIT},
		{"VME_CTL_USER_ONLY", VME_CTL_USER_ONLY},
		{"VME_CTL_SUPER_ONLY", VME_CTL_SUPER_ONLY},
		{"VME_CTL_DATA_ONLY", VME_CTL_DATA_ONLY},
		{"VME_CTL_PROGRAM_ONLY", VME_CTL_PROGRAM_ONLY},
		{"VME_CTL_PREN", VME_CTL_PREN},
		{"VME_CTL_EXCLUSIVE", VME_CTL_EXCLUSIVE},
		{"VME_DMA_64_BIT", VME_DMA_64_BIT},
		{"VME_DMA_VON_256", VME_DMA_VON_256},
		{"VME_DMA_VON_512", VME_DMA_VON_512},
		{"VME_DMA_VON_1024", VME_DMA_VON_1024},
		{"VME_DMA_VON_2048", VME_DMA_VON_2048},
		{"VME_DMA_VON_4096", VME_DMA_VON_4096},
		{"VME_DMA_VON_8192", VME_DMA_VON_8192},
		{"VME_DMA_VON_16384", VME_DMA_VON_16384},
		{"VME_DMA_VOFF_16", VME_DMA_VOFF_16},
		{"VME_DMA_VOFF_32", VME_DMA_VOFF_32},
		{"VME_DMA_VOFF_64", VME_DMA_VOFF_64},
		{"VME_DMA_VOFF_128", VME_DMA_VOFF_128},
		{"VME_DMA_VOFF_256", VME_DMA_VOFF_256},
		{"VME_DMA_VOFF_512", VME_DMA_VOFF_512},
		{"VME_DMA_VOFF_1024", VME_DMA_VOFF_1024},
		{"VME_DMA_VOFF_2000", VME_DMA_VOFF_2000},
		{"VME_DMA_VOFF_4000", VME_DMA_VOFF_4000},
		{"VME_DMA_VOFF_8000", VME_DMA_VOFF_8000},
		{"VME_DMA_DW_8", VME_DMA_DW_8},
		{"VME_DMA_DW_16", VME_DMA_DW_16},
		{"VME_DMA_DW_32", VME_DMA_DW_32},
		{"VME_DMA_DW_64", VME_DMA_DW_64},
		{NULL, 0}
	};
	struct __flag_table *table;
	char *token;
	int ii, jj;

	/* Strip out whitespace
	 */
	for (ii = 0, jj = 0; (str[ii] != '\0') && (str[ii] != '\n'); ++ii) {
		if (str[ii] != ' ')
			str[jj++] = str[ii];
	}

	str[jj] = '\0';		/* move the NUL terminator up */
	token = strtok(str, "|");

	*flags = 0;
	do {
		table = flag_table;
		while (NULL != table->name) {
			if (0 == strcmp(token, table->name)) {
				*flags |= table->value;
				break;
			}
			table++;
		}
	}
	while ((token = strtok(NULL, "|")));

	return 0;
}


/*===========================================================================
 * Dump the contents of data to stdout
 * Returns: 0 or -1
 */
int vdump(void *data, int nelem, int dw)
{
	const int LF = 0x10;
	int ii;

	switch (dw) {
	case VME_D8:
		{
			uint8_t *w = data;

			for (ii = 0; ii < nelem; w++)
				printf("%.2x%c", *w, (++ii % LF) ? ' ' : '\n');
		}
		break;
	case VME_D16:
		{
			uint16_t *w = data;

			for (ii = 0; ii < nelem; w++)
				printf("%.4x%c", *w,
				       (((++ii) * 2) % LF) ? ' ' : '\n');
		}
		break;
	case VME_D32:
		{
			uint32_t *w = data;

			for (ii = 0; ii < nelem; w++)
				printf("%.8x%c", *w,
				       (((++ii) * 4) % LF) ? ' ' : '\n');
		}
		break;

	case VME_D64:
		{
			uint64_t *w = data;

			for (ii = 0; ii < nelem; w++)
				printf("%.16llx%c", *w,
				       (((++ii) * 8) % LF) ? ' ' : '\n');
		}
		break;
	default:
		errno = EINVAL;
		return -1;
	}

	printf("\n");

	return 0;
}


/*===========================================================================
 * Copy data using the width specified
 * Returns: 0 or -1
 */
int vmemcpy(void *dest, const void *src, int nelem, int dw)
{
	int ii;

	switch (dw) {
	case VME_D8:
		{
			const uint8_t *s = src;
			uint8_t *d = dest;

			for (ii = 0; ii < nelem; ++ii, ++s, ++d)
				*d = *s;
		}
		break;
	case VME_D16:
		{
			const uint16_t *s = src;
			uint16_t *d = dest;

			for (ii = 0; ii < nelem; ++ii, ++s, ++d)
				*d = *s;
		}
		break;
	case VME_D32:
		{
			const uint32_t *s = src;
			uint32_t *d = dest;

			for (ii = 0; ii < nelem; ++ii, ++s, ++d)
				*d = *s;
		}
		break;
	case VME_D64:
		{
			const uint64_t *s = src;
			uint64_t *d = dest;

			for (ii = 0; ii < nelem; ++ii, ++s, ++d)
				*d = *s;
		}
		break;
	default:
		errno = EINVAL;
		return -1;
	}

	return 0;
}


/*===========================================================================
 * Get hex values from a file ans store them into a data buffer.  The input
 * values will be interpeted as hexidecimal values regardless of whether or
 * not they start with '0x'.
 * NOTE: This function allocates memory which must be freed by the user.
 * Returns: The number of data width size elements read.
 */
int vconvert_hexvals(void **data, int dw, FILE * fileptr)
{
	int ii = 0;
	size_t size;

	*data = NULL;
	size = (1 + (ii / (0x1000 / dw))) * 0x1000;

	switch (dw) {
	case VME_D8:
		{
			uint8_t *d = *data;

			while (!feof(fileptr)) {
				if (0 == ii % (0x1000 / dw)) {
					d = *data = realloc(*data, size);
					if (!d) {
						perror("realloc");
						return 0;
					}
				}

				fscanf(fileptr, "%hhx", &d[ii]);
				++ii;
			}
		}
		break;
	case VME_D16:
		{
			uint16_t *d = *data;

			while (!feof(fileptr)) {
				if (0 == ii % (0x1000 / dw)) {
					d = *data = realloc(*data, size);
					if (!d) {
						perror("realloc");
						return 0;
					}
				}

				fscanf(fileptr, "%hx", &d[ii]);
				++ii;
			}
		}
		break;
	case VME_D32:
		{
			uint32_t *d = *data;

			while (!feof(fileptr)) {
				if (0 == ii % (0x1000 / dw)) {
					d = *data = realloc(*data, size);
					if (!d) {
						perror("realloc");
						return 0;
					}
				}

				fscanf(fileptr, "%x", &d[ii]);
				++ii;
			}
		}
		break;
	case VME_D64:
		{
			uint64_t *d = *data;

			while (!feof(fileptr)) {
				if (0 == ii % (0x1000 / dw)) {
					d = *data = realloc(*data, size);
					if (!d) {
						perror("realloc");
						return 0;
					}
				}

				fscanf(fileptr, "%llx", &d[ii]);
				++ii;
			}
		}
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}

	return ii - 1;
}



/*===========================================================================
 * Get hex arguments and convert them to an array of hex values.  The input
 * values will be interpeted as hexidecimal values regardless of whether or
 * not they start with '0x'.
 * NOTE: This function allocates memory which must be freed by the user.
 * Returns: 0 or -1
 */
int vconvert_hexargs(void **data, int nelem, int dw, char **argv)
{
	int ii;

	*data = malloc(nelem * dw);
	if (NULL == *data) {
		perror("malloc");
		return -1;
	}

	switch (dw) {
	case VME_D8:
		{
			uint8_t *d = *data;

			for (ii = 0; ii < nelem; ++ii, ++d)
				*d = strtoul(argv[ii], NULL, 16);
		}
		break;
	case VME_D16:
		{
			uint16_t *d = *data;

			for (ii = 0; ii < nelem; ++ii, ++d)
				*d = strtoul(argv[ii], NULL, 16);
		}
		break;
	case VME_D32:
		{
			uint32_t *d = *data;

			for (ii = 0; ii < nelem; ++ii, ++d)
				*d = strtoul(argv[ii], NULL, 16);
		}
		break;
	case VME_D64:
		{
			uint64_t *d = *data;

			for (ii = 0; ii < nelem; ++ii, ++d)
				*d = strtoull(argv[ii], NULL, 16);
		}
		break;
	default:
		errno = EINVAL;
		return -1;
		break;
	}

	return 0;
}
