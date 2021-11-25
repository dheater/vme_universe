
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


#ifndef __VMIVME_H
#define __VMIVME_H


  /*
   * PLX device registers
   */
#define VMIVMEP_COMM                       0x00
#define VMIVMEP_B_INT_STATUS               0x04
#define VMIVMEP_B_INT_MASK                 0x08
#define VMIVMEP_B_ADDRESS                  0x0C
#define VMIVMEP_CSR                        0x68


  /*
   * PLX comm register
   */
#define VMIVMEP_COMM__VME_EN               0x0800
#define VMIVMEP_COMM__MB_M3                0x0400
#define VMIVMEP_COMM__MB_M2                0x0200
#define VMIVMEP_COMM__MB_M1                0x0100
#define VMIVMEP_COMM__MB_M0                0x0080
#define VMIVMEP_COMM__BERRIM               0x0040
#define VMIVMEP_COMM__BTOV                 0x0030
#define VMIVMEP_COMM__BTOV__1000           0x0030
#define VMIVMEP_COMM__BTOV__256            0x0020
#define VMIVMEP_COMM__BTOV__64             0x0010
#define VMIVMEP_COMM__BTOV__16             0x0000
#define VMIVMEP_COMM__BTO                  0x0080
#define VMIVMEP_COMM__ABLE                 0x0004
#define VMIVMEP_COMM__SEC                  0x0002
#define VMIVMEP_COMM__MEC                  0x0001


  /*
   * PLX b_int_status register
   */
#define VMIVMEP_B_INT_STATUS__BERR_S       0x00000001
#define VMIVMEP_B_INT_STATUS__AM           0x003F0000
#define VMIVMEP_BERR_AM(status)  (((status) & VMIVMEP_B_INT_STATUS__AM) >> 16)


  /*
   * PLX b_int_mask register
   */
#define VMIVMEP_B_INT_MASK__BERR_M         0x0001


  /*
   * PLX CSR register
   */
#define VMIVMEP_CSR__INTR_EN               0x0001


  /*
   * D8000 device registers
   */
#define VMIVMEI_BASE                       0xD8000
#define VMIVMEI_COMM                       0x0E
#define VMIVMEI_VBAR                       0x10
#define VMIVMEI_VBAMR                      0x14
#define VMIVMEI_BID                        0x16


  /*
   * D8000 comm register
   */
#define VMIVMEI_COMM__VME_EN               0x0800
#define VMIVMEI_COMM__BYPASS               0x0400
#define VMIVMEI_COMM__WTDSYS               0x0100
#define VMIVMEI_COMM__BERRST               0x0080
#define VMIVMEI_COMM__BERRI                0x0040
#define VMIVMEI_COMM__BTOV                 0x0030
#define VMIVMEI_COMM__BTOV__1000           0x0030
#define VMIVMEI_COMM__BTOV__256            0x0020
#define VMIVMEI_COMM__BTOV__64             0x0010
#define VMIVMEI_COMM__BTOV__16             0x0000
#define VMIVMEI_COMM__BTO                  0x0080
#define VMIVMEI_COMM__ABLE                 0x0004
#define VMIVMEI_COMM__SEC                  0x0002
#define VMIVMEI_COMM__MEC                  0x0001


  /*
   * D8000 VMEbus error address modifier register
   */
#define VMIVMEI_VBAMR__AM                  0x003F

  /*
   * FPGA device registers
   */
#define VMIVMEF_COMM                       0x00
#define VMIVMEF_VBAMR                      0x04
#define VMIVMEF_VBAR                       0x08


  /*
   * FPGA comm register
   */
#define VMIVMEF_COMM__VME_EN               0x00000800
#define VMIVMEF_COMM__BYPASS               0x00000400
#define VMIVMEF_COMM__WTDSYS               0x00000100
#define VMIVMEF_COMM__BERRST               0x00000080
#define VMIVMEF_COMM__BERRI                0x00000040
#define VMIVMEF_COMM__BTOV                 0x00000030
#define VMIVMEF_COMM__BTOV__1000           0x00000030
#define VMIVMEF_COMM__BTOV__256            0x00000020
#define VMIVMEF_COMM__BTOV__64             0x00000010
#define VMIVMEF_COMM__BTOV__16             0x00000000
#define VMIVMEF_COMM__BTO                  0x00000080
#define VMIVMEF_COMM__ABLE                 0x00000004
#define VMIVMEF_COMM__SEC                  0x00000002
#define VMIVMEF_COMM__MEC                  0x00000001


  /*
   * FPGA comm register
   */
#define VMIVMEF_VBAMR__AM                  0x0000003F


typedef enum
{
  VMIVME_NONE,
  VMIVME_PLX,
  VMIVME_ISA,
  VMIVME_FPGA
}
vmic_reg_type_t;


#endif                          /* __VMIVME_H */
