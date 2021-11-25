
/*
===============================================================================
                            COPYRIGHT NOTICE

    Copyright (C) 2001-2003 GE Fanuc
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


#ifndef __UNIVERSE_H
#define __UNIVERSE_H

#ifdef __cplusplus
extern "C"
{

#endif                          /* __cplusplus */


  /* 
   *  Universe device registers 
   */
#define UNIV_PCI_ID     0x000   /* PCI configuration space ID register */
#define UNIV_PCI_CSR    0x004   /* PCI configuration space control and status */
#define UNIV_PCI_CLASS  0x008   /* PCI configuration class register */
#define UNIV_PCI_MISC0  0x00C   /* PCI configuration miscellaneous 0 regster */
#define UNIV_PCI_BS0    0x010   /* PCI configuration base address 0 register */
#define UNIV_PCI_BS1    0x014   /* PCI configuration base address 1 register */
#define UNIV_PCI_MISC1  0x03C   /* PCI configuration miscellaneous 1 regster */
  /* PCI target image (x) control register */
#define UNIV_LSI_CTL(x) 0x100 + ((x) * 0x14) + (((x) >= 4) ? 0x50 : 0)
#define UNIV_LSI_BS(x)  UNIV_LSI_CTL(x) + 0x4   /* PCI target image (x) base
                                                   address register */
#define UNIV_LSI_BD(x)  UNIV_LSI_CTL(x) + 0x8   /* PCI target image (x) bound
                                                   address register */
#define UNIV_LSI_TO(x)  UNIV_LSI_CTL(x) + 0xC   /* PCI target image (x)
                                                   translation offset
                                                   register */
#define UNIV_SCYC_CTL   0x170   /* Special cycle control register */
#define UNIV_SCYC_ADDR  0x174   /* Special cycle PCI bus address register */
#define UNIV_SCYC_EN    0x178   /* Special cycle swap/compare enable register */
#define UNIV_SCYC_CMP   0x17C   /* Special cycle compare data register */
#define UNIV_SCYC_SWP   0x180   /* Special cycle swap data register */
#define UNIV_LMISC      0x184   /* PCI micellaneous register */
#define UNIV_SLSI       0x188   /* Special PCI target image register */
#define UNIV_DCTL       0x200   /* DMA transfer control register */
#define UNIV_DTBC       0x204   /* DMA transfer byte count register */
#define UNIV_DLA        0x208   /* DMA PCI bus address register */
#define UNIV_DVA        0x210   /* DMA VME address register */
#define UNIV_DCPP       0x218   /* DMA command packet pointer */
#define UNIV_DGCS       0x220   /* DMA general control and status register */
#define UNIV_D_LLUE     0x224   /* DMA linked list update enable register */
#define UNIV_LINT_EN    0x300   /* PCI interrupt enable register */
#define UNIV_LINT_STAT  0x304   /* PCI interrupt status register */
#define UNIV_LINT_MAP0  0x308   /* PCI interrupt map 0 register */
#define UNIV_LINT_MAP1  0x30c   /* PCI interrupt map 1 register */
#define UNIV_VINT_EN    0x310   /* VME interrupt enable register */
#define UNIV_VINT_STAT  0x314   /* VME interrupt status register */
#define UNIV_VINT_MAP0  0x318   /* VME interrupt map 0 register */
#define UNIV_VINT_MAP1  0x31c   /* VME interrupt map 1 register */
#define UNIV_STATID     0x320   /* Interrupt status/id out register */
#define UNIV_V_STATID(x)  (0x324 + (((x) - 1) * 4 ))    /* VIRQ(x) status/id in
                                                           register */
#define UNIV_LINT_MAP2  0x340   /* PCI interrupt map 2 register */
#define UNIV_VINT_MAP2  0x344   /* VME interrupt map 1 register */
#define UNIV_MBOX(x)    0x348 + ((x) * 4)       /* Mailbox (x) register */
#define UNIV_SEMA(x)    0x358 + (x)     /* Semaphore(x) register, byte access */
#define UNIV_MAST_CTL   0x400   /* Master control register */
#define UNIV_MISC_CTL   0x404   /* Miscellaneous control register */
#define UNIV_MISC_STAT  0x408   /* Miscellaneous status register */
#define UNIV_USER_AM    0x40C   /* User address modifier codes register */
#define UNIV_U2SPEC     0x4FC   /* Universe II Specific Register */
  /* Slave image (x) control register */
#define UNIV_VSI_CTL(x) 0xF00 + ((x) * 0x14) + (((x) >= 4) ? 0x40 : 0)
#define UNIV_VSI_BS(x)  UNIV_VSI_CTL(x) + 0x4   /* Slave image (x) base address
                                                   register */
#define UNIV_VSI_BD(x)  UNIV_VSI_CTL(x) + 0x8   /* Slave image (x) bound address
                                                   register */
#define UNIV_VSI_TO(x)  UNIV_VSI_CTL(x) + 0xC   /* Slave image (x) translation
                                                   offset register */
#define UNIV_LM_CTL     0xF64   /* Location monitor control register */
#define UNIV_LM_BS      0xF68   /* Location monitor base address register */
#define UNIV_VRAI_CTL   0xF70   /* VME register access image control register */
#define UNIV_VRAI_BS    0xF74   /* VME register access image base address
                                   register */
#define UNIV_VCSR_CTL   0xF80   /* VMEbus CSR control register */
#define UNIV_VCSR_TO    0xF84   /* VMEbus CSR translation offset register */
#define UNIV_V_AMERR    0xF88   /* VMEbus AM code error log */
#define UNIV_VAERR      0xF8C   /* VMEbus address error log */
#define UNIV_VCSR_CLR   0xFF4   /* VMEbus CSR bit clear register */
#define UNIV_VCSR_SET   0xFF8   /* VMEbus CSR bit set register */
#define UNIV_VCSR_BS    0xFFC   /* VMEbus CSR base address register */

  /* 
   *  PCI Configuration space control and status register 
   */
#define UNIV_PCI_CSR__D_PE                  0x80000000
#define UNIV_PCI_CSR__S_SERR                0x40000000
#define UNIV_PCI_CSR__R_MA                  0x20000000
#define UNIV_PCI_CSR__R_TA                  0x10000000
#define UNIV_PCI_CSR__S_TA                  0x08000000
#define UNIV_PCI_CSR__DEVSEL                0x06000000
#define UNIV_PCI_CSR__DP_D                  0x01000000
#define UNIV_PCI_CSR__TFBBC                 0x00800000
#define UNIV_PCI_CSR__MFBBC                 0x00000200
#define UNIV_PCI_CSR__SERR_EN               0x00000100
#define UNIV_PCI_CSR__WAIT                  0x00000080
#define UNIV_PCI_CSR__PERESP                0x00000040
#define UNIV_PCI_CSR__VGAPS                 0x00000020
#define UNIV_PCI_CSR__MWI_EN                0x00000010
#define UNIV_PCI_CSR__SC                    0x00000008
#define UNIV_PCI_CSR__BM                    0x00000004
#define UNIV_PCI_CSR__MS                    0x00000002
#define UNIV_PCI_CSR__IOS                   0x00000001


  /*
   * PCI configuration class registers
   */
#define UNIV_PCI_CLASS__BASE                0xFF000000
#define UNIV_PCI_CLASS__SUB                 0x00FF0000
#define UNIV_PCI_CLASS__PROG                0x0000FF00
#define UNIV_PCI_CLASS__RID                 0x000000FF


  /*
   * PCI configuration miscellaneous 0 register
   */
#define UNIV_PCI_MISC0__BISTC               0x80000000
#define UNIV_PCI_MISC0__SBIST               0x40000000
#define UNIV_PCI_MISC0__CCODE               0x0F000000
#define UNIV_PCI_MISC0__MFUNCT              0x00800000
#define UNIV_PCI_MISC0__LAYOUT              0x003F0000
#define UNIV_PCI_MISC0__LTIMER              0x0000F800


  /*
   * PCI configuration miscellaneous 1 register
   */
#define UNIV_PCI_MISC1__MAX_LAT             0xFF000000
#define UNIV_PCI_MISC1__MIN_GNT             0x00FF0000
#define UNIV_PCI_MISC1__INT_PIN             0x0000FF00
#define UNIV_PCI_MISC1__INT_LINE            0x000000FF


  /* 
   *  PCI target image control registers
   */
#define UNIV_LSI_CTL__EN                    0x80000000
#define UNIV_LSI_CTL__PWEN                  0x40000000
#define UNIV_LSI_CTL__VDW                   0x00C00000
#define UNIV_LSI_CTL__VDW__D64              0x00C00000
#define UNIV_LSI_CTL__VDW__D32              0x00800000
#define UNIV_LSI_CTL__VDW__D16              0x00400000
#define UNIV_LSI_CTL__VDW__D8               0x00000000
#define UNIV_LSI_CTL__VAS                   0x00070000
#define UNIV_LSI_CTL__VAS__USER2            0x00070000
#define UNIV_LSI_CTL__VAS__USER1            0x00060000
#define UNIV_LSI_CTL__VAS__CRCSR            0x00050000
#define UNIV_LSI_CTL__VAS__A32              0x00020000
#define UNIV_LSI_CTL__VAS__A24              0x00010000
#define UNIV_LSI_CTL__VAS__A16              0x00000000
#define UNIV_LSI_CTL__PGM                   0x00004000
#define UNIV_LSI_CTL__SUPER                 0x00001000
#define UNIV_LSI_CTL__VCT                   0x00000100
#define UNIV_LSI_CTL__LAS                   0x00000001

#define UNIV_LSI__4KB_MASK                  0x00000FFF /* LSI 0,4 */
#define UNIV_LSI__64KB_MASK                 0x0000FFFF /* LSI 1,2,3,5,6,7 */

  /*
   * Special cycle control register
   */
#define UNIV_SCYC_CTL__LAS                  0x00000004
#define UNIV_SCYC_CTL__SCYC                 0x00000003
#define UNIV_SCYC_CTL__SCYC__RMW            0x00000001
#define UNIV_SCYC_CTL__SCYC__ADOH           0x00000002
#define UNIV_SCYC_CTL__SCYC__DISABLE        0x00000000

#define SCYC_EN__MASK                       0xFFFFFFFF
#define SCYC_CMP__TO_SET                    0x00000000 /* sysBusTas */
#define SCYC_SWP__TO_SET                    0xFFFFFFFF /* sysBusTas */

#define SCYC_CMP__TO_CLR                    0xFFFFFFFF /* sysBusTasClear */
#define SCYC_SWP__TO_CLR                    0x00000000 /* sysBusTasClear */

  /*
   * PCI miscellaneous register
   */
#define UNIV_LMISC__CRT                     0xF0000000
#define UNIV_LMISC__CWT                     0x07000000
#define UNIV_LMISC__CWT__512                0x06000000
#define UNIV_LMISC__CWT__256                0x05000000
#define UNIV_LMISC__CWT__128                0x04000000
#define UNIV_LMISC__CWT__32                 0x02000000
#define UNIV_LMISC__CWT__64                 0x03000000
#define UNIV_LMISC__CWT__16                 0x01000000


  /* 
   *  Special PCI target image register
   */
#define UNIV_SLSI__EN                       0x80000000
#define UNIV_SLSI__PWEN                     0x40000000
#define UNIV_SLSI__VDW(x)                   (0x00100000 << (x))
#define UNIV_SLSI__VDW__D32(x)              (0x00100000 << (x))
#define UNIV_SLSI__VDW__D16(x)              0x00000000
#define UNIV_SLSI__PGM(x)                   (0x00001000 << (x))
#define UNIV_SLSI__SUPER(x)                 (0x00000100 << (x))
#define UNIV_SLSI__BS                       0x000000FC
#define UNIV_SLSI__LAS                      0x00000001


  /*
   * PCI command error log register
   */
#define UNIV_L_CMDERR__CMDERR               0xF0000000
#define UNIV_L_CMDERR__M_ERR                0x08000000
#define UNIV_L_CMDERR__L_STAT               0x00800000


  /* 
   *  DMA Transfer Control Register
   */
#define UNIV_DCTL__L2V                      0x80000000
#define UNIV_DCTL__VDW                      0x00C00000
#define UNIV_DCTL__VDW__D64                 0x00C00000
#define UNIV_DCTL__VDW__D32                 0x00800000
#define UNIV_DCTL__VDW__D16                 0x00400000
#define UNIV_DCTL__VDW__D8                  0x00000000
#define UNIV_DCTL__VAS                      0x00070000
#define UNIV_DCTL__VAS__USER2               0x00070000
#define UNIV_DCTL__VAS__USER1               0x00060000
#define UNIV_DCTL__VAS__A32                 0x00020000
#define UNIV_DCTL__VAS__A24                 0x00010000
#define UNIV_DCTL__VAS__A16                 0x00000000
#define UNIV_DCTL__PGM                      0x00004000
#define UNIV_DCTL__SUPER                    0x00001000
#define UNIV_DCTL__VCT                      0x00000100
#define UNIV_DCTL__LD64EN                   0x00000080


  /* 
   * DMA General Control/Status Register
   */
#define UNIV_DGCS__GO                       0x80000000
#define UNIV_DGCS__STOP_REQ                 0x40000000
#define UNIV_DGCS__HALT_REQ                 0x20000000
#define UNIV_DGCS__CHAIN                    0x08000000
#define UNIV_DGCS__VON                      0x00700000
#define UNIV_DGCS__VON__UNTIL_DONE          0x00000000
#define UNIV_DGCS__VON__256                 0x00100000
#define UNIV_DGCS__VON__512                 0x00200000
#define UNIV_DGCS__VON__1024                0x00300000
#define UNIV_DGCS__VON__2048                0x00400000
#define UNIV_DGCS__VON__4096                0x00500000
#define UNIV_DGCS__VON__8192                0x00600000
#define UNIV_DGCS__VON__16384               0x00700000
#define UNIV_DGCS__VOFF                     0x000f0000
#define UNIV_DGCS__VOFF__0                  0x00000000
#define UNIV_DGCS__VOFF__16                 0x00010000
#define UNIV_DGCS__VOFF__32                 0x00020000
#define UNIV_DGCS__VOFF__64                 0x00030000
#define UNIV_DGCS__VOFF__128                0x00040000
#define UNIV_DGCS__VOFF__256                0x00050000
#define UNIV_DGCS__VOFF__512                0x00060000
#define UNIV_DGCS__VOFF__1024               0x00070000
#define UNIV_DGCS__VOFF__2000               0x00080000
#define UNIV_DGCS__VOFF__4000               0x00090000
#define UNIV_DGCS__VOFF__8000               0x000a0000
#define UNIV_DGCS__ACT                      0x00008000
#define UNIV_DGCS__STOP                     0x00004000
#define UNIV_DGCS__HALT                     0x00002000
#define UNIV_DGCS__DONE                     0x00000800
#define UNIV_DGCS__LERR                     0x00000400
#define UNIV_DGCS__VERR                     0x00000200
#define UNIV_DGCS__P_ERR                    0x00000100
#define UNIV_DGCS__INT_STOP                 0x00000040
#define UNIV_DGCS__INT_HALT                 0x00000020
#define UNIV_DGCS__INT_DONE                 0x00000008
#define UNIV_DGCS__INT_LERR                 0x00000004
#define UNIV_DGCS__INT_VERR                 0x00000002
#define UNIV_DGCS__INT_P_ERR                0x00000001


  /*
   * DMA linked list update enabled register
   */
#define UNIV_D_LLUE__UPDATE                 0x80000000


  /*
   * PCI interrupt enable and status registers
   */
#define UNIV_LINT__LM3                      0x00800000
#define UNIV_LINT__LM2                      0x00400000
#define UNIV_LINT__LM1                      0x00200000
#define UNIV_LINT__LM0                      0x00100000
#define UNIV_LINT__LM(x)                    (0x00100000 << (x))
#define UNIV_LINT__MBOX3                    0x00080000
#define UNIV_LINT__MBOX2                    0x00040000
#define UNIV_LINT__MBOX1                    0x00020000
#define UNIV_LINT__MBOX0                    0x00010000
#define UNIV_LINT__MBOX(x)                  (0x00010000 << (x))
#define UNIV_LINT__ACFAIL                   0x00008000
#define UNIV_LINT__SYSFAIL                  0x00004000
#define UNIV_LINT__SW_INT                   0x00002000
#define UNIV_LINT__SW_IACK                  0x00001000
#define UNIV_LINT__VERR                     0x00000400
#define UNIV_LINT__LERR                     0x00000200
#define UNIV_LINT__DMA                      0x00000100
#define UNIV_LINT__VIRQ7                    0x00000080
#define UNIV_LINT__VIRQ6                    0x00000040
#define UNIV_LINT__VIRQ5                    0x00000020
#define UNIV_LINT__VIRQ4                    0x00000010
#define UNIV_LINT__VIRQ3                    0x00000008
#define UNIV_LINT__VIRQ2                    0x00000004
#define UNIV_LINT__VIRQ1                    0x00000002
#define UNIV_LINT__VIRQ(x)                  (1 << (x))
#define UNIV_LINT__VOWN                     0x00000001
#define UNIV_MAX_IRQ                        23

#define UNIV_LINT_STAT_INT_PENDING      (UNIV_LINT__VOWN | \
                                         UNIV_LINT__VIRQ(1) | \
                                         UNIV_LINT__VIRQ(2) | \
                                         UNIV_LINT__VIRQ(3) | \
                                         UNIV_LINT__VIRQ(4) | \
                                         UNIV_LINT__VIRQ(5) | \
                                         UNIV_LINT__VIRQ(6) | \
                                         UNIV_LINT__VIRQ(7) | \
                                         UNIV_LINT__DMA | \
                                         UNIV_LINT__LERR | \
                                         UNIV_LINT__VERR | \
                                         UNIV_LINT__SW_IACK | \
                                         UNIV_LINT__SW_INT | \
                                         UNIV_LINT__SYSFAIL | \
                                         UNIV_LINT__ACFAIL | \
                                         UNIV_LINT__MBOX(0) | \
                                         UNIV_LINT__MBOX(1) | \
                                         UNIV_LINT__MBOX(2) | \
                                         UNIV_LINT__MBOX(3) | \
                                         UNIV_LINT__LM(0) | \
                                         UNIV_LINT__LM(1) | \
                                         UNIV_LINT__LM(2) | \
                                         UNIV_LINT__LM(3))

  /* Sometimes it's helpful to have all of the interrupt level/vectors in a
     single array.  These macros are useful for that purpose. In this case the
     first MAX_INT_LEVELS * MAX_INT_VECTORS elements are the vectored VME
     IRQ's.  The other interrupt levels sit on top of the VME IRQ's and are
     indexed by UNIV_VECTOR_IRQS + level.
   */
#define UNIV_VIRQS        7      /* Number of vectored VME irq's */
#define UNIV_VECTORS      256    /* Number of VIRQ vectors. */
#define UNIV_VECTOR_IRQS (UNIV_VIRQS * UNIV_VECTORS)
#define UNIV_IRQS         UNIV_VECTOR_IRQS + UNIV_MAX_IRQ + 1 

#define UNIV_INTERRUPT_INDEX(_lvl, _vec) \
(((1 <= (_lvl)) && (UNIV_VIRQS >= (_lvl))) ? \
(((_lvl) - 1) * UNIV_VECTORS) + (_vec) : UNIV_VECTOR_IRQS + (_lvl))

#define UNIV_INTERRUPT_LEVEL(_idx) \
((UNIV_VECTOR_IRQS > (_idx)) ? ((_idx) / UNIV_VECTORS + 1) : \
(_idx) - UNIV_VECTOR_IRQS)

#define UNIV_INTERRUPT_VECTOR(_idx) \
((UNIV_VECTOR_IRQS > (_idx)) ? (_idx) % UNIV_VECTORS : 0)


  /*
   * PCI interrupt map registers
   */
#define UNIV_LINT_MAP0__VIRQ7               0x70000000
#define UNIV_LINT_MAP0__VIRQ6               0x07000000
#define UNIV_LINT_MAP0__VIRQ5               0x00700000
#define UNIV_LINT_MAP0__VIRQ4               0x00070000
#define UNIV_LINT_MAP0__VIRQ3               0x00007000
#define UNIV_LINT_MAP0__VIRQ2               0x00000700
#define UNIV_LINT_MAP0__VIRQ1               0x00000070
#define UNIV_LINT_MAP0__VIRQ(virq,lint)     (((lint) & 0x07) << (4 * (virq)))
#define UNIV_LINT_MAP0__VOWN                0x00000007
#define UNIV_LINT_MAP0__VOWN_LEVEL(x)       (((x) & 0x07) << 0)
#define UNIV_LINT_MAP0__ACFAIL              0x70000000
#define UNIV_LINT_MAP1__ACFAIL_LEVEL(x)     (((x) & 0x07) << 28)
#define UNIV_LINT_MAP1__SYSFAIL             0x07000000
#define UNIV_LINT_MAP1__SYSFAIL_LEVEL(x)    (((x) & 0x07) << 24)
#define UNIV_LINT_MAP1__SWINT               0x00700000
#define UNIV_LINT_MAP1__SWINT_LEVEL(x)      (((x) & 0x07) << 20)
#define UNIV_LINT_MAP1__SWIACK              0x00070000
#define UNIV_LINT_MAP1__SWIACK_LEVEL(x)     (((x) & 0x07) << 16)
#define UNIV_LINT_MAP1__VERR                0x00000700
#define UNIV_LINT_MAP1__VERR_LEVEL(x)       (((x) & 0x07) << 8)
#define UNIV_LINT_MAP1__LERR                0x00000070
#define UNIV_LINT_MAP1__LERR_LEVEL(x)       (((x) & 0x07) << 4)
#define UNIV_LINT_MAP1__DMA                 0x00000007
#define UNIV_LINT_MAP1__DMA_LEVEL(x)        (((x) & 0x07) << 0)
#define UNIV_LINT_MAP2__LM3                 0x70000000
#define UNIV_LINT_MAP2__LM2                 0x07000000
#define UNIV_LINT_MAP2__LM1                 0x00700000
#define UNIV_LINT_MAP2__LM0                 0x00070000
#define UNIV_LINT_MAP2__LM(lm,lint)         (((lint) & 0x7) << ((4 * (lm)) + 16))
#define UNIV_LINT_MAP2__LM3                 0x70000000
#define UNIV_LINT_MAP2__MBOX3               0x00007000
#define UNIV_LINT_MAP2__MBOX2               0x00000700
#define UNIV_LINT_MAP2__MBOX1               0x00000070
#define UNIV_LINT_MAP2__MBOX0               0x00000007
#define UNIV_LINT_MAP2__MBOX(mb,lint)       (((lint) & 0x7) << (4 * (mb)))


  /*
   * VMEbus interrupt enable and status registers
   */
#define UNIV_VINT__SW_INT7                  0x80000000
#define UNIV_VINT__SW_INT6                  0x40000000
#define UNIV_VINT__SW_INT5                  0x20000000
#define UNIV_VINT__SW_INT4                  0x10000000
#define UNIV_VINT__SW_INT3                  0x08000000
#define UNIV_VINT__SW_INT2                  0x04000000
#define UNIV_VINT__SW_INT1                  0x02000000
#define UNIV_VINT__MBOX3                    0x00080000
#define UNIV_VINT__MBOX2                    0x00040000
#define UNIV_VINT__MBOX1                    0x00020000
#define UNIV_VINT__MBOX0                    0x00010000
#define UNIV_VINT__SW_INT                   0x00001000
#define UNIV_VINT__VERR                     0x00000400
#define UNIV_VINT__LERR                     0x00000200
#define UNIV_VINT__DMA                      0x00000100
#define UNIV_VINT__LINT7                    0x00000080
#define UNIV_VINT__LINT6                    0x00000040
#define UNIV_VINT__LINT5                    0x00000020
#define UNIV_VINT__LINT4                    0x00000010
#define UNIV_VINT__LINT3                    0x00000008
#define UNIV_VINT__LINT2                    0x00000004
#define UNIV_VINT__LINT1                    0x00000002
#define UNIV_VINT__LINT0                    0x00000001


  /*
   * VMEbus interrupt map registers
   */
#define UNIV_VINT_MAP0__LINT7               0x70000000
#define UNIV_VINT_MAP0__LINT6               0x07000000
#define UNIV_VINT_MAP0__LINT5               0x00700000
#define UNIV_VINT_MAP0__LINT4               0x00070000
#define UNIV_VINT_MAP0__LINT3               0x00007000
#define UNIV_VINT_MAP0__LINT2               0x00000700
#define UNIV_VINT_MAP0__LINT1               0x00000070
#define UNIV_VINT_MAP0__LINT0               0x00000007
#define UNIV_VINT_MAP0__LINT(lint,vint)     (((vint) & 0x7) << (4 * (lint))
#define UNIV_VINT_MAP1__SW_INT              0x00070000
#define UNIV_VINT_MAP1__SW_INT_LEVEL(x)     (((x) & 0x7) << 16)
#define UNIV_VINT_MAP1__VERR                0x00000700
#define UNIV_VINT_MAP1__VERR_LEVEL(x)       (((x) & 0x7) << 8)
#define UNIV_VINT_MAP1__LERR                0x00000070
#define UNIV_VINT_MAP1__LERR_LEVEL(x)       (((x) & 0x7) << 4)
#define UNIV_VINT_MAP1__DMA                 0x00000007
#define UNIV_VINT_MAP1__DMA_LEVEL(x)        (((x) & 0x7) << 0)
#define UNIV_VINT_MAP2__MBOX3               0x00007000
#define UNIV_VINT_MAP2__MBOX2               0x00000700
#define UNIV_VINT_MAP2__MBOX1               0x00000070
#define UNIV_VINT_MAP2__MBOX0               0x00000007
#define UNIV_VINT_MAP2__MBOX(mb,vint)       (((mb) & 0x7) << (4 * (vint))

  /*
   * Interrupt status/id out register
   */
#define UNIV_STATID__STAT_ID(x)             ((unsigned int)(((x) & 0xFF) << 24))

  /*
   * VIRQ status/id register
   */
#define UNIV_V_STATID__ERR                  0x00000100
#define UNIV_V_STATID__STATID               0x000000FF


  /*
   * Semaphore registers
   */
#define UNIV_SEM_MIN                        0
#define UNIV_SEM_MAX                        7
#define UNIV_SEM__SEM                       0x80
#define UNIV_SEM__TAG                       0x7F


  /* 
   *  Master control register
   */
#define UNIV_MAST_CTL__MAXRTRY              0xF0000000
#define UNIV_MAST_CTL__MAXRTRY__960         0xF0000000
#define UNIV_MAST_CTL__MAXRTRY__896         0xE0000000
#define UNIV_MAST_CTL__MAXRTRY__832         0xD0000000
#define UNIV_MAST_CTL__MAXRTRY__768         0xC0000000
#define UNIV_MAST_CTL__MAXRTRY__704         0xB0000000
#define UNIV_MAST_CTL__MAXRTRY__640         0xA0000000
#define UNIV_MAST_CTL__MAXRTRY__576         0x90000000
#define UNIV_MAST_CTL__MAXRTRY__512         0x80000000
#define UNIV_MAST_CTL__MAXRTRY__448         0x70000000
#define UNIV_MAST_CTL__MAXRTRY__384         0x60000000
#define UNIV_MAST_CTL__MAXRTRY__320         0x50000000
#define UNIV_MAST_CTL__MAXRTRY__256         0x40000000
#define UNIV_MAST_CTL__MAXRTRY__192         0x30000000
#define UNIV_MAST_CTL__MAXRTRY__128         0x20000000
#define UNIV_MAST_CTL__MAXRTRY__64          0x10000000
#define UNIV_MAST_CTL__MAXRTRY__FOREVER     0x00000000
#define UNIV_MAST_CTL__PWON                 0x0F000000
#define UNIV_MAST_CTL__PWON__EARLY_RELEASE  0x0F000000
#define UNIV_MAST_CTL__PWON__4096           0x05000000
#define UNIV_MAST_CTL__PWON__2048           0x04000000
#define UNIV_MAST_CTL__PWON__1024           0x03000000
#define UNIV_MAST_CTL__PWON__512            0x02000000
#define UNIV_MAST_CTL__PWON__256            0x01000000
#define UNIV_MAST_CTL__PWON__128            0x00000000
#define UNIV_MAST_CTL__VRL                  0x00C00000
#define UNIV_MAST_CTL__VRL__3               0x00C00000
#define UNIV_MAST_CTL__VRL__2               0x00800000
#define UNIV_MAST_CTL__VRL__1               0x00400000
#define UNIV_MAST_CTL__VRL__0               0x00000000
#define UNIV_MAST_CTL__VRM                  0x00200000
#define UNIV_MAST_CTL__VRM__FAIR            0x00200000
#define UNIV_MAST_CTL__VRM__DEMAND          0x00000000
#define	UNIV_MAST_CTL__VREL                 0x00100000
#define	UNIV_MAST_CTL__VREL__ON_REQ         0x00100000
#define UNIV_MAST_CTL__VREL__WHEN_DONE      0x00000000
#define UNIV_MAST_CTL__VOWN                 0x00080000
#define UNIV_MAST_CTL__VOWN_ACK             0x00040000
#define UNIV_MAST_CTL__PABS                 0x00001000
#define UNIV_MAST_CTL__PABS__128            0x00002000
#define UNIV_MAST_CTL__PABS__64             0x00001000
#define UNIV_MAST_CTL__PABS__32             0x00000000
#define UNIV_MAST_CTL__BUS_NO(x)            ((x) & 0xFF)


  /* 
   *  Miscellaneous control register
   */
#define UNIV_MISC_CTL__VBTO                 0xF0000000
#define UNIV_MISC_CTL__VBTO__1024           0x70000000
#define UNIV_MISC_CTL__VBTO__512            0x60000000
#define UNIV_MISC_CTL__VBTO__256            0x50000000
#define UNIV_MISC_CTL__VBTO__128            0x40000000
#define UNIV_MISC_CTL__VBTO__64             0x30000000
#define UNIV_MISC_CTL__VBTO__32             0x20000000
#define UNIV_MISC_CTL__VBTO__16             0x10000000
#define UNIV_MISC_CTL__VBTO__DISABLE        0x00000000
#define UNIV_MISC_CTL__VARB                 0x04000000
#define UNIV_MISC_CTL__VARB__PRIORITY       0x04000000
#define UNIV_MISC_CTL__VARB__ROUND_ROBIN    0x00000000
#define	UNIV_MISC_CTL__VARBTO               0x03000000
#define	UNIV_MISC_CTL__VARBTO__256          0x02000000
#define UNIV_MISC_CTL__VARBTO__16           0x01000000
#define UNIV_MISC_CTL__VARBTO__DISABLE      0x00000000
#define UNIV_MISC_CTL__SW_LRST              0x00800000
#define UNIV_MISC_CTL__SW_SRST              0x00400000
#define UNIV_MISC_CTL__BI                   0x00100000
#define UNIV_MISC_CTL__ENGBI                0x00080000
#define UNIV_MISC_CTL__RESCIND              0x00040000
#define UNIV_MISC_CTL__SYSCON               0x00020000
#define UNIV_MISC_CTL__V64AUTO              0x00010000


 /*
  * Miscellaneous status register
  */
#define UNIV_MISC_STAT__LCLSIZE__64         0x40000000
#define UNIV_MISC_STAT__DY4AUTO             0x08000000
#define UNIV_MISC_STAT__MYBBSY              0x00200000
#define UNIV_MISC_STAT__DY4_DONE            0x00080000
#define UNIV_MISC_STAT__TXFE                0x00040000
#define UNIV_MISC_STAT__RXFE                0x00020000
#define UNIV_MISC_STAT__DY4AUTOID           0x0000FF00

  /*
   * User address modifier codes register
   */
#define UNIV_USER_AM__USER1AM               0x3C000000
#define UNIV_USER_AM__USER2AM               0x003C0000

  /*
   * Universe II specific register (used to control filters).
   */
#define UNIV_U2SPEC__DS                     0x00004000
#define UNIV_U2SPEC__AS                     0x00002000
#define UNIV_U2SPEC__DTKFLT                 0x00001000
#define UNIV_U2SPEC__MAST                   0x00000400
#define UNIV_U2SPEC__READ                   0x00000300
#define UNIV_U2SPEC__READ__DEFAULT          0x00000000
#define UNIV_U2SPEC__READ__FAST             0x00000100
#define UNIV_U2SPEC__READ__NODELAY          0x00000200
#define UNIV_U2SPEC__POS                    0x00000004
#define UNIV_U2SPEC__PRE                    0x00000001

  /* 
   * VMEbus slave image control registers
   */
#define UNIV_VSI_CTL__EN                    0x80000000
#define UNIV_VSI_CTL__PWEN                  0x40000000
#define UNIV_VSI_CTL__PREN                  0x20000000
#define UNIV_VSI_CTL__PGM                   0x00C00000
#define UNIV_VSI_CTL__PGM__BOTH             0x00C00000
#define UNIV_VSI_CTL__PGM__PROGRAM          0x00800000
#define UNIV_VSI_CTL__PGM__DATA             0x00400000
#define UNIV_VSI_CTL__SUPER                 0x00300000
#define UNIV_VSI_CTL__SUPER__BOTH           0x00300000
#define UNIV_VSI_CTL__SUPER__SUPER          0x00200000
#define UNIV_VSI_CTL__SUPER__USER           0x00100000
#define UNIV_VSI_CTL__VAS                   0x00070000
#define UNIV_VSI_CTL__VAS__USER2            0x00070000
#define UNIV_VSI_CTL__VAS__USER1            0x00060000
#define UNIV_VSI_CTL__VAS__A32              0x00020000
#define UNIV_VSI_CTL__VAS__A24              0x00010000
#define UNIV_VSI_CTL__VAS__A16              0x00000000
#define UNIV_VSI_CTL__LD64EN                0x00000080
#define UNIV_VSI_CTL__LLRMW                 0x00000040 /* Don't USE !! */
#define UNIV_VSI_CTL__LAS                   0x00000003
#define UNIV_VSI_CTL__LAS__CONFIG           0x00000002
#define UNIV_VSI_CTL__LAS__IO               0x00000001
#define UNIV_VSI_CTL__LAS__MEM              0x00000000

#define UNIV_VSI__4KB_MASK                  0x00000FFF /* LSI 0,4 */
#define UNIV_VSI__64KB_MASK                 0x0000FFFF /* LSI 1,2,3,5,6,7 */


  /* 
   *  Location monitor control register
   */
#define UNIV_LM_CTL__EN                     0x80000000
#define UNIV_LM_CTL__PGM                    0x00C00000
#define UNIV_LM_CTL__PGM__BOTH              0x00C00000
#define UNIV_LM_CTL__PGM__PROGRAM           0x00800000
#define UNIV_LM_CTL__PGM__DATA              0x00400000
#define UNIV_LM_CTL__SUPER                  0x00300000
#define UNIV_LM_CTL__SUPER__BOTH            0x00300000
#define UNIV_LM_CTL__SUPER__SUPER           0x00200000
#define UNIV_LM_CTL__SUPER__USER            0x00100000
#define UNIV_LM_CTL__VAS                    0x00030000
#define UNIV_LM_CTL__VAS__USER2             0x00070000
#define UNIV_LM_CTL__VAS__USER1             0x00060000
#define UNIV_LM_CTL__VAS__A32               0x00020000
#define UNIV_LM_CTL__VAS__A24               0x00010000
#define UNIV_LM_CTL__VAS__A16               0x00000000


  /* 
   * VMEbus register access control register
   */
#define UNIV_VRAI_CTL__EN                   0x80000000
#define UNIV_VRAI_CTL__PGM                  0x00C00000
#define UNIV_VRAI_CTL__PGM__BOTH            0x00C00000
#define UNIV_VRAI_CTL__PGM__PROGRAM         0x00800000
#define UNIV_VRAI_CTL__PGM__DATA            0x00400000
#define UNIV_VRAI_CTL__SUPER                0x00300000
#define UNIV_VRAI_CTL__SUPER__BOTH          0x00300000
#define UNIV_VRAI_CTL__SUPER__SUPER         0x00200000
#define UNIV_VRAI_CTL__SUPER__USER          0x00100000
#define UNIV_VRAI_CTL__VAS                  0x00070000
#define UNIV_VRAI_CTL__VAS__USER2           0x00070000
#define UNIV_VRAI_CTL__VAS__USER1           0x00060000
#define UNIV_VRAI_CTL__VAS__A32             0x00020000
#define UNIV_VRAI_CTL__VAS__A24             0x00010000
#define UNIV_VRAI_CTL__VAS__A16             0x00000000


 /*
  * VMEbus CSR control register
  */
#define UNIV_VCSR_CTL__EN                   0x80000000
#define UNIV_VCSR_CTL__LAS                  0x00000003
#define UNIV_VCSR_CTL__LAS__CONFIG          0x00000002
#define UNIV_VCSR_CTL__LAS__IO              0x00000001
#define UNIV_VCSR_CTL__LAS__MEM             0x00000000

  /*
   * VMEbus CSR translation offset register
   */
#define UNIV_VCSR_TO__TO                    0xFFF80000

  /*
   * VMEbus AM code error log
   */
#define UNIV_V_AMERR__AMERR                 0xFC000000
#define UNIV_V_AMERR__IACK                  0x02000000
#define UNIV_V_AMERR__M_ERR                 0x01000000
#define UNIV_V_AMERR__V_STAT                0x00800000
#define UNIV_BERR_AM(status)  (((status) & UNIV_V_AMERR__AMERR) >> 26)

 /*
  * VMEbus CSR bit clear and set registers
  */
#define UNIV_VCSR__RESET                    0x80000000
#define UNIV_VCSR__SYSFAIL                  0x40000000
#define UNIV_VCSR__FAIL                     0x20000000

  /*
   * VMEbus CSR base address register
   */
#define UNIV_VCSR_BS__BS                    0xF8000000

#ifdef __cplusplus
}

#endif                          /* __cplusplus */

#endif                          /* __UNIVERSE_H */

