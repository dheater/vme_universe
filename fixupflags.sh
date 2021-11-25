#!/bin/sh
# This script tests for a RedHat 9 or similarly disfunctional kernel
# and outputs macros to fix the compile line.

KERNELSRC=/lib/modules/`uname -r`/build
MM_HEADER=$KERNELSRC/include/linux/mm.h

# 2.4 kernels should have 4 arguments to remap_page_range, the first being
# an unsigned int. RedHat broke this function call in the /STABLE/ kernel
# tree by adding a struct vm_area_struct as the first argument.
	if [ 4 -ge "$(uname -r | cut -d. -f2)" ]; then
		if [ -f $1 ]; then
			if [ -n "$(grep remap_page_range\ *\(\ *struct $MM_HEADER)" ];
			then echo -DRH9BRAINDAMAGE; \
			fi 
		fi
	fi
