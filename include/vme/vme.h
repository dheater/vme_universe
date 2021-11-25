
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


#ifndef __VME_H
#define __VME_H


#ifndef __KERNEL__
#include <signal.h>
#include <stdint.h>
#else
#include <linux/types.h>
#include <asm/siginfo.h>
#endif


#define VME_IF_MAJOR  3
#define VME_IF_MINOR  0

/* Macro to calculate the VMEbus interface version number
 */
#define __VME_IF_VERSION(mj, mi)  (((mj) << 8) | (mi))

/* Current VMEbus interface version number
 */
#define VME_IF_VERSION  __VME_IF_VERSION(VME_IF_MAJOR, VME_IF_MINOR)


typedef enum
{
  VME_A64MB = 0x00,             /* A64 multiplex block transfer */
  VME_A64S = 0x01,              /* A64 single cycle access */
  VME_A64B = 0x03,              /* A64 block transfer */
  VME_A64LK = 0x04,             /* A64 lock command */
  VME_A32LK = 0x05,             /* A32 lock command */
  VME_A32UMB = 0x08,            /* A32 nonprivileged multiplex block
                                   transfer */
  VME_A32UD = 0x09,             /* A32 nonprivileged data access */
  VME_A32UP = 0x0A,             /* A32 nonprivileged program access */
  VME_A32UB = 0x0B,             /* A32 nonprivileged block transfer */
  VME_A32SMB = 0x0C,            /* A32 supervisory multiplex block transfer */
  VME_A32SD = 0x0D,             /* A32 supervisory data access */
  VME_A32SP = 0x0E,             /* A32 supervisory program access */
  VME_A32SB = 0x0F,             /* A32 supervisory block transfer */
  VME_A16U = 0x29,              /* A16 nonprivileged access */
  VME_A16LK = 0x2C,             /* A16 lock cycle */
  VME_A16S = 0x2D,              /* A16 supervisory access */
  VME_CR_CSR = 0x2F,            /* Configuration ROM/Sontrol & status register
                                   access */
  VME_A24LK = 0x32,             /* A24 lock command */
  VME_A40 = 0x34,               /* A40 single cycle access */
  VME_A40LK = 0x35,             /* A40 lock command */
  VME_A40B = 0x37,              /* A40 block transfer */
  VME_A24UMB = 0x38,            /* A24 nonprivileged multiplex block
                                   transfer */
  VME_A24UD = 0x39,             /* A24 nonprivileged data access */
  VME_A24UP = 0x3A,             /* A24 nonprivileged program access */
  VME_A24UB = 0x3B,             /* A24 nonprivileged block transfer */
  VME_A24SMB = 0x3C,            /* A24 supervisory multiplex block transfer */
  VME_A24SD = 0x3D,             /* A24 supervisory data access */
  VME_A24SP = 0x3E,             /* A24 supervisory program access */
  VME_A24SB = 0x3F              /* A24 supervisory block transfer */
}
vme_addr_mod_t;


typedef enum
{
  VME_A16,                      /* Short VMEbus address space */
  VME_A24,                      /* Standard VMEbus address space */
  VME_A32,                      /* Extended VMEbus address space */
  VME_A64
}
vme_address_space_t;


typedef enum
{
  VME_D8 = 1,                   /* Byte */
  VME_D16 = 2,                  /* Word */
  VME_D32 = 4,                  /* Double word */
  VME_D64 = 8                   /* Quad word */
}
vme_dwidth_t;


 /**************************************************************************
    NOTE: Many of the enumerated values below were chosen to simplify
    programming the Tundra Universe II device.  Make changes here with great
    care.
  **************************************************************************/

typedef enum
{
  VME_DEMAND,                   /* VMEbus request mode demand */
  VME_FAIR                      /* VMEbus request mode fair */
}
vme_bus_request_mode_t;


typedef enum
{
  VME_RELEASE_WHEN_DONE,        /* VMEbus release when done */
  VME_RELEASE_ON_REQUEST        /* VMEbus release on request */
}
vme_bus_release_mode_t;


typedef enum
{
  VME_ROUND_ROBIN,              /* VMEbus round-robin mode arbitration */
  VME_PRIORITY                  /* VMEbus priority mode arbitration */
}
vme_bus_arb_mode_t;


typedef enum
{
  VME_CTL_PCI_IO_SPACE = 0x000000001,   /* Also a valid slave window flag */
  VME_CTL_MAX_DW_8 = 0x00200000,
  VME_CTL_MAX_DW_16 = 0x00400000,
  VME_CTL_MAX_DW_32 = 0x00800000,
  VME_CTL_MAX_DW_64 = 0x00C00000,
  VME_CTL_EXCLUSIVE = 0x10000000,
  VME_CTL_PWEN = 0x40000000,    /* Also a valid slave window flag */
  VME_CTL_LEGACY_WINNUM = 0x80000000    /* For legacy purposes only */
}
vme_master_ctl_flags_t;


typedef enum
{
  /* VME_CTL_PCI_IO_SPACE = 0x000000001, Defined above */
  VME_CTL_PCI_CONFIG = 0x00000002,
  VME_CTL_RMW = 0x00000040,
  VME_CTL_64_BIT = 0x00000080,
  VME_CTL_USER_ONLY = 0x00100000,
  VME_CTL_SUPER_ONLY = 0x00200000,
  VME_CTL_DATA_ONLY = 0x00400000,
  VME_CTL_PROGRAM_ONLY = 0x00800000,
  VME_CTL_PREN = 0x20000000
    /* VME_CTL_PWEN = 0x40000000, Defined above */
}
vme_slave_ctl_flags_t;


typedef enum
{
  VME_DMA_64_BIT = 0x00000080,
  VME_DMA_VON_256 = 0x00001000,
  VME_DMA_VON_512 = 0x00002000,
  VME_DMA_VON_1024 = 0x00003000,
  VME_DMA_VON_2048 = 0x00004000,
  VME_DMA_VON_4096 = 0x00005000,
  VME_DMA_VON_8192 = 0x00006000,
  VME_DMA_VON_16384 = 0x00007000,
  VME_DMA_VOFF_16 = 0x00010000,
  VME_DMA_VOFF_32 = 0x00020000,
  VME_DMA_VOFF_64 = 0x00030000,
  VME_DMA_VOFF_128 = 0x00040000,
  VME_DMA_VOFF_256 = 0x00050000,
  VME_DMA_VOFF_512 = 0x00060000,
  VME_DMA_VOFF_1024 = 0x00070000,
  VME_DMA_VOFF_2000 = 0x00080000,
  VME_DMA_VOFF_4000 = 0x00090000,
  VME_DMA_VOFF_8000 = 0x000a0000,
  VME_DMA_DW_8 = 0x00200000,
  VME_DMA_DW_16 = 0x00400000,
  VME_DMA_DW_32 = 0x00800000,
  VME_DMA_DW_64 = 0x00C00000,
}
vme_dma_flags_t;


typedef enum
{
  VME_INTERRUPT_BLOCKING,       /* Interrupt attach should block */
  VME_INTERRUPT_SIGEVENT,       /* Generate a signal on interrupt */
  VME_INTERRUPT_RESERVE         /* Assume control of the interrupt level */
}
vme_interrupt_reply_t;


typedef enum
{
  VME_INTERRUPT_VOWN = 0,
  VME_INTERRUPT_VIRQ1,
  VME_INTERRUPT_VIRQ2,
  VME_INTERRUPT_VIRQ3,
  VME_INTERRUPT_VIRQ4,
  VME_INTERRUPT_VIRQ5,
  VME_INTERRUPT_VIRQ6,
  VME_INTERRUPT_VIRQ7,
  VME_INTERRUPT_DMA,
  VME_INTERRUPT_LERR,
  VME_INTERRUPT_BERR,
  VME_INTERRUPT_SW_IACK = 12,
  VME_INTERRUPT_SW_INT,
  VME_INTERRUPT_SYSFAIL,
  VME_INTERRUPT_ACFAIL,
  VME_INTERRUPT_MBOX0,
  VME_INTERRUPT_MBOX1,
  VME_INTERRUPT_MBOX2,
  VME_INTERRUPT_MBOX3,
  VME_INTERRUPT_LM0,
  VME_INTERRUPT_LM1,
  VME_INTERRUPT_LM2,
  VME_INTERRUPT_LM3
}
vme_interrupt_level_t;


struct vmectl_window_t
{
  void *id;                     /* Handle id, assigned by the driver */
  uint64_t vaddr;               /* VMEbus address */
  unsigned long size;
  int am;                       /* Address modifier */
  long flags;                   /* If 0, driver default value is used */
  void *paddr;                  /* Physical address to use */
};


struct vmectl_dma_t
{
  void *id;                     /* Handle id, assigned by the driver */
  uint64_t vaddr;               /* VMEbus address */
  unsigned long size;           /* Number of bytes to transfer */
  unsigned long offset;         /* Offset from the base physical address for
                                   transfers */
  int am;                       /* Address modifer */
  long flags;
  void *paddr;                  /* Physical address to use */
};


struct vmectl_interrupt_t
{
  void *id;                     /* Handle id, assigned by the driver */
  int level;
  int vector;
  long flags;                   /* Response type for interrupt attach.
                                   See vme_interrrupt_reply_t */
  union
  {
    int data;                   /* Data returned for VME_BLOCKING interrupt */
    struct sigevent event;      /* For use with VME_SIGEVENT flag */
  }
  int_data;
};


struct vmectl_rmw_t
{
  void *id;
  size_t offset;
  uint64_t mask;                /* Bits to enable for comparison */
  uint64_t cmp;                 /* Value to bitwise compare with */
  uint64_t swap;                /* Bit values to write if bit compare is true */
  int dw;
};


#define VME_MAGIC  0xbe
#define VMECTL_VERSION                  _IOR(VME_MAGIC, 1, long)
#define VMECTL_MASTER_WINDOW_REQUEST    \
  _IOWR(VME_MAGIC, 1, struct vmectl_window_t)
#define VMECTL_MASTER_WINDOW_RELEASE    _IOW(VME_MAGIC, 2, void *)
#define VMECTL_MASTER_WINDOW_TRANSLATE  \
  _IOW(VME_MAGIC, 3, struct vmectl_window_t)
#define VMECTL_SLAVE_WINDOW_REQUEST     \
  _IOWR(VME_MAGIC, 11, struct vmectl_window_t)
#define VMECTL_SLAVE_WINDOW_RELEASE     _IOW(VME_MAGIC, 12, void *)
#define VMECTL_DMA_BUFFER_ALLOC         \
  _IOWR(VME_MAGIC, 21, struct vmectl_dma_t)
#define VMECTL_DMA_BUFFER_FREE          _IOW(VME_MAGIC, 22, void *)
#define VMECTL_DMA_READ                 _IOW(VME_MAGIC, 23, struct vmectl_dma_t)
#define VMECTL_DMA_WRITE                _IOW(VME_MAGIC, 24, struct vmectl_dma_t)
#define VMECTL_INTERRUPT_ATTACH         \
  _IOWR(VME_MAGIC, 31, struct vmectl_interrupt_t)
#define VMECTL_INTERRUPT_RELEASE        _IOW(VME_MAGIC, 32, void *)
#define VMECTL_INTERRUPT_GENERATE       \
  _IOW(VME_MAGIC, 33, struct vmectl_interrupt_t)
#define VMECTL_ACQUIRE_BUS_OWNERSHIP    _IO(VME_MAGIC, 41)
#define VMECTL_RELEASE_BUS_OWNERSHIP    _IO(VME_MAGIC, 42)
#define VMECTL_GET_BUS_OWNERSHIP        _IOR(VME_MAGIC, 43, long)
#define VMECTL_SET_MAX_RETRY            _IOW(VME_MAGIC, 44, long)
#define VMECTL_GET_MAX_RETRY            _IOR(VME_MAGIC, 45, long)
#define VMECTL_SET_POSTED_WRITE_COUNT   _IOW(VME_MAGIC, 46, long)
#define VMECTL_GET_POSTED_WRITE_COUNT   _IOR(VME_MAGIC, 47, long)
#define VMECTL_SET_BUS_REQUEST_LEVEL    _IOW(VME_MAGIC, 48, long)
#define VMECTL_GET_BUS_REQUEST_LEVEL    _IOR(VME_MAGIC, 49, long)
#define VMECTL_SET_BUS_REQUEST_MODE     _IOW(VME_MAGIC, 50, long)
#define VMECTL_GET_BUS_REQUEST_MODE     _IOR(VME_MAGIC, 51, long)
#define VMECTL_SET_BUS_RELEASE_MODE     _IOW(VME_MAGIC, 52, long)
#define VMECTL_GET_BUS_RELEASE_MODE     _IOR(VME_MAGIC, 53, long)
#define VMECTL_SET_BUS_TIMEOUT          _IOW(VME_MAGIC, 54, long)
#define VMECTL_GET_BUS_TIMEOUT          _IOR(VME_MAGIC, 55, long)
#define VMECTL_SET_BUS_ARB_MODE         _IOW(VME_MAGIC, 56, long)
#define VMECTL_GET_BUS_ARB_MODE         _IOR(VME_MAGIC, 57, long)
#define VMECTL_SET_BUS_ARB_TIMEOUT      _IOW(VME_MAGIC, 58, long)
#define VMECTL_GET_BUS_ARB_TIMEOUT      _IOR(VME_MAGIC, 59, long)
#define VMECTL_SET_MEC                  _IOW(VME_MAGIC, 60, long)
#define VMECTL_GET_MEC                  _IOR(VME_MAGIC, 61, long)
#define VMECTL_SET_SEC                  _IOW(VME_MAGIC, 62, long)
#define VMECTL_GET_SEC                  _IOR(VME_MAGIC, 63, long)
#define VMECTL_SET_ENDIAN_BYPASS        _IOW(VME_MAGIC, 64, long)
#define VMECTL_GET_ENDIAN_BYPASS        _IOR(VME_MAGIC, 65, long)
#define VMECTL_ASSERT_SYSRESET          _IO(VME_MAGIC, 91)
#define VMECTL_RMW                      _IOW(VME_MAGIC, 92, struct vmectl_rmw_t)
#define VMECTL_VRAI_REQUEST             \
  _IOWR(VME_MAGIC, 93, struct vmectl_window_t)
#define VMECTL_VRAI_RELEASE             _IOW(VME_MAGIC, 94, void *)
#define VMECTL_LM_REQUEST               \
  _IOWR(VME_MAGIC, 95, struct vmectl_window_t)
#define VMECTL_LM_RELEASE               _IOW(VME_MAGIC, 96, void *)


#endif /* __VME_H */
