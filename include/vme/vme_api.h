
/*
===============================================================================
                            COPYRIGHT NOTICE

    Copyright (C) 2001-2002 GE Fanuc
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


#ifndef __VME_API_H
#define __VME_API_H


#ifndef __KERNEL__
#include <stdint.h>
#include <sys/types.h>
#else
#include <linux/types.h>
#endif


#ifdef __cplusplus
extern "C"
{
#endif


  typedef struct _vme_bus_handle *vme_bus_handle_t;
  typedef struct _vme_master_handle *vme_master_handle_t;
  typedef struct _vme_slave_handle *vme_slave_handle_t;
  typedef struct _vme_dma_handle *vme_dma_handle_t;
  typedef struct _vme_interrupt_handle *vme_interrupt_handle_t;
  typedef struct _vme_lm_handle *vme_lm_handle_t;
  typedef struct _vme_vrai_handle *vme_vrai_handle_t;


  extern int vme_init (vme_bus_handle_t * handle);
  extern int vme_term (vme_bus_handle_t handle);
  extern int vme_set_max_retry (vme_bus_handle_t handle, int maxretry);
  extern int vme_get_max_retry (vme_bus_handle_t handle, int *maxretry);
  extern int vme_set_posted_write_count (vme_bus_handle_t handle, int pwon);
  extern int vme_get_posted_write_count (vme_bus_handle_t handle, int *pwon);
  extern int vme_set_bus_request_level (vme_bus_handle_t handle,
                                        int br_level);
  extern int vme_get_bus_request_level (vme_bus_handle_t handle,
                                        int *br_level);
  extern int vme_set_bus_request_mode (vme_bus_handle_t handle, int br_mode);
  extern int vme_get_bus_request_mode (vme_bus_handle_t handle, int *br_mode);
  extern int vme_set_bus_release_mode (vme_bus_handle_t handle, int br_mode);
  extern int vme_get_bus_release_mode (vme_bus_handle_t handle, int *br_mode);
  extern int vme_acquire_bus_ownership (vme_bus_handle_t handle);
  extern int vme_release_bus_ownership (vme_bus_handle_t handle);
  extern int vme_get_bus_ownership (vme_bus_handle_t handle, int *vown);
  extern int vme_set_bus_timeout (vme_bus_handle_t handle, int to);
  extern int vme_get_bus_timeout (vme_bus_handle_t handle, int *to);
  extern int vme_set_arbitration_mode (vme_bus_handle_t handle, int arb_mode);
  extern int vme_get_arbitration_mode (vme_bus_handle_t handle,
                                       int *arb_mode);
  extern int vme_set_arbitration_timeout (vme_bus_handle_t handle, int to);
  extern int vme_get_arbitration_timeout (vme_bus_handle_t handle, int *to);
  extern int vme_set_master_endian_conversion (vme_bus_handle_t handle,
                                               int endian);
  extern int vme_get_master_endian_conversion (vme_bus_handle_t handle,
                                               int *endian);
  extern int vme_set_slave_endian_conversion (vme_bus_handle_t handle,
                                              int endian);
  extern int vme_get_slave_endian_conversion (vme_bus_handle_t handle,
                                              int *endian);
  extern int vme_set_endian_conversion_bypass (vme_bus_handle_t handle,
                                               int bypass);
  extern int vme_get_endian_conversion_bypass (vme_bus_handle_t handle,
                                               int *bypass);
  extern int vme_sysreset (vme_bus_handle_t handle);
  extern int vme_master_window_create (vme_bus_handle_t bus_handle,
                                       vme_master_handle_t * handle,
                                       uint64_t vme_addr, int am, size_t size,
                                       int flags, void *phys_addr);
  extern int vme_master_window_release (vme_bus_handle_t bus_handle,
                                        vme_master_handle_t handle);
  extern void *vme_master_window_phys_addr (vme_bus_handle_t bus_handle,
                                            vme_master_handle_t handle);
  extern void *vme_master_window_map (vme_bus_handle_t bus_handle,
                                      vme_master_handle_t handle, int flags);
  extern int vme_master_window_unmap (vme_bus_handle_t bus_handle,
                                      vme_master_handle_t handle);
  extern int vme_master_window_translate (vme_bus_handle_t bus_handle,
                                          vme_master_handle_t handle,
                                          uint64_t vme_addr);
  extern int vme_read_modify_write (vme_bus_handle_t bus_handle,
                                    vme_master_handle_t handle, size_t offset,
                                    int dw, uint64_t mask, uint64_t cmp,
                                    uint64_t swap);
  extern int vme_slave_window_create (vme_bus_handle_t bus_handle,
                                      vme_slave_handle_t * handle,
                                      uint64_t vme_addr, int am, size_t size,
                                      int flags, void *phys_addr);
  extern int vme_slave_window_release (vme_bus_handle_t bus_handle,
                                       vme_slave_handle_t handle);
  extern void *vme_slave_window_phys_addr (vme_bus_handle_t bus_handle,
                                           vme_slave_handle_t handle);
  extern void *vme_slave_window_map (vme_bus_handle_t bus_handle,
                                     vme_slave_handle_t handle, int flags);
  extern int vme_slave_window_unmap (vme_bus_handle_t bus_handle,
                                     vme_slave_handle_t handle);
  extern int vme_dma_buffer_create (vme_bus_handle_t bus_handle,
                                    vme_dma_handle_t * handle, size_t size,
                                    int flags, void *phys_addr);
  extern int vme_dma_buffer_release (vme_bus_handle_t bus_handle,
                                     vme_dma_handle_t handle);
  extern void *vme_dma_buffer_phys_addr (vme_bus_handle_t bus_handle,
                                         vme_dma_handle_t handle);
  extern void *vme_dma_buffer_map (vme_bus_handle_t bus_handle,
                                   vme_dma_handle_t handle, int flags);
  extern int vme_dma_buffer_unmap (vme_bus_handle_t bus_handle,
                                   vme_dma_handle_t handle);
  extern int vme_dma_read (vme_bus_handle_t bus_handle,
                           vme_dma_handle_t handle, unsigned long offset,
                           uint64_t vme_addr, int am, size_t nbytes,
                           int flags);
  extern int vme_dma_write (vme_bus_handle_t bus_handle,
                            vme_dma_handle_t handle, unsigned long offset,
                            uint64_t vme_addr, int am, size_t nbytes,
                            int flags);
  extern int vme_interrupt_attach (vme_bus_handle_t bus_handle,
                                   vme_interrupt_handle_t * handle, int level,
                                   int vector, int flags, void *data);
  extern int vme_interrupt_release (vme_bus_handle_t bus_handle,
                                    vme_interrupt_handle_t handle);
  extern int vme_interrupt_generate (vme_bus_handle_t bus_handle, int level,
                                     int vector);
  extern int vme_register_image_create (vme_bus_handle_t bus_handle,
                                        vme_vrai_handle_t * window_handle,
                                        uint64_t vme_addr, int as, int flags);
  extern int vme_register_image_release (vme_bus_handle_t bus_handle,
                                         vme_vrai_handle_t window_handle);
  extern int vme_location_monitor_create (vme_bus_handle_t bus_handle,
                                          vme_lm_handle_t * lm_handle,
                                          uint64_t vme_addr, int as,
                                          int reserved, int flags);
  extern int vme_location_monitor_release (vme_bus_handle_t bus_handle,
                                           vme_lm_handle_t lm_handle);



/* These functions are only available to kernel level code.
 */
#ifdef __KERNEL__

  extern int vme_interrupt_irq (vme_bus_handle_t bus_handle, int *irq);
  extern int vme_interrupt_enable (vme_bus_handle_t bus_handle,
                                   vme_interrupt_handle_t handle);
  extern int vme_interrupt_disable (vme_bus_handle_t bus_handle,
                                    vme_interrupt_handle_t handle);
  extern int vme_interrupt_clear (vme_bus_handle_t bus_handle,
                                  vme_interrupt_handle_t handle);
  extern int vme_interrupt_asserted (vme_bus_handle_t bus_handle,
                                     vme_interrupt_handle_t handle);
  extern int vme_interrupt_vector (vme_bus_handle_t bus_handle,
                                   vme_interrupt_handle_t handle,
                                   int *vector);

#endif                          /* __KERNEL__ */


#ifdef __cplusplus
}
#endif


#endif                          /* VME_API_H */
