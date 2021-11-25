
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


#ifndef __LEGACY_UNIVERSE_H
#define __LEGACY_UNIVERSE_H

#ifdef __cplusplus
extern "C"
{

#endif                          /* __cplusplus */


#warning Use of the header file vmic/universe.h has been deprecated in favor of the header file vme/universe.h


#include <vme/universe.h>
#include <vme/vme.h>


#ifndef PAGESIZE
#define PAGESIZE                   0x1000
#endif

  /* 
   *  Universe PCI device definitions 
   */
#define UNIVERSE_VENDOR_ID          0x10e3
#define UNIVERSE_DEVICE_ID          0x0

#define UNIVERSE_BASE_ADDR_REG      0x10
#define UNIVERSE_ALT_BASE_ADDR_REG  0x14

  /* 
   *  VME window definitions 
   */
#define MAX_VME_MASTER_WINDOWS      8
#define MAX_VME_SLAVE_WINDOWS       8

  /* 
   *  Universe device registers 
   */
#define PCI_CSR    0x4          /* PCI configuration space control and status */
  /* PCI target image (x) control register */
#define LSI_CTL(x) 0x100 + ((x) * 0x14) + (((x) >= 4) ? 0x50 : 0)
#define LSI_BS(x)  LSI_CTL(x) + 0x4     /* PCI target image (x) base address
                                           register */
#define LSI_BD(x)  LSI_CTL(x) + 0x8     /* PCI target image (x) bound address
                                           register */
#define LSI_TO(x)  LSI_CTL(x) + 0xC     /* PCI target image (x) translation
                                           offset register */
#define SLSI       0x188        /* Special PCI target image register */
#define DCTL       0x200        /* DMA transfer control register */
#define DTBC       0x204        /* DMA transfer byte count register */
#define DLA        0x208        /* DMA PCI bus address register */
#define DVA        0x210        /* DMA VME address register */
#define DCPP       0x218        /* DMA command packet pointer */
#define DGCS       0x220        /* DMA general control and status register */
#define D_LLUE     0x224        /* DMA linked list update enable register */
#define LINT_EN    0x300        /* PCI interrupt enable register */
#define LINT_STAT  0x304        /* PCI interrupt status register */
#define LINT_MAP0  0x308        /* PCI interrupt map 0 register */
#define LINT_MAP1  0x30c        /* PCI interrupt map 1 register */
#define LINT_MAP2  0x340        /* PCI interrupt map 2 register */
#define VINT_EN    0x310        /* VME interrupt enable register */
#define VINT_STAT  0x314        /* VME interrupt status register */
#define VINT_MAP0  0x318        /* VME interrupt map 0 register */
#define VINT_MAP1  0x31c        /* VME interrupt map 1 register */
#define VINT_MAP2  0x344        /* VME interrupt map 1 register */
#define STATID_OUT 0x320        /* Interrupt status/id out register */
#define STATID_IN(x)  (0x324 + (((x) - 1) * 4 ))        /* VIRQ(x) status/id in
                                                           register */
#define MBOX(x)    0x348 + ((x) * 4)    /* Mailbox (x) register */
#define SEMA(x)    0x358 + ((x) * 4)    /* Semaphore (x) register */
#define MAST_CTL   0x400        /* Master control register */
#define MISC_CTL   0x404        /* Miscellaneous control register */
  /* Slave image (x) control register */
#define VSI_CTL(x) 0xF00 + ((x) * 0x14) + ((x >= 4) ? 0x40 : 0)
#define VSI_BS(x)  VSI_CTL(x) + 0x4     /* Slave image (x) base address
                                           register */
#define VSI_BD(x)  VSI_CTL(x) + 0x8     /* Slave image (x) bound address
                                           register */
#define VSI_TO(x)  VSI_CTL(x) + 0xC     /* Slave image (x) translation offset
                                           register */
#define LM_CTL     0xF64        /* Location monitor control register */
#define LM_BS      0xF68        /* Location monitor base address register */
#define VRAI_CTL   0xF70        /* VME register access image control register */
#define VRAI_BS    0xF74        /* VME register access image base address
                                   register */
#define VAMERR     0xF88        /* VMEbus AM code error log */
#define VAERR      0xF8C        /* VMEbus address error log */

  /* 
   *  PCI Configuration space control and status register 
   */
#define PCI_MS_EN                   0x00000004
#define PCI_MS_DIS                  ~MS_EN      /* and'd */

  /*
   * Common image values
   */
#define IMAGE_EN                    0x80000000
#define IMAGE_DIS                   ~IMAGE_EN   /* and'd */
#define PWEN                        0x40000000

  /* 
   *  PCI target image registers
   */
#define TRGT_VDW_64                 0x00C00000
#define TRGT_VDW_32                 0x00800000
#define TRGT_VDW_16                 0x00400000
#define TRGT_VDW_8                  0x00000000
#define TRGT_VAS_USER2              0x00070000
#define TRGT_VAS_USER1              0x00060000
#define TRGT_VAS_A32                0x00020000
#define TRGT_VAS_A24                0x00010000
#define TRGT_VAS_A16                0x00000000
#define TRGT_VAS_CRCSR              0x00050000
#define TRGT_PGM_PROGRAM            0x00004000
#define TRGT_PGM_DATA               0x00000000
#define TRGT_SUPER_P                0x00001000
#define TRGT_SUPER_NP               0x00000000
#define TRGT_VCT_BLT_EN             0x00000100
#define TRGT_VCT_NO_BLT             0x00000000
#define TRGT_LAS_IO                 0x00000001
#define TRGT_LAS_MEM                0x00000000

  /* 
   *  Special PCI target image register defines (SLSI) 
   */
#define SLSI_TRGT_PWEN              0x40000000
#define SLSI0_TRGT_VDW_16           0x00000000
#define SLSI0_TRGT_VDW_32           0x00100000
#define SLSI1_TRGT_VDW_16           0x00000000
#define SLSI1_TRGT_VDW_32           0x00200000
#define SLSI2_TRGT_VDW_16           0x00000000
#define SLSI2_TRGT_VDW_32           0x00400000
#define SLSI3_TRGT_VDW_16           0x00000000
#define SLSI3_TRGT_VDW_32           0x00800000
#define SLSI0_TRGT_PGM_PRGM         0x00001000
#define SLSI0_TRGT_PGM_DATA         0x00000000
#define SLSI1_TRGT_PGM_PRGM         0x00002000
#define SLSI1_TRGT_PGM_DATA         0x00000000
#define SLSI2_TRGT_PGM_PRGM         0x00004000
#define SLSI2_TRGT_PGM_DATA         0x00000000
#define SLSI3_TRGT_PGM_PRGM         0x00008000
#define SLSI3_TRGT_PGM_DATA         0x00000000
#define SLSI0_TRGT_SUPER_P          0x00000100
#define SLSI0_TRGT_SUPER_NP         0x00000000
#define SLSI1_TRGT_SUPER_P          0x00000200
#define SLSI1_TRGT_SUPER_NP         0x00000000
#define SLSI2_TRGT_SUPER_P          0x00000400
#define SLSI2_TRGT_SUPER_NP         0x00000000
#define SLSI3_TRGT_SUPER_P          0x00000800
#define SLSI3_TRGT_SUPER_NP         0x00000000
#define SLSI_TRGT_LAS_IO            0x00000001
#define SLSI_TRGT_LAS_MEM           0x00000000

  /* 
   *  Interrupt register defines (LINT_EN/LINT_STAT/VINT_EN/VINT_STAT) 
   */
#define LINT_DIS_ALL                0xFF000800  /* and'd */
#define LINT_CLR_ALL                ~LINT_DIS_ALL       /* or'd */
#define VINT_DIS_ALL                0x01F0E800  /* and'd */
#define VINT_CLR_ALL                ~VINT_DIS_ALL       /* or'd */

  /* 
   *  Master control register defines (MAST_CTL) 
   */
#define MAXRTRY(x)                  (((x) & 0x3C0 ) << 22)
#define PWON_CNT_128                0x00000000
#define PWON_CNT_256                0x01000000
#define PWON_CNT_512                0x02000000
#define PWON_CNT_1024               0x03000000
#define PWON_CNT_2048               0x04000000
#define PWON_CNT_4096               0x05000000
#define PWON_EARLY_RELEASE          0x0f000000
#define VRL_0                       0x00000000
#define VRL_1                       0x00400000
#define VRL_2                       0x00800000
#define VRL_3                       0x00C00000
#define VRM_DEMAND                  0x00000000
#define VRM_FAIR                    0x00200000
#define	VREL_ON_REQ                 0x00100000
#define VREL_WHEN_DONE              0x00000000
#define VOWN_ACQ_AND_HOLD_BUS       0x00080000
#define VOWN_RELEASE_BUS            0x00000000
#define VOWN_ACK_NOT_OWNED          0x00000000
#define VOWN_ACK_OWNED_AND_HELD     0x00040000
#define PABS_32                     0x00000000
#define PABS_64                     0x00001000
#define BUS_NO(x)                   ((x) & 0xFF)

  /* 
   *  Miscellaneous control register defines (MISC_CTL) 
   */
#define VBTO_DISABLE                0x00000000
#define VBTO_16US                   0x10000000
#define VBTO_32US                   0x20000000
#define VBTO_64US                   0x30000000
#define VBTO_128US                  0x40000000
#define VBTO_256US                  0x50000000
#define VBTO_512US                  0x60000000
#define VBTO_1024US                 0x70000000
#define VARB_RNDRBN                 0x00000000
#define VARB_PRIORITY               0x04000000
#define VARB_TOUT_DISABL            0x00000000
#define VARB_TOUT_16US              0x01000000
#define	VARB_TOUT_256US             0x02000000
#define SW_LRST                     0x00800000
#define SW_SYSRST                   0x00400000
#define BI_MODE_EN                  0x00100000
#define BI_MODE_DIS                 0xFFEFFFFF
#define ENGBI_MODE                  0x00080000
#define RESCIND_DTACK_EN            0x00040000
#define SYSCON                      0x00020000
#define V64AUTO_ID                  0x0000FF00

  /* 
   *  VME slave image registers
   */
#define SLAVE_PWEN                  0x40000000
#define SLAVE_PREN                  0x20000000
#define SLAVE_PGM_PROGRAM           0x00800000
#define SLAVE_PGM_DATA              0x00400000
#define SLAVE_SUPER_NP              0x00200000
#define SLAVE_SUPER_P               0x00100000
#define SLAVE_VAS_A32               0x00020000
#define SLAVE_VAS_A24               0x00010000
#define SLAVE_VAS_A16               0x00000000
#define SLAVE_VAS_USER2             0x00070000
#define SLAVE_VAS_USER1             0x00060000
#define SLAVE_LD64EN                0x00000080
#define SLAVE_LLRMW                 0x00000040
#define SLAVE_LAS_CONFIG            0x00000002
#define SLAVE_LAS_IO                0x00000001
#define SLAVE_LAS_MEM               0x00000000

  /*
   * VMEbus AM Code Error Log (VAMERR)
   */
#define V_STAT                      0x00800000

  /* 
   *  DMA Transfer Control Register (DCTL)
   */
#define DMA_RD                      0x00000000
#define DMA_WR                      0x80000000
#define DMA_BLT                     0x00000100
#define DMA_LD64EN                  0x00000080

  /* 
   * DMA General Control/Status Register (DGCS)
   */
#define DMA_GO                      0x80000000
#define DMA_STOP_REQ                0x40000000
#define DMA_HALT_REQ                0x20000000
#define DMA_CHAIN                   0x08000000
#define DMA_VON_UNTIL_DONE          0x00000000
#define DMA_VON_256BYTES            0x00100000
#define DMA_VON_512BYTES            0x00200000
#define DMA_VON_1024BYTES           0x00300000
#define DMA_VON_2048BYTES           0x00400000
#define DMA_VON_4096BYTES           0x00500000
#define DMA_VON_8192BYTES           0x00600000
#define DMA_VON_16384BYTES          0x00700000
#define DMA_VOFF_0US                0x00000000
#define DMA_VOFF_16US               0x00010000
#define DMA_VOFF_32US               0x00020000
#define DMA_VOFF_64US               0x00030000
#define DMA_VOFF_128US              0x00040000
#define DMA_VOFF_256US              0x00050000
#define DMA_VOFF_512US              0x00060000
#define DMA_VOFF_1024US             0x00070000
#define DMA_VOFF_2US                0x00080000
#define DMA_VOFF_4US                0x00090000
#define DMA_VOFF_8US                0x000a0000
#define DMA_ACTIVE                  0x00008000
#define DMA_STOPPED                 0x00004000
#define DMA_HALTED                  0x00002000
#define DMA_DONE                    0x00000800
#define DMA_LERR                    0x00000400
#define DMA_VERR                    0x00000200
#define DMA_PERR                    0x00000100
#define DMA_INT_STOP                0x00000040
#define DMA_INT_HALT                0x00000020
#define DMA_INT_DONE                0x00000008
#define DMA_INT_LERR                0x00000004
#define DMA_INT_VERR                0x00000002
#define DMA_INT_PERR                0x00000001

  /* 
   * Fortunately, none of the needed bits overlap, so dma flags can just be the
   * & of the valid flags for both of these registers
   */
#define DMA_VALID_DCTL_FLAGS        0x80000180
#define DMA_VALID_DGCS_FLAGS        0x687f0000
#define DMA_VALID_USER_FLAGS        0x707f0180
#define DMA_BLOCKING                0x10000000  /* Or this to flags to make a
                                                   blocking DMA transfer call */
#define DMA_DEFAULT_FLAGS           DMA_BLOCKING

  /* 
   * VMIC PCI definitions
   */
#define VMIC_VENDOR_ID              0x114a
#define VMIC_PLX_DEVICE_ID          0x0001
#define VMIC_FPGA_DEVICE_ID1        0x0004
#define VMIC_FPGA_DEVICE_ID2        0x0005
#define VMIC_PLX_BASE_ADDR_REG      0x18
#define PLX_BASE_ADDR_REG           0x10
#define VMIC_FPGA_BASE_ADDR_REG     0x10

  /* 
   * Register offsets for ISA
   */
#define VMIC_ISA_BASE               0xd8000
#define ISA_COMM_OFFSET             0x0e
#define ISA_VBAR_OFFSET             0x10
#define ISA_VBAMR_OFFSET            0x14
#define ISA_ID_OFFSET               0x16

  /* 
   * Register offsets for PLX device
   */
#define PLX_COMM_OFFSET             0x00
#define PLX_B_INT_STATUS_OFFSET     0x04
#define PLX_B_INT_MASK_OFFSET       0x08
#define PLX_B_ADDRESS_OFFSET        0x0C
#define PLX_CSR_OFFSET              0x68

  /*
   * Register offset for FPGA device
   */
#define FPGA_COMM_OFFSET            0x00
#define FPGA_VBAMR_OFFSET           0x04
#define FPGA_VBAR_OFFSET            0x08

  /* 
   * Bit values common to PLX, FPGA, and ISA registers
   */
#define MEC                         0x00000001
#define SEC                         0x00000002
#define ABLE                        0x00000004
#define BTO_EN                      0x00000008
#define BTO_16US                    0x00000000
#define BTO_64US                    0x00000010
#define BTO_256US                   0x00000020
#define BTO_1MS                     0x00000030
#define VME_EN                      0x00000800
#define VME_DIS                     0x0000F7FF

  /* 
   * ISA register specific bit values
   */
#define BERRI_ISA                   0x00000040
#define BERRST_ISA                  0x00000080
#define WTDSYS_ISA                  0x00000100

  /* 
   * PLX specific bit values
   */
#define BERRIM_INTA_PLX             0x00000000
#define BERRIM_NMI_PLX              0x00000040
#define MB_M0_EN_PLX                0x00000080
#define BERR_S_PLX                  0x00000001
#define BERR_M_PLX                  0x0001
#define PLX_INTR_ENABLE             0x00000800

  /* 
   * FPGA register specific bit values
   */
#define BERRI_FPGA                  0x00000040
#define BERRST_FPGA                 0x00000080
#define WTDSYS_FPGA                 0x00000100


/* Redefine old enumerated types that had names that confict with the new
   enumerated types
 */
#define V_A32UMB                    VME_A32UMB
#define V_A32UD                     VME_A32UD
#define V_A32UP                     VME_A32UP
#define V_A32UB                     VME_A32UB
#define V_A32SMB                    VME_A32SMB
#define V_A32SD                     VME_A32SD
#define V_A32SP                     VME_A32SP
#define V_A32SB                     VME_A32SB
#define V_A16UD                     VME_A16U
#define V_A16SD                     VME_A16S
#define V_A24UMB                    VME_A24UMB
#define V_A24UD                     VME_A24UD
#define V_A24UP                     VME_A24UP
#define V_A24UB                     VME_A24UB
#define V_A24SMB                    VME_A24SMB
#define V_A24SD                     VME_A24SD
#define V_A24SP                     VME_A24SP
#define V_A24SB                     VME_A24SB

#define V_D8                        VME_D8
#define V_D16                       VME_D16
#define V_D32                       VME_D32
#define V_D64                       VME_D64

#define V_DEMAND                    VME_DEMAND
#define V_FAIR                      VME_FAIR

#define V_RWD                       VME_RELEASE_ON_REQUEST
#define V_ROR                       VME_RELEASE_WHEN_DONE

  typedef enum
  {
    V_POSTED_WRITE_CNT_0 = 0,   /* Early release of BBSY */
    V_POSTED_WRITE_CNT_128 = 128,       /* 128 byte posted write transfer count */
    V_POSTED_WRITE_CNT_512 = 512,       /* 512 byte posted write transfer count */
    V_POSTED_WRITE_CNT_1024 = 1024,     /* 1024 byte posted write transfer count */
    V_POSTED_WRITE_CNT_2048 = 2048,     /* 2048 byte posted write transfer count */
    V_POSTED_WRITE_CNT_4096 = 4096      /* 4096 byte posted write transfer count */
  }
  vme_posted_write_count_t;

  typedef enum
  {
    V_BR0 = 0,                  /* VMEbus request level 0 */
    V_BR1 = 1,                  /* VMEbus request level 1 */
    V_BR2 = 2,                  /* VMEbus request level 2 */
    V_BR3 = 3                   /* VMEbus request level 3 */
  }
  vme_bus_request_level_t;

  typedef enum
  {
    V_RELEASE_BUS,              /* VMEbus release */
    V_ACQ_AND_HOLD_BUS          /* VMEbus aquire and hold */
  }
  vme_vown_t;

  typedef enum
  {
    V_BTO_DISABLE = 0,          /* VMEbus timeout disabled */
    V_BTO_16uS = 16,            /* VMEbus 16us timeout */
    V_BTO_32uS = 32,            /* VMEbus 32us timeout */
    V_BTO_64uS = 64,            /* VMEbus 64us timeout */
    V_BTO_128uS = 128,          /* VMEbus 128us timeout */
    V_BTO_256uS = 256,          /* VMEbus 256us timeout */
    V_BTO_512uS = 512,          /* VMEbus 512us timeout */
    V_BTO_1024uS = 1024         /* VMEbus 1024us timeout */
  }
  vme_bus_timeout_t;

  typedef enum
  {
    V_ARB_RNDRBN,               /* VMEbus round-robin mode arbitration */
    V_ARB_PRIORITY              /* VMEbus priority mode arbitration */
  }
  vme_bus_arbitration_mode_t;

  typedef enum
  {
    V_ARB_TO_DISABLE = 0,       /* VMEbus arbitration timeout disabled */
    V_ARB_TO_16uS = 16,         /* VMEbus 16us arbitration timeout */
    V_ARB_TO_256uS = 256        /* VMEbus 256us arbitration timeout */
  }
  vme_bus_arbitration_timeout_t;

  typedef enum
  {
    V_ENDIAN_CONVERSION_OFF = 0,        /* VMEbus hardware endian conversion off */
    V_ENDIAN_CONVERSION_ON = 1  /* VMEbus hardware endian conversion on */
  }
  vme_endian_conversion_t;

  typedef enum
  {
    VOWN_INT_LVL = 0,           /* VOWN interrupt */
    VME_INT_LVL1 = 1,           /* VMEbus interrupt level 1 */
    VME_INT_LVL2 = 2,           /* VMEbus interrupt level 2 */
    VME_INT_LVL3 = 3,           /* VMEbus interrupt level 3 */
    VME_INT_LVL4 = 4,           /* VMEbus interrupt level 4 */
    VME_INT_LVL5 = 5,           /* VMEbus interrupt level 5 */
    VME_INT_LVL6 = 6,           /* VMEbus interrupt level 6 */
    VME_INT_LVL7 = 7,           /* VMEbus interrupt level 7 */
    DMA_INT_LVL = 8,            /* DMA interrupt */
    PCI_BERR_LVL = 9,           /* PCI error interrupt */
    VME_BERR_LVL = 10,          /* VMEbus error interrupt */
    IACK_INT_LVL = 12,          /* Software interrupt acknowledge interrupt */
    SW_INT_LVL = 13,            /* Software interrupt */
    SYSFAIL_INT_LVL = 14,       /* System fail interrupt */
    ACFAIL_INT_LVL = 15,        /* Power fail interrupt */
    MB_INT_LVL1 = 16,           /* Mailbox interrupt level 1 */
    MB_INT_LVL2 = 17,           /* Mailbox interrupt level 2 */
    MB_INT_LVL3 = 18,           /* Mailbox interrupt level 3 */
    MB_INT_LVL4 = 19,           /* Mailbox interrupt level 4 */
    LM_INT_LVL1 = 20,           /* Location monitor interrupt level 1 */
    LM_INT_LVL2 = 21,           /* Location monitor interrupt level 2 */
    LM_INT_LVL3 = 22,           /* Location monitor interrupt level 3 */
    LM_INT_LVL4 = 23            /* Location monitor interrupt level 4 */
  }
  vme_intr_lvl_t;
#define MAX_INT_LVL      LM_INT_LVL4

  /* 
   * Interrupt vectors 
   */
#define MAX_AUX_VECTORS  24     /* Max Auxiliary vectors. */
#define MAX_INT_VECTORS  256    /* Max number of IRQ vectors. */
#define MAX_INT_LEVELS   7      /* Max Interrupt levels [1-7] */

  /* 
   * Total number of interrupt handlers including all VMEbus interrupt
   * levels and vectors plus the other interrupt levels 
   */
#define MAX_HANDLERS  ( ( MAX_INT_VECTORS * MAX_INT_LEVELS ) + MAX_AUX_VECTORS)

  /* 
   * These vectors sit on top of the VMEbus interrupt vectors in a single
   * dimensional array 
   */
#define VOWN_VECTOR          ( MAX_INT_LEVELS * MAX_INT_VECTORS )
#define DMA_VECTOR           ( VOWN_VECTOR + DMA_INT_LVL )
#define PCI_BERR_VECTOR      ( VOWN_VECTOR + PCI_BERR_LVL )
#define VME_BERR_VECTOR      ( VOWN_VECTOR + VME_BERR_LVL )
#define IACK_VECTOR          ( VOWN_VECTOR + IACK_INT_LVL )
#define SW_VECTOR            ( VOWN_VECTOR + SW_INT_LVL )
#define SYSFAIL_VECTOR       ( VOWN_VECTOR + SYSFAIL_INT_LVL )
#define ACFAIL_VECTOR        ( VOWN_VECTOR + ACFAIL_INT_LVL )
#define MBOX_VECTOR( num )   ( VOWN_VECTOR + MB_INT_LVL1 + ( num ) )
#define LM_VECTOR( num )     ( VOWN_VECTOR + LM_INT_LVL1 + ( num ) )

  /*
   * Computation to get interrupt table index
   */
#define INTR_TABLE_INDEX( _lvl, _vec ) \
( ( ( VME_INT_LVL1 <= ( _lvl ) ) && ( VME_INT_LVL7 >= ( _lvl ) ) ) ? \
( ( ( _lvl ) - 1 ) * MAX_INT_VECTORS ) + ( _vec ) : VOWN_VECTOR + ( _lvl ) )

#define INDEX_TO_LVL_VEC( _idx, _lvl, _vec ) \
if ( VOWN_VECTOR <= ( _idx ) ) \
{ ( _lvl ) = ( _idx ) - VOWN_VECTOR; ( _vec ) = 0; } \
else \
{ ( _lvl ) = ( ( _idx ) / MAX_INT_VECTORS ) + 1; \
( _vec ) = ( _idx ) % MAX_INT_VECTORS; }


#ifdef __cplusplus
}

#endif                          /* __cplusplus */

#endif                          /* __LEGACY_UNIVERSE_H */
