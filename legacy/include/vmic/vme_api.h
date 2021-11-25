
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


#ifndef __LEGACY_VME_API_H
#define __LEGACY_VME_API_H

#ifdef __cplusplus
extern "C"
{
#endif


#warning Use of the header file vmic/vme_api.h has been deprecated in favor of the header file vme/vme_api.h


#include <vmic/vme.h>
#include <vme/vme_api.h>


/* Driver control functions */
  int vmeInit ();
  int vmeTerm ();
  int vmeSetMaxRetry (int maxrtry);
  int vmeGetMaxRetry (int * maxrtry);
  int vmeSetPostedWriteCount (vme_posted_write_count_t pwon);
  int vmeGetPostedWriteCount (vme_posted_write_count_t * pwon);
  int vmeSetBusRequestLevel (vme_bus_request_level_t br_level);
  int vmeGetBusRequestLevel (vme_bus_request_level_t * br_level);
  int vmeSetBusRequestMode (vme_bus_request_mode_t br_mode);
  int vmeGetBusRequestMode (vme_bus_request_mode_t * br_mode);
  int vmeSetBusReleaseMode (vme_bus_release_mode_t br_mode);
  int vmeGetBusReleaseMode (vme_bus_release_mode_t * br_mode);
  int vmeSetVown (vme_vown_t vown_val);
  int vmeGetVown (vme_vown_t * vown_val);
  int vmeSetBusTimeout (vme_bus_timeout_t to);
  int vmeGetBusTimeout (vme_bus_timeout_t * to);
  int vmeSetBusArbitrationMode (vme_bus_arbitration_mode_t arb_mode);
  int vmeGetBusArbitrationMode (vme_bus_arbitration_mode_t * arb_mode);
  int vmeSetBusArbitrationTimeout (vme_bus_arbitration_timeout_t arb_to);
  int vmeGetBusArbitrationTimeout (vme_bus_arbitration_timeout_t * arb_to);
  int vmeSetMEC (vme_endian_conversion_t endian);
  int vmeGetMEC (vme_endian_conversion_t * endian);
  int vmeSetSEC (vme_endian_conversion_t endian);
  int vmeGetSEC (vme_endian_conversion_t * endian);
  int vmeSysreset ();

/* Master window functions */
  void *vmeMapWindow (int winnum, unsigned long size);
  void *vmeMapAddr (uint64_t addr, unsigned long size, vme_addr_mod_t am,
                    vme_dwidth_t dw);

/* Slave window functions */
  void *vmeMapSlaveWindow (int winnum, unsigned long size);
  void *vmeMapSlaveAddr (uint64_t addr, unsigned long size, vme_addr_mod_t am);

/* DMA functions */
  void *vmeAllocDmaBuff (vme_dma_handle_t * handle, unsigned long nbytes,
                         int flags);
  int vmeFreeDmaBuff (vme_dma_handle_t * handle);
  int vmeReadDma (vme_dma_handle_t handle, uint64_t addr, unsigned long nelem,
                  vme_addr_mod_t am, vme_dwidth_t dw, unsigned long offset,
                  unsigned int flags, void *data);
  int vmeWriteDma (vme_dma_handle_t handle, uint64_t addr, unsigned long nelem,
                   vme_addr_mod_t am, vme_dwidth_t dw, unsigned long offset,
                   unsigned int flags, void *data);

/* Interrupt functions */
  int vmeGenerateIntr (vme_intr_lvl_t level, uint8_t vector);
  int vmeInstallIntrHandler (vme_intr_lvl_t level, uint8_t vector,
                             unsigned int flags, void *data);
  int vmeRemoveIntrHandler (vme_intr_lvl_t level, uint8_t vector);


#ifdef __cplusplus
}
#endif

#endif                          /* LEGACY_VME_API_H */
