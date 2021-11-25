
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
   o Neither the name of GE Fanuc nor the names of its contributors may be used      to endorse or promote products derived from this software without specific
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


 /* Notes on the implementation of the legacy VMEbus interface:
    We create and map resources here, but since the Linux 2.x driver did not
    dynamically allocate resources, the API provided no way to free the
    resources.  The resource will continue to exist until the application exits.
    The driver knows to free all resources granted to a process when vmeTerm()
    is called, or when the process terminates.
  */

#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/stddef.h>
#include <sys/mman.h>
#include <vmic/vme_api.h>


#define UNIVERSE_DEVICE  "/dev/bus/vme/ctl"


static vme_interrupt_handle_t interrupt[UNIV_IRQS] = { 0 };
static vme_bus_handle_t bus_handle;


/*============================================================================
 * Initialize the VMEbus driver interface
 */
int                             /* Returns 0 on success or -1 on failure */
vmeInit ()
{
  return (vme_init (&bus_handle));
}


/*============================================================================
 * Cleanup the VMEbus driver interface
 */
int                             /* Returns a positive int on success or -1 
                                   on failure */
vmeTerm ()
{
  return (vme_term (bus_handle));
}


/*****************************************************************************
 * Set the number of retries before the PCI master interface signals an error
 */
int                             /* 0 on success, -1 on failure */
vmeSetMaxRetry (int maxrtry     /* Number of retry attempts. This value must
                                   be a multiple of 64 between the
                                   values of 0 and 960. The value 0
                                   means retry forever */
  )
{
  return (vme_set_max_retry (bus_handle, maxrtry));
}


/*****************************************************************************
 * Get the number of retries before the PCI master interface signals an error
 */
int                             /* 0 on success, -1 on failure */
vmeGetMaxRetry (int *maxrtry /* Number of retry attempts */ )
{
  return (vme_get_max_retry (bus_handle, maxrtry));
}


/*****************************************************************************
 * Set the transfer count at which the PCI slave channel posted write FIFO
 * gives up the VMEbus master interface
 */
int                             /* 0 on success, -1 on failure */
vmeSetPostedWriteCount (vme_posted_write_count_t pwon /* Transfer count */ )
{
  return (vme_set_posted_write_count (bus_handle, pwon));
}


/*****************************************************************************
 * Set the transfer count at which the PCI slave channel posted write FIFO
 * gives up the VMEbus master interface
 */
int                             /* 0 on success, -1 on failure */
vmeGetPostedWriteCount (vme_posted_write_count_t * pwon /* Transfer count */ )
{
  return (vme_get_posted_write_count (bus_handle, (int *) pwon));
}


/*****************************************************************************
 * Set the VMEbus request level
 */
int                             /* 0 on success, -1 on failure */
vmeSetBusRequestLevel (vme_bus_request_level_t br_level /* VMEbus request
                                                           level */
  )
{
  return (vme_set_bus_request_level (bus_handle, br_level));
}


/*****************************************************************************
 * Get the VMEbus request level
 */
int                             /* 0 on success, -1 on failure */
vmeGetBusRequestLevel (vme_bus_request_level_t * br_level       /* VMEbus
                                                                   request
                                                                   level */
  )
{
  return (vme_get_bus_request_level (bus_handle, (int *) br_level));
}


/*****************************************************************************
 * Set the VMEbus request mode
 */
int                             /* 0 on success, -1 on failure */
vmeSetBusRequestMode (vme_bus_request_mode_t br_mode    /* VMEbus request
                                                           mode */
  )
{
  return (vme_set_bus_request_mode (bus_handle, br_mode));
}


/*****************************************************************************
 * Get the VMEbus request mode
 */
int                             /* 0 on success, -1 on failure */
vmeGetBusRequestMode (vme_bus_request_mode_t * br_mode  /* VMEbus request
                                                           mode */
  )
{
  return (vme_get_bus_request_mode (bus_handle, (int *) br_mode));
}


/*****************************************************************************
 * Set the VMEbus release mode
 */
int                             /* 0 on success, -1 on failure */
vmeSetBusReleaseMode (vme_bus_release_mode_t br_mode    /* VMEbus release
                                                           mode */
  )
{
  return (vme_set_bus_release_mode (bus_handle, br_mode));
}


/*****************************************************************************
 * Get the VMEbus release mode
 */
int                             /* 0 on success, -1 on failure */
vmeGetBusReleaseMode (vme_bus_release_mode_t * br_mode  /* VMEbus release 
                                                           mode */
  )
{
  return (vme_get_bus_release_mode (bus_handle, (int *) br_mode));
}


/*****************************************************************************
 * Set the state of VMEbus ownership
 */
int                             /* 0 on success, -1 on failure */
vmeSetVown (vme_vown_t vown /* VMEbus ownership state */ )
{
  if (vown)
    return (vme_acquire_bus_ownership (bus_handle));
  else
    return (vme_release_bus_ownership (bus_handle));
}


/*****************************************************************************
 * Get the state of VMEbus ownership
 */
int                             /* 0 on success, -1 on failure */
vmeGetVown (vme_vown_t * vown /* VMEbus ownership state */ )
{
  return (vme_get_bus_ownership (bus_handle, (int *) vown));
}


/*****************************************************************************
 * Set the VMEbus timeout value
 */
int                             /* 0 on success, -1 on failure */
vmeSetBusTimeout (vme_bus_timeout_t to /* VMEbus timeout value */ )
{
  return (vme_set_bus_timeout (bus_handle, to));
}


/*****************************************************************************
 * Get the VMEbus timeout value
 */
int                             /* 0 on success, -1 on failure */
vmeGetBusTimeout (vme_bus_timeout_t * to /* VMEbus timeout value */ )
{
  return (vme_get_bus_timeout (bus_handle, (int *) to));
}


/*****************************************************************************
 * Set the VMEbus arbitration mode
 */
int                             /* 0 on success, -1 on failure */
vmeSetBusArbitrationMode (vme_bus_arbitration_mode_t arb_mode   /* VMEbus
                                                                   arbitration
                                                                   mode */
  )
{
  return (vme_set_arbitration_mode (bus_handle, arb_mode));
}


/*****************************************************************************
 * Get the VMEbus arbitration mode
 */
int                             /* 0 on success, -1 on failure */
vmeGetBusArbitrationMode (vme_bus_arbitration_mode_t * arb_mode /* VMEbus
                                                                   arbitration
                                                                   mode */
  )
{
  return (vme_get_arbitration_mode (bus_handle, (int *) arb_mode));
}


/*****************************************************************************
 * Set the VMEbus arbitration timeout value
 */
int                             /* 0 on success, -1 on failure */
vmeSetBusArbitrationTimeout (vme_bus_arbitration_timeout_t to   /* VMEbus
                                                                   arbitration
                                                                   timeout */
  )
{
  return (vme_set_arbitration_timeout (bus_handle, to));
}


/*****************************************************************************
 * Get the VMEbus arbitration timeout value
 */
int                             /* 0 on success, -1 on failure */
vmeGetBusArbitrationTimeout (vme_bus_arbitration_timeout_t * to /* VMEbus
                                                                   arbitration
                                                                   timeout */
  )
{
  return (vme_get_arbitration_timeout (bus_handle, (int *) to));
}


/*****************************************************************************
 * Set the hardware master window endian control register
 */
int                             /* 0 on success, -1 on failure */
vmeSetMEC (vme_endian_conversion_t endian /* hardware endian control */ )
{
  return (vme_set_master_endian_conversion (bus_handle, endian));
}


/*****************************************************************************
 * Get the state of hardware master window endian control register
 */
int                             /* 0 on success, -1 on failure */
vmeGetMEC (vme_endian_conversion_t * endian /* hardware endian control */ )
{
  return (vme_get_master_endian_conversion (bus_handle, (int *) endian));
}


/*****************************************************************************
 * Set the hardware slave window endian control register
 */
int                             /* 0 on success, -1 on failure */
vmeSetSEC (vme_endian_conversion_t endian /* hardware endian control */ )
{
  return (vme_set_slave_endian_conversion (bus_handle, endian));
}


/*****************************************************************************
 * Get the state of hardware slave window endian control register
 */
int                             /* 0 on success, -1 on failure */
vmeGetSEC (vme_endian_conversion_t * endian /* hardware endian control */ )
{
  return (vme_get_slave_endian_conversion (bus_handle, (int *) endian));
}


/*****************************************************************************
 * Assert a VMEbus sysreset
 */
int                             /* 0 on success, -1 on failure */
vmeSysreset ()
{
  return (vme_sysreset (bus_handle));
}


/*============================================================================
 * Get a pointer into a VMEbus master window conforming to the input parameters 
 */
void *                          /* Pointer to VMEbus master window, or NULL on
                                   error */
vmeMapAddr (uint64_t addr,      /* Address on the VMEbus */
            unsigned long size, /* Minimum size of the window from addr */
            vme_addr_mod_t am,  /* VMEbus address modifier */
            vme_dwidth_t dw     /* Maximum data width supported by the window.
                                   All accesses to the bus will be made at this 
                                   data width or smaller */
  )
{
  vme_master_handle_t handle;
  int flags = VME_CTL_PWEN;
  void *ptr;

  /* The new Tundra Universe driver does not tolerate address modifiers with
     inconsistant data widths.  i.e. The Universe chip generates MBLT cycles if
     the data width is 64-bit, whether you specified BLT's or not.  For that
     reason, this function must fix the call parameters so old code does not
     fail this new check.
   */
  switch (am)
    {
    case VME_A24UMB:
    case VME_A24SMB:
    case VME_A32UMB:
    case VME_A32SMB:
      dw = VME_D64;
      break;
    default:
      if (VME_D64 == dw)
        dw = VME_D32;
    }

  switch (dw)
    {
    case VME_D8:
      flags |= VME_CTL_MAX_DW_8;
      break;
    case VME_D16:
      flags |= VME_CTL_MAX_DW_16;
      break;
    case VME_D32:
      flags |= VME_CTL_MAX_DW_32;
      break;
    case VME_D64:
      flags |= VME_CTL_MAX_DW_64;
      break;
    }

  if (0 >
      vme_master_window_create (bus_handle, &handle, addr, am, size, flags,
                                NULL))
    return (NULL);

  ptr = vme_master_window_map (bus_handle, handle, 0);

  free (handle);

  return (ptr);
}


/*============================================================================
 * Get a pointer to the base address of a VMEbus master window
 */
void *                          /* Pointer to VMEbus master window, or NULL on
                                   error */
vmeMapWindow (int winnum,       /* Window number to map */
              unsigned long size        /* Size of the region to map */
  )
{
  vme_master_handle_t handle;
  void *ptr;

  /* The window number is passed as the VMEbus address. We supply a dummy
     address modifier.  The flag VME_CTL_LEGACY_WINNUM lets the driver know
     our intentions.
   */
  if (0 >
      vme_master_window_create (bus_handle, &handle, winnum,
                                VME_A32SD, size,
                                VME_CTL_PWEN | VME_CTL_LEGACY_WINNUM, NULL))
    return (NULL);

  ptr = vme_master_window_map (bus_handle, handle, 0);

  free (handle);

  return (ptr);
}


/*============================================================================
 * Get a pointer to a VMEbus slave window conforming to the input parameters
 */
void *                          /* Pointer to VMEbus slave memory, or NULL on
                                   error */
vmeMapSlaveAddr (uint64_t addr, /* Address on the VMEbus */
                 unsigned long size,    /* Minimum size of the window from addr */
                 vme_addr_mod_t am      /* VMEbus address modifier */
  )
{
  vme_slave_handle_t handle;
  int as;
  void *ptr;

  /* Convert the address modifier to an address space
   */
  switch (0xF0 & am)
    {
    case 0x00:
      as = VME_A32;
      break;
    case 0x20:
      as = VME_A16;
      break;
    case 0x30:
      as = VME_A24;
      break;
    default:
      errno = EINVAL;
      return (NULL);
    }

  if (0 >
      vme_slave_window_create (bus_handle, &handle, addr, as, size,
                               VME_CTL_PWEN | VME_CTL_PREN, NULL))
    return (NULL);

  ptr = vme_slave_window_map (bus_handle, handle, 0);

  free (handle);

  return (ptr);
}


/*============================================================================
 * Get a pointer to the base address of the VMEbus slave window
 */
void *                          /* Pointer to VMEbus slave memory, or NULL on
                                   error */
vmeMapSlaveWindow (int winnum,  /* Window number to map */
                   unsigned long size   /* Size of the region to map */
  )
{
  vme_slave_handle_t handle;
  void *ptr;

  /* The window number is passed as the VMEbus address. We supply a dummy
     address modifier.  The flag VME_CTL_LEGACY_WINNUM lets the driver know
     our intentions.
   */
  if (0 >
      vme_slave_window_create (bus_handle, &handle, winnum,
                               VME_A32SD, size,
                               VME_CTL_PWEN | VME_CTL_PREN |
                               VME_CTL_LEGACY_WINNUM, NULL))
    return (NULL);

  ptr = vme_slave_window_map (bus_handle, handle, 0);

  free (handle);

  return (ptr);
}


/*============================================================================
 * Allocate memory for DMA transfer.
 */
void *                          /* Pointer the specified region, or NULL on
                                   failure */
vmeAllocDmaBuff (vme_dma_handle_t * handle,     /* Pointer to a handle for the
                                                   allocated DMA memory */
                 unsigned long nbytes,  /* Number of bytes to allocate and map */
                 int flags      /* There are currently no flags defined, use 0 */
  )
{

  if (0 > vme_dma_buffer_create (bus_handle, handle, nbytes, 0, NULL))
    return (NULL);

  return (vme_dma_buffer_map (bus_handle, *handle, 0));
}


/*============================================================================
 * Free allocated DMA memory.
 */
int                             /* 0 on success -1 on failure */
vmeFreeDmaBuff (vme_dma_handle_t * handle       /* Pointer to a handle for the
                                                   allocated DMA memory */
  )
{
  int rval = 0;

  rval += vme_dma_buffer_unmap (bus_handle, *handle);
  rval += vme_dma_buffer_release (bus_handle, *handle);

  return ((rval) ? -1 : 0);
}


/*============================================================================
 * Map in allocated DMA memory. 
 */
void *                          /* Pointer the specified region, or NULL on
                                   failure */
vmeMapDmaBuff (vme_dma_handle_t handle, /* Handle to a DMA buffer allocated by
                                           vmeAllocDmaBuff */
               uint32_t size,   /* Size of the region to map. */
               uint32_t offset  /* Offset from the base of the DMA buffer to
                                   begin mapping. */
  )
{
  return (vme_dma_buffer_map (bus_handle, handle, offset));
}


/*============================================================================
 * Read data from the specified VMEbus address into the DMA buffer at offset
 */
int                             /* Returns 0 on success or -1 on failure */
vmeReadDma (vme_dma_handle_t handle,    /* Handle to allocated DMA memory */
            uint64_t addr,      /* Address on the VMEbus to read from */
            unsigned long nelem,        /* Number of elements of data width
                                           sized elements to read. */
            vme_addr_mod_t am,  /* VMEbus address modifier */
            vme_dwidth_t dw,    /* Transfer data width */
            unsigned long offset,       /* Offset into the DMA buffer */
            unsigned int flags, /* The following constants may be OR'ed
                                   together to control the DMA operation:

                                   - DMA_LD64EN - enable 64-bit PCI transactions

                                   One of the following settings of the VON
                                   counter:
                                   - DMA_VON_UNITL_DONE
                                   - DMA_VON_256BYTE
                                   - DMA_VON_512BYTE
                                   - DMA_VON_1024BYTE
                                   - DMA_VON_2048BYTE
                                   - DMA_VON_4096BYTES
                                   - DMA_VON_8192BYTES
                                   - DMA_VON_16384BYTES

                                   One of the following settings of the VOFF
                                   counter:
                                   - DMA_VOFF_0US
                                   - DMA_VOFF_16US
                                   - DMA_VOFF_32US
                                   - DMA_VOFF_64US
                                   - DMA_VOFF_128US
                                   - DMA_VOFF_256US
                                   - DMA_VOFF_512US
                                   - DMA_VOFF_1024US
                                   - DMA_VOFF_2US
                                   - DMA_VOFF_4US
                                   - DMA_VOFF_8US
                                 */
            void *data          /* Pass in struct sigevent if this is a
                                   non-blocking call. Returns state of the
                                   DGCS register for blocking calls */
  )
{

  /* Non-blocking DMAs are not currently supported by the new driver.
   */
  if (DMA_BLOCKING & flags)
    {
      flags &= ~DMA_BLOCKING;
    }
  else
    {
      errno = EINVAL;
      return (-1);
    }

  /* Massage the flags to fit the new API
   */
  if (0x00700000 & flags)
    {
      flags |= (0x0070000 & flags) >> 8;
      flags &= ~0x0070000;
    }

  /* The new Tundra Universe driver does not tolerate address modifiers with
     inconsistant data widths.  i.e. The Universe chip generates MBLT cycles if
     the data width is 64-bit, whether you specified BLT's or not.  For that
     reason, this function must fix the call parameters so old code does not
     fail this new check.
   */
  switch (am)
    {
    case VME_A24UMB:
    case VME_A24SMB:
    case VME_A32UMB:
    case VME_A32SMB:
      dw = VME_D64;
      break;
    default:
      if (VME_D64 == dw)
        dw = VME_D32;
    }

  switch (dw)
    {
    case VME_D16:
      flags |= VME_DMA_DW_16;
      break;
    case VME_D32:
      flags |= VME_DMA_DW_32;
      break;
    case VME_D64:
      flags |= VME_DMA_DW_64;
      break;
    default:
      errno = EINVAL;
      return (-1);
    }

  /* The new driver does not return the DGCS register so we fake it out here.
   */
  *(int *) data = UNIV_DGCS__DONE;

  if (0 >
      (vme_dma_read
       (bus_handle, handle, offset, addr, am, nelem * dw, flags)))
    {
      *(int *) data = UNIV_DGCS__VERR;
      return (-1);
    }

  return (0);
}


/*============================================================================
 * Write data from the DMA buffer at offset to the specified VMEbus address
 */
int                             /* Returns 0 on success or -1 on failure */
vmeWriteDma (vme_dma_handle_t handle,   /* Handle to allocated DMA memory */
             uint64_t addr,     /* Address on the VMEbust to read from */
             unsigned long nelem,       /* Number of elements of data width sized
                                           elements to read. */
             vme_addr_mod_t am, /* VMEbus address modifier */
             vme_dwidth_t dw,   /* Transfer data width */
             unsigned long offset,      /* Offset into the DMA buffer */
             unsigned int flags,        /* The following constants may be OR'ed
                                           together to control the DMA
                                           operation:

                                           - DMA_LD64EN - enable 64-bit PCI
                                           transactions

                                           One of the following settings of
                                           the VON counter:
                                           - DMA_VON_UNITL_DONE
                                           - DMA_VON_256BYTE
                                           - DMA_VON_512BYTE
                                           - DMA_VON_1024BYTE
                                           - DMA_VON_2048BYTE
                                           - DMA_VON_4096BYTES
                                           - DMA_VON_8192BYTES
                                           - DMA_VON_16384BYTES

                                           One of the following settings of
                                           the VOFF counter:
                                           - DMA_VOFF_0US
                                           - DMA_VOFF_16US
                                           - DMA_VOFF_32US
                                           - DMA_VOFF_64US
                                           - DMA_VOFF_128US
                                           - DMA_VOFF_256US
                                           - DMA_VOFF_512US
                                           - DMA_VOFF_1024US
                                           - DMA_VOFF_2US
                                           - DMA_VOFF_4US
                                           - DMA_VOFF_8US
                                         */
             void *data         /* Pass in struct sigevent if this is a
                                   non-blocking call. Returns state of the
                                   DGCS register for blocking calls */
  )
{

  /* Non-blocking DMAs are not currently supported by the new driver.
   */
  if (DMA_BLOCKING & flags)
    {
      flags &= ~DMA_BLOCKING;
    }
  else
    {
      errno = EINVAL;
      return (-1);
    }

  /* Massage the flags to fit the new API
   */
  if (0x00700000 & flags)
    {
      flags |= (0x0070000 & flags) >> 8;
      flags &= ~0x0070000;
    }

  /* The new Tundra Universe driver does not tolerate address modifiers with
     inconsistant data widths.  i.e. The Universe chip generates MBLT cycles if
     the data width is 64-bit, whether you specified BLT's or not.  For that
     reason, this function must fix the call parameters so old code does not
     fail this new check.
   */
  switch (am)
    {
    case VME_A24UMB:
    case VME_A24SMB:
    case VME_A32UMB:
    case VME_A32SMB:
      dw = VME_D64;
      break;
    default:
      if (VME_D64 == dw)
        dw = VME_D32;
    }

  switch (dw)
    {
    case VME_D16:
      flags |= VME_DMA_DW_16;
      break;
    case VME_D32:
      flags |= VME_DMA_DW_32;
      break;
    case VME_D64:
      flags |= VME_DMA_DW_64;
      break;
    default:
      errno = EINVAL;
      return (-1);
    }

  /* The new driver does not return the DGCS register so we fake it out here.
   */
  *(int *) data = UNIV_DGCS__DONE;

  if (0 >
      (vme_dma_write
       (bus_handle, handle, offset, addr, am, nelem * dw, flags)))
    {
      *(int *) data = UNIV_DGCS__VERR;
      return (-1);
    }

  return (0);
}


/*****************************************************************************
 * Install an interrupt handler
 */
int                             /* 0 on success, -1 on failure */
vmeInstallIntrHandler (vme_intr_lvl_t level,    /* Interrupt level */
                       uint8_t vector,  /* Interrupt vector, used for VMEbus
                                           interrupts only, otherwise use 0 */
                       unsigned int flags,      /* One of the following:
                                                   - V_BLOCKING,
                                                   - V_SIGEVENT */
                       void *data       /* For V_SIGEVENT this is an event
                                           of type struct sigaction.

                                           For V_BLOCKING, this is a pointer
                                           to the data value to be returned.
                                           For Mailbox interrupts, data is the
                                           data read from the mailbox register.
                                           For DMA interrupts, it is the value
                                           of the DGCS register.  For VMEbus
                                           errors, it is the address of the
                                           access that caused the error */
  )
{
  int index = UNIV_INTERRUPT_INDEX (level, vector);

  if (interrupt[index])
    {
      errno = EBUSY;
      return (-1);
    }

  if ((SIGRTMAX >= flags) && (0 < flags))
    {
      /* Handle legacy flags;  Flags used to be the signal number. */
      struct sigevent event;

      event.sigev_signo = flags;
      event.sigev_notify = SIGEV_SIGNAL;

      return (vme_interrupt_attach
              (bus_handle, &interrupt[index], level, vector,
               VME_INTERRUPT_SIGEVENT, &event));
    }

  if (V_SIGEVENT == flags)
    return (vme_interrupt_attach
            (bus_handle, &interrupt[index], level, vector,
             VME_INTERRUPT_SIGEVENT, data));

  return (vme_interrupt_attach
          (bus_handle, &interrupt[index], level, vector,
           VME_INTERRUPT_BLOCKING, data));
}


/*****************************************************************************
 * Uninstall an interrupt handler
 */
int                             /* 0 on success, -1 on failure */
vmeRemoveIntrHandler (vme_intr_lvl_t level,     /* Interrupt level */
                      uint8_t vector    /* Interrupt vector.  Only used for
                                           VMEbus interrupts, otherwise pass 0 */
  )
{
  int index = UNIV_INTERRUPT_INDEX (level, vector);
  int status;

  status = vme_interrupt_release (bus_handle, interrupt[index]);
  interrupt[index] = 0;

  return (status);
}


/*****************************************************************************
 * Cancel a blocking call to vmeInstallIntrHandler()
 */
int                             /* 0 on success, -1 on failure */
vmeCancelIntrHandler (vme_intr_lvl_t level,     /* Interrupt level */
                      uint8_t vector    /* Interrupt vector.  Only used for
                                           VMEbus interrupts. Otherwise use 0 */
  )
{
  return (vmeRemoveIntrHandler (level, vector));
}


/*****************************************************************************
 * Generate an interrupt onto the VMEbus
 */
int                             /* 0 on success, -1 on failure */
vmeGenerateIntr (vme_intr_lvl_t level,  /* Interrupt level */
                 uint8_t vector /* Interrupt vector. */
  )
{
  return (vme_interrupt_generate (bus_handle, level, vector));
}
