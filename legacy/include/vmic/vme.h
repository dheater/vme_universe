
/*
===============================================================================
                            COPYRIGHT NOTICE

    Copyright (C) 2002 GE Fanuc
    International Copyright Secured.  All Rights Reserved.

-------------------------------------------------------------------------------

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

   o Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.
   o Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.
   o Neither the name of GE Fanuc nor the names of its contributors may be used
     to endorse or promote products derived from this software without specific
     prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

===============================================================================
*/


#ifndef __LEGACY_VME_H
#define __LEGACY_VME_H


#ifdef __cplusplus
extern "C"
{

#endif                          /* __cplusplus */


#warning Use of the header file vmic/vme.h has been deprecated in favor of the header file vme/vme.h


#ifndef __KERNEL__
#include <signal.h>
#include <stdint.h>
#endif                          /* ! __KERNEL__ */

#include <linux/ioctl.h>
#include <vmic/universe.h>


  /*
   * Types of interrupt notification
   */
#define V_BLOCKING        0
#define V_SIGEVENT        0x10000000


/*
 * Macros for converting the return data from an interrupt back to level and
 * vector
 */
#define SIGINFO_TO_LVL_VEC( siginfo, level, vector ) \
level = ( siginfo->si_value.sival_int & 0xff00 ) >> 8; \
vector = siginfo->si_value.sival_int & 0xff

#define INTDATA_TO_LVL_VEC( intdata, level, vector ) \
level = ( ( intdata ) & 0xff00 ) >> 8; \
vector = ( intdata ) & 0xff

#define SIGINFO_TO_MBVAL( siginfo, mbval ) \
mbval = siginfo->si_value.sival_int & 0xff

#define INTDATA_TO_MBVAL( intdata, mbval ) \
mbval = ( intdata ) & 0xff

#define SIGINFO_TO_DMASTAT( siginfo, dmastat ) \
dmastat = siginfo->si_value.sival_int

#define SIGINFO_TO_BERR_ADDR( siginfo, addr ) \
addr = siginfo->si_value.sival_int


#ifdef __cplusplus
}

#endif                          /* __cplusplus */

#endif                          /* __LEGACY_VME_H */
