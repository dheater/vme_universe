/*
===============================================================================
                            COPYRIGHT NOTICE

    Copyright (C) 2004 VMIC
    International Copyright Secured.  All Rights Reserved.

===============================================================================

*/

/* includes */
#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif
#include <linux/module.h>

#include "hw_diag.h"
#include "gt64360r.h"
#include <asm/io.h>

//extern uint32_t mv64360_base;
unsigned int gtInternalRegBaseAddr; 

/* defines */
#define GT_CORE_SHOW
/****************************************/
/*          GENERAL Definitions         */
/****************************************/
#define NO_BIT          0x00000000
#define BIT0            0x00000001
#define BIT1            0x00000002
#define BIT2            0x00000004
#define BIT3            0x00000008
#define BIT4            0x00000010
#define BIT5            0x00000020
#define BIT6            0x00000040
#define BIT7            0x00000080
#define BIT8            0x00000100
#define BIT9            0x00000200
#define BIT10           0x00000400
#define BIT11           0x00000800
#define BIT12           0x00001000
#define BIT13           0x00002000
#define BIT14           0x00004000
#define BIT15           0x00008000
#define BIT16           0x00010000
#define BIT17           0x00020000
#define BIT18           0x00040000
#define BIT19           0x00080000
#define BIT20           0x00100000
#define BIT21           0x00200000
#define BIT22           0x00400000
#define BIT23           0x00800000
#define BIT24           0x01000000
#define BIT25           0x02000000
#define BIT26           0x04000000
#define BIT27           0x08000000
#define BIT28           0x10000000
#define BIT29           0x20000000
#define BIT30           0x40000000
#define BIT31           0x80000000

/* PCI defines */
#define PCI_SELF                            32
#define PCI_MAX_DEVICES                     32
#define PCI_ERROR_CODE                      0xffffffff
#define PCI_DEV_NUMBER_MASK                 0x0000f800
#define PCI_FUNCTION_NUMBER_MASK            0x00000700
#define PCI_REG_OFFSET_MASK                 0x000000fc

#define PCI_CONFIG_ENABLE                          BIT31

#define	PCI_CFG_VENDOR_ID		0x00
#define	PCI_CFG_DEVICE_ID		0x02
#define	PCI_CFG_COMMAND			0x04
#define	PCI_CFG_STATUS			0x06
#define	PCI_CFG_REVISION		0x08
#define	PCI_CFG_PROGRAMMING_IF	0x09
#define	PCI_CFG_SUBCLASS		0x0a
#define	PCI_CFG_CLASS			0x0b
#define	PCI_CFG_CACHE_LINE_SIZE	0x0c
#define	PCI_CFG_LATENCY_TIMER	0x0d
#define	PCI_CFG_HEADER_TYPE		0x0e
#define	PCI_CFG_BIST			0x0f
#define	PCI_CFG_BASE_ADDRESS_0	0x10
#define	PCI_CFG_BASE_ADDRESS_1	0x14
#define	PCI_CFG_BASE_ADDRESS_2	0x18
#define	PCI_CFG_BASE_ADDRESS_3	0x1c
#define	PCI_CFG_BASE_ADDRESS_4	0x20
#define	PCI_CFG_BASE_ADDRESS_5	0x24
#define	PCI_CFG_CIS				0x28
#define	PCI_CFG_SUB_VENDER_ID	0x2c
#define	PCI_CFG_SUB_SYSTEM_ID	0x2e
#define	PCI_CFG_EXPANSION_ROM	0x30
#define PCI_CFG_CAP_PTR			0x34
#define	PCI_CFG_RESERVED_0		0x35
#define	PCI_CFG_RESERVED_1		0x38
#define	PCI_CFG_DEV_INT_LINE	0x3c
#define	PCI_CFG_DEV_INT_PIN		0x3d
#define	PCI_CFG_MIN_GRANT		0x3e
#define	PCI_CFG_MAX_LATENCY		0x3f
#define PCI_CFG_SPECIAL_USE     0x41
#define PCI_CFG_MODE            0x43


/* There are 21 memory windows dedicated for the varios interfaces (PCI,
   devCS (devices), CS(DDR), interenal registers and SRAM) used by the CPU's
   address decoding mechanism. */
typedef enum _memoryWindow {CS_0_WINDOW = BIT0, CS_1_WINDOW = BIT1,
                            CS_2_WINDOW = BIT2, CS_3_WINDOW = BIT3,
                            DEVCS_0_WINDOW = BIT4, DEVCS_1_WINDOW = BIT5,
                            DEVCS_2_WINDOW = BIT6, DEVCS_3_WINDOW = BIT7,
                            BOOT_CS_WINDOW = BIT8, PCI_0_IO_WINDOW = BIT9,
                            PCI_0_MEM0_WINDOW = BIT10,
                            PCI_0_MEM1_WINDOW = BIT11,
                            PCI_0_MEM2_WINDOW = BIT12,
                            PCI_0_MEM3_WINDOW = BIT13, PCI_1_IO_WINDOW = BIT14,
                            PCI_1_MEM0_WINDOW = BIT15, PCI_1_MEM1_WINDOW =BIT16,
                            PCI_1_MEM2_WINDOW = BIT17, PCI_1_MEM3_WINDOW =BIT18,
                            INTEGRATED_SRAM_WINDOW = BIT19,
                            INTERNAL_SPACE_WINDOW = BIT20,
                            ALL_WINDOWS = 0X1FFFFF
                           } MEMORY_WINDOW;

/* typedefs */

typedef enum _pciInternalBAR{PCI_CS0_BAR, PCI_CS1_BAR, PCI_CS2_BAR,
                             PCI_CS3_BAR, PCI_DEV_CS0_BAR, PCI_DEV_CS1_BAR,
                             PCI_DEV_CS2_BAR, PCI_DEV_CS3_BAR, PCI_BOOT_CS_BAR,
                             PCI_MEM_MAPPED_INTERNAL_REG_BAR,
                             PCI_IO_MAPPED_INTERNAL_REG_BAR

/* These values are not valid if the MV device does not support the features. */
#ifdef INCLUDE_P2P
                             ,PCI_P2P_MEM0_BAR, PCI_P2P_MEM1_BAR, PCI_P2P_IO_BAR
#endif

#ifdef INCLUDE_CPU_MAPPING 
                             ,PCI_CPU_BAR
#endif

#ifdef INCLUDE_INTERNAL_SRAM
                             ,PCI_INTERNAL_SRAM_BAR
#endif
                            } PCI_INTERNAL_BAR;


#define MV_WORD_SWAP(X)  ((((X)&0xff)<<24) | (((X)&0xff00)<<8) | (((X)&0xff0000)>>8) | (((X)&0xff000000)>>24))

#define GT_REG_READ(offset,pData) \
   (*(pData) = MV_WORD_SWAP(*((volatile unsigned int *) \
   (gtInternalRegBaseAddr | offset))))

#define GT_REG_WRITE(offset, data) \
            ((*((volatile unsigned int *) \
            (gtInternalRegBaseAddr | \
            offset))) = MV_WORD_SWAP((data)))

unsigned int GTREGREAD (int gt_reg)
{
	unsigned int reg_val;
	GT_REG_READ(gt_reg, &reg_val);
	return reg_val;
}

#if defined(GT_CORE_SHOW)
/*******************************************************************************
* gtPciGetBusNumber - Get PCI bus number.
*
* DESCRIPTION:
*       Read the PCI device number from the PCI_0/1_P2P_CONFIG register.
*
*       The P2P configuration register fields are:
*
*        31    29 28       24 23       16 15      8 7       0     <=bit Number
*       |Reserved| MV Device | Local bus | 2nd bus | 2nd bus |
*       |        |  Number   |  Number   |  High   | Low     |    <=field Name
*
*      NOTE:
*                            Conventional PCI  |  PCI-X bus
*          Local bus Number:       0           |   0xFF
*          MV Device Number:       0           |   0x1F
*
* INPUT:
*       pciInterfaceNumber - The PCI interface number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       PCI bus number.
*
*******************************************************************************/
unsigned int gtPciGetBusNumber(unsigned int pciInterfaceNumber)
{
    switch(pciInterfaceNumber)
    {
    case 0:
        return ((GTREGREAD(PCI_0_P2P_CONFIG) >> 16) & 0xFF);
    case 1:
        return ((GTREGREAD(PCI_1_P2P_CONFIG) >> 16) & 0xFF);
    default:
        return PCI_ERROR_CODE;
    }
}

/*******************************************************************************
* gtPciGetDevNumber - Get PCI Device number.
*
* DESCRIPTION:
*       Read the PCI device number from the PCI_0/1_P2P_CONFIG register.
*
*       The P2P configuration register fields are:
*
*        31    29 28       24 23       16 15      8 7       0     <=bit Number
*       |Reserved| MV Device | Local bus | 2nd bus | 2nd bus |
*       |        |  Number   |  Number   |  High   | Low     |    <=field Name
*
*      NOTE:
*                            Conventional PCI  |  PCI-X bus
*          Local bus Number:       0           |   0xFF
*          MV Device Number:       0           |   0x1F
*
* INPUT:
*       pciInterfaceNumber - The PCI interface number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       PCI Device number.
*
*******************************************************************************/
unsigned int gtPciGetDevNumber(unsigned int pciInterfaceNumber)
{
    switch(pciInterfaceNumber)
    {
    case 0:
        return ((GTREGREAD(PCI_0_P2P_CONFIG) >> 24) & 0x1F);
    case 1:
        return ((GTREGREAD(PCI_1_P2P_CONFIG) >> 24) & 0x1F);
    default:
        return PCI_ERROR_CODE;
    }
}


/*******************************************************************************
* gtPci0ReadConfigReg - Read from a PCI configuration register.
*
* DESCRIPTION:
*       The MV holds two registers to support configuration accesses as defined
*       in the PCI spec rev 2.2: Configuration Address and Configuration Data
*       registers. The mechanism for accessing configuration space is to write a
*       value into the Configuration Address register that specifies the PCI bus
*       number (this function use the value of 0 by default for this parameter),
*       Device number on the bus, Function number within the device (will be
*       combined with the register offset) and Configuration register offset
*       within the device/function being accessed. A subsequent read to the PCI
*       Configuration Data register causes the MV to translate that
*       Configuration Address value to the requested cycle on the PCI bus (in
*       this case - read) or internal configuration space. This function reads
*       from an agent's configuration register at any of the 8 possible function
*       in its Configuration Space Header.
*
*      EXAMPLE:
*       The value 0x004 is combined from the function number (bits[11:8]) and
*       the register offset (bits[7:0]) in the Configuration Space Header. In
*       this case, the fuction number is 0 and the register offset is 0x04.
*
*       ...
*       data = gtPci0ReadConfigReg(0x004,6);
*       ...
*
*       The configuration address register (0xCF8) fields are:
*
*             31 30    24 23  16 15  11 10     8 7      2  0     <=bit Number
*       |congif|Reserved|  Bus |Device|Function|Register|00|
*       |Enable|        |Number|Number| Number | Number |  |    <=field Name
*
*
* INPUT:
*       regOffset - The register offset PCI configuration Space Header combined
*                   with the function number as shown in the example above.
*       pciDevNum - The agent's device number.
*
* OUTPUT:
*       PCI read configuration cycle.
*
* RETURN:
*       32 bit read data from the agent's configuration register.
*       if the data == PCI_ERROR_CODE check the master abort bit in the cause
*       register to make sure the data is valid.
*
*******************************************************************************/
unsigned int gtPci0ReadConfigReg (unsigned int regOffset,unsigned int pciDevNum)
{
    volatile unsigned int DataForRegCf8;
   	unsigned int data;
    unsigned int functionNum;
    unsigned int busNum;

    if(pciDevNum > PCI_MAX_DEVICES) /* illegal device Number */
        return PCI_ERROR_CODE;
    if(pciDevNum == PCI_SELF) /* configure our configuration space. */
    {
        pciDevNum = gtPciGetDevNumber(0);
    }
    busNum = gtPciGetBusNumber(0) << 16;
    functionNum = regOffset & PCI_FUNCTION_NUMBER_MASK;
    if((functionNum >> 8) > 0x7) /* illegal function Number */
        return PCI_ERROR_CODE;
    pciDevNum = ( pciDevNum << 11) & PCI_DEV_NUMBER_MASK;
    regOffset = regOffset & PCI_REG_OFFSET_MASK;
    DataForRegCf8 = PCI_CONFIG_ENABLE|busNum|pciDevNum|functionNum|regOffset;
    GT_REG_WRITE(PCI_0_CONFIG_ADDR,DataForRegCf8);
    /* Check that the data was written in the right order to both CF8 and CFC
       registers */
    if (GTREGREAD(PCI_0_CONFIG_ADDR) != DataForRegCf8)
        return PCI_ERROR_CODE;
    GT_REG_READ(PCI_0_CONFIG_DATA_VIRTUAL_REG, &data);
    return data;
}


/*******************************************************************************
* gtPci1ReadConfigReg - Read from a PCI configuration register.
*
* DESCRIPTION:
*       The MV holds two registers to support configuration accesses as defined
*       in the PCI spec rev 2.2: Configuration Address and Configuration Data
*       registers. The mechanism for accessing configuration space is to write a
*       value into the Configuration Address register that specifies the PCI bus
*       number (this function use the value of 0 by default for this parameter),
*       Device number on the bus, Function number within the device (will be
*       combined with the register offset) and Configuration register offset
*       within the device/function being accessed. A subsequent read to the PCI
*       Configuration Data register causes the MV to translate that
*       Configuration Address value to the requested cycle on the PCI bus (in
*       this case - read) or internal configuration space. This function reads
*       from an agent's configuration register at any of the 8 possible function
*       in its Configuration Space Header.
*
*      EXAMPLE:
*       The value 0x004 is combined from the function number (bits[11:8]) and
*       the register offset (bits[7:0]) in the Configuration Space Header. In
*       this case, the fuction number is 0 and the register offset is 0x04.
*
*       ...
*       data = gtPci1ReadConfigReg(0x004,6);
*       ...
*
*       The configuration address register (0xCF8) fields are:
*
*             31 30    24 23  16 15  11 10     8 7      2  0     <=bit Number
*       |congif|Reserved|  Bus |Device|Function|Register|00|
*       |Enable|        |Number|Number| Number | Number |  |    <=field Name
*
*
* INPUT:
*       regOffset - The register offset PCI configuration Space Header combined
*                   with the function number as shown in the example above.
*       pciDevNum - The agent's device number.
*
* OUTPUT:
*       PCI read configuration cycle.
*
* RETURN:
*       32 bit read data from the agent's configuration register.
*       if the data == PCI_ERROR_CODE check the master abort bit in the cause
*       register to make sure the data is valid.
*
*******************************************************************************/
unsigned int gtPci1ReadConfigReg (unsigned int regOffset,unsigned int pciDevNum)
{
    volatile unsigned int DataForRegCf8;
   	unsigned int data;
    unsigned int functionNum;
    unsigned int busNum;

    if(pciDevNum > PCI_MAX_DEVICES) /* illegal device Number */
        return PCI_ERROR_CODE;
    if(pciDevNum == PCI_SELF) /* configure our configuration space. */
    {
        pciDevNum = gtPciGetDevNumber(1);
    }
    busNum = gtPciGetBusNumber(1) << 16;
    functionNum = regOffset & PCI_FUNCTION_NUMBER_MASK;
    if((functionNum >> 8) > 0x7) /* illegal function Number */
        return PCI_ERROR_CODE;
    pciDevNum = ( pciDevNum << 11) & PCI_DEV_NUMBER_MASK;
    regOffset = regOffset & PCI_REG_OFFSET_MASK;
    DataForRegCf8 = PCI_CONFIG_ENABLE|busNum|pciDevNum|functionNum|regOffset;
    GT_REG_WRITE(PCI_1_CONFIG_ADDR,DataForRegCf8);
    /* Check that the data was written in the right order to both CF8 and CFC
       registers */
    if (GTREGREAD(PCI_1_CONFIG_ADDR) != DataForRegCf8)
        return PCI_ERROR_CODE;
    GT_REG_READ(PCI_1_CONFIG_DATA_VIRTUAL_REG, &data);
    return data;
}


/*******************************************************************************
* gtCoreShow - show MV64360 Registers
*
*/  
void gtCoreShow(void)
{
#define GTPRINTREG(gtReg)  printk(KERN_NOTICE "%-42.41s (0x%04x) = 0x%08x\n",#gtReg, gtReg, GTREGREAD(gtReg)) 

#define GTPRINTCONFIGREG(gtReg) { \
printk(KERN_NOTICE "PCI0: %-42.41s (0x%04x) = 0x%08x\n",#gtReg, gtReg, gtPci0ReadConfigReg(gtReg,PCI_SELF)); \
printk(KERN_NOTICE "PCI1: %-42.41s (0x%04x) = 0x%08x\n",#gtReg, gtReg, gtPci1ReadConfigReg(gtReg,PCI_SELF)); \
}



    printk(KERN_NOTICE "A.3 CPU Address Decode Registers\n"); 
    printk(KERN_NOTICE "DDR SDRAM BAR and size registers\n"); 
    GTPRINTREG(CS_0_BASE_ADDR);
    GTPRINTREG(CS_0_SIZE);
    GTPRINTREG(CS_1_BASE_ADDR);
    GTPRINTREG(CS_1_SIZE);
    GTPRINTREG(CS_2_BASE_ADDR);
    GTPRINTREG(CS_2_SIZE);
    GTPRINTREG(CS_3_BASE_ADDR);
    GTPRINTREG(CS_3_SIZE);

    printk(KERN_NOTICE "\nDevices BAR and size registers\n"); 
    GTPRINTREG(DEV_CS0_BASE_ADDR);
    GTPRINTREG(DEV_CS0_SIZE);
    GTPRINTREG(DEV_CS1_BASE_ADDR);
    GTPRINTREG(DEV_CS1_SIZE);
    GTPRINTREG(DEV_CS2_BASE_ADDR);
    GTPRINTREG(DEV_CS2_SIZE);
    GTPRINTREG(DEV_CS3_BASE_ADDR);
    GTPRINTREG(DEV_CS3_SIZE);
    GTPRINTREG(BOOTCS_BASE_ADDR);
    GTPRINTREG(BOOTCS_SIZE);

    printk(KERN_NOTICE "\nPCI 0 BAR and size registers\n"); 
    GTPRINTREG(PCI_0_IO_BASE_ADDR);
    GTPRINTREG(PCI_0_IO_SIZE);
    GTPRINTREG(PCI_0_MEMORY0_BASE_ADDR);
    GTPRINTREG(PCI_0_MEMORY0_SIZE);     
    GTPRINTREG(PCI_0_MEMORY1_BASE_ADDR);
    GTPRINTREG(PCI_0_MEMORY1_SIZE);     
    GTPRINTREG(PCI_0_MEMORY2_BASE_ADDR);
    GTPRINTREG(PCI_0_MEMORY2_SIZE);     
    GTPRINTREG(PCI_0_MEMORY3_BASE_ADDR);
    GTPRINTREG(PCI_0_MEMORY3_SIZE);     

    printk(KERN_NOTICE "\nPCI 1 BAR and size registers\n"); 
    GTPRINTREG(PCI_1_IO_BASE_ADDR);
    GTPRINTREG(PCI_1_IO_SIZE);
    GTPRINTREG(PCI_1_MEMORY0_BASE_ADDR);
    GTPRINTREG(PCI_1_MEMORY0_SIZE);     
    GTPRINTREG(PCI_1_MEMORY1_BASE_ADDR);
    GTPRINTREG(PCI_1_MEMORY1_SIZE);     
    GTPRINTREG(PCI_1_MEMORY2_BASE_ADDR);
    GTPRINTREG(PCI_1_MEMORY2_SIZE);     
    GTPRINTREG(PCI_1_MEMORY3_BASE_ADDR);
    GTPRINTREG(PCI_1_MEMORY3_SIZE);     

    printk(KERN_NOTICE "\nSRAM base address\n"); 
    GTPRINTREG(INTEGRATED_SRAM_BASE_ADDR);     

    printk(KERN_NOTICE "\ninternal registers space base address\n"); 
    GTPRINTREG(INTERNAL_SPACE_BASE_ADDR);     

    printk(KERN_NOTICE "\nEnables the CS , DEV_CS , PCI 0 and PCI 1 windows above\n"); 
    GTPRINTREG(BASE_ADDR_ENABLE);     

    printk(KERN_NOTICE "\nPCI remap registers\n"); 
    GTPRINTREG(PCI_0_IO_ADDR_REMAP          ); 
    GTPRINTREG(PCI_0_MEMORY0_LOW_ADDR_REMAP ); 
    GTPRINTREG(PCI_0_MEMORY0_HIGH_ADDR_REMAP); 
    GTPRINTREG(PCI_0_MEMORY1_LOW_ADDR_REMAP ); 
    GTPRINTREG(PCI_0_MEMORY1_HIGH_ADDR_REMAP); 
    GTPRINTREG(PCI_0_MEMORY2_LOW_ADDR_REMAP ); 
    GTPRINTREG(PCI_0_MEMORY2_HIGH_ADDR_REMAP); 
    GTPRINTREG(PCI_0_MEMORY3_LOW_ADDR_REMAP ); 
    GTPRINTREG(PCI_0_MEMORY3_HIGH_ADDR_REMAP); 
    GTPRINTREG(PCI_1_IO_ADDR_REMAP          ); 
    GTPRINTREG(PCI_1_MEMORY0_LOW_ADDR_REMAP ); 
    GTPRINTREG(PCI_1_MEMORY0_HIGH_ADDR_REMAP); 
    GTPRINTREG(PCI_1_MEMORY1_LOW_ADDR_REMAP ); 
    GTPRINTREG(PCI_1_MEMORY1_HIGH_ADDR_REMAP); 
    GTPRINTREG(PCI_1_MEMORY2_LOW_ADDR_REMAP ); 
    GTPRINTREG(PCI_1_MEMORY2_HIGH_ADDR_REMAP); 
    GTPRINTREG(PCI_1_MEMORY3_LOW_ADDR_REMAP ); 
    GTPRINTREG(PCI_1_MEMORY3_HIGH_ADDR_REMAP); 
    GTPRINTREG(CPU_PCI_0_HEADERS_RETARGET_CONTROL); 
    GTPRINTREG(CPU_PCI_0_HEADERS_RETARGET_BASE   ); 
    GTPRINTREG(CPU_PCI_1_HEADERS_RETARGET_CONTROL); 
    GTPRINTREG(CPU_PCI_1_HEADERS_RETARGET_BASE   ); 
    GTPRINTREG(CPU_GE_HEADERS_RETARGET_CONTROL   ); 
    GTPRINTREG(CPU_GE_HEADERS_RETARGET_BASE      ); 
    GTPRINTREG(CPU_IDMA_HEADERS_RETARGET_CONTROL ); 
    GTPRINTREG(CPU_IDMA_HEADERS_RETARGET_BASE    ); 

    printk(KERN_NOTICE "\nCPU Control Registers\n"); 
    GTPRINTREG(CPU_CONFIG);
    GTPRINTREG(CPU_MODE);
    GTPRINTREG(CPU_PADS_CALIBRATION);
    GTPRINTREG(CPU_MASTER_CONTROL);
    GTPRINTREG(CPU_CROSS_BAR_CONTROL_LOW);
    GTPRINTREG(CPU_CROSS_BAR_CONTROL_HIGH);
    GTPRINTREG(CPU_CROSS_BAR_TIMEOUT);

    printk(KERN_NOTICE "\nSMP RegisterS\n"); 
    GTPRINTREG(SMP_WHO_AM_I           );
    GTPRINTREG(SMP_CPU0_DOORBELL      );
    GTPRINTREG(SMP_CPU0_DOORBELL_CLEAR);
    GTPRINTREG(SMP_CPU1_DOORBELL      );
    GTPRINTREG(SMP_CPU1_DOORBELL_CLEAR);
    GTPRINTREG(SMP_CPU0_DOORBELL_MASK );
    GTPRINTREG(SMP_CPU1_DOORBELL_MASK );
    GTPRINTREG(SMP_SEMAPHOR0          );
    GTPRINTREG(SMP_SEMAPHOR1          );
    GTPRINTREG(SMP_SEMAPHOR2          );
    GTPRINTREG(SMP_SEMAPHOR3          );
    GTPRINTREG(SMP_SEMAPHOR4          );
    GTPRINTREG(SMP_SEMAPHOR5          );
    GTPRINTREG(SMP_SEMAPHOR6          );
    GTPRINTREG(SMP_SEMAPHOR7          );

    printk(KERN_NOTICE "\nCPU Sync Barrier Register\n"); 
    GTPRINTREG(CPU_0_SYNC_BARRIER_TRIGGER);
    GTPRINTREG(CPU_0_SYNC_BARRIER_VIRTUAL);
    GTPRINTREG(CPU_1_SYNC_BARRIER_TRIGGER);
    GTPRINTREG(CPU_1_SYNC_BARRIER_VIRTUAL);

    printk(KERN_NOTICE "\nCPU Access Protect\n"); 
    GTPRINTREG(CPU_PROTECT_WINDOW_0_BASE_ADDR);
    GTPRINTREG(CPU_PROTECT_WINDOW_0_SIZE     );
    GTPRINTREG(CPU_PROTECT_WINDOW_1_BASE_ADDR);
    GTPRINTREG(CPU_PROTECT_WINDOW_1_SIZE     );
    GTPRINTREG(CPU_PROTECT_WINDOW_2_BASE_ADDR);
    GTPRINTREG(CPU_PROTECT_WINDOW_2_SIZE     );
    GTPRINTREG(CPU_PROTECT_WINDOW_3_BASE_ADDR);
    GTPRINTREG(CPU_PROTECT_WINDOW_3_SIZE     );

    printk(KERN_NOTICE "\nCPU Error Report\n"); 
    GTPRINTREG(CPU_ERROR_ADDR_LOW ); 
    GTPRINTREG(CPU_ERROR_ADDR_HIGH); 
    GTPRINTREG(CPU_ERROR_DATA_LOW ); 
    GTPRINTREG(CPU_ERROR_DATA_HIGH); 
    GTPRINTREG(CPU_ERROR_PARITY   ); 
    GTPRINTREG(CPU_ERROR_CAUSE    ); 
    GTPRINTREG(CPU_ERROR_MASK     ); 

    printk(KERN_NOTICE "\nCPU Interface Debug Registers\n"); 
    GTPRINTREG(PUNIT_SLAVE_DEBUG_LOW  ); 
    GTPRINTREG(PUNIT_SLAVE_DEBUG_HIGH ); 
    GTPRINTREG(PUNIT_MASTER_DEBUG_LOW ); 
    GTPRINTREG(PUNIT_MASTER_DEBUG_HIGH); 
    GTPRINTREG(PUNIT_MMASK            ); 

    printk(KERN_NOTICE "\nIntegrated SRAM Registers\n"); 
    GTPRINTREG(SRAM_CONFIG           ); 
    GTPRINTREG(SRAM_TEST_MODE        ); 
    GTPRINTREG(SRAM_ERROR_CAUSE      ); 
    GTPRINTREG(SRAM_ERROR_ADDR       ); 
    GTPRINTREG(SRAM_ERROR_ADDR_HIGH  ); 
    GTPRINTREG(SRAM_ERROR_DATA_LOW   ); 
    GTPRINTREG(SRAM_ERROR_DATA_HIGH  ); 
    GTPRINTREG(SRAM_ERROR_DATA_PARITY); 

    printk(KERN_NOTICE "\nSDRAM Configuration\n"); 
    GTPRINTREG(SDRAM_CONFIG            );         
    GTPRINTREG(D_UNIT_CONTROL_LOW         );      
    GTPRINTREG(D_UNIT_CONTROL_HIGH           );   
    GTPRINTREG(SDRAM_TIMING_CONTROL_LOW        ); 
    GTPRINTREG(SDRAM_TIMING_CONTROL_HIGH       ); 
    GTPRINTREG(SDRAM_ADDR_CONTROL              ); 
    GTPRINTREG(SDRAM_OPEN_PAGES_CONTROL        ); 
    GTPRINTREG(SDRAM_OPERATION                 ); 
    GTPRINTREG(SDRAM_MODE                      ); 
    GTPRINTREG(EXTENDED_DRAM_MODE              ); 
    GTPRINTREG(SDRAM_CROSS_BAR_CONTROL_LOW     ); 
    GTPRINTREG(SDRAM_CROSS_BAR_CONTROL_HIGH    ); 
    GTPRINTREG(SDRAM_CROSS_BAR_TIMEOUT         ); 
    GTPRINTREG(SDRAM_ADDR_CTRL_PADS_CALIBRATION); 
    GTPRINTREG(SDRAM_DATA_PADS_CALIBRATION     ); 

    printk(KERN_NOTICE "\nSDRAM Error Report\n"); 
    GTPRINTREG(SDRAM_ERROR_DATA_LOW   ); 
    GTPRINTREG(SDRAM_ERROR_DATA_HIGH  ); 
    GTPRINTREG(SDRAM_ERROR_ADDR       ); 
    GTPRINTREG(SDRAM_RECEIVED_ECC     ); 
    GTPRINTREG(SDRAM_CALCULATED_ECC   ); 
    GTPRINTREG(SDRAM_ECC_CONTROL      ); 
    GTPRINTREG(SDRAM_ECC_ERROR_COUNTER); 

    printk(KERN_NOTICE "\nControlled Delay Line (CDL) Registers\n"); 
    GTPRINTREG(DFCDL_CONFIG0); 
    GTPRINTREG(DFCDL_CONFIG1); 
    GTPRINTREG(DLL_WRITE    ); 
    GTPRINTREG(DLL_READ     ); 
    GTPRINTREG(SRAM_ADDR    ); 
    GTPRINTREG(SRAM_DATA0   ); 
    GTPRINTREG(SRAM_DATA1   ); 
    GTPRINTREG(SRAM_DATA2   ); 
    GTPRINTREG(DFCL_PROBE   ); 

    printk(KERN_NOTICE "\nDebug Registers\n"); 
    GTPRINTREG(DUNIT_DEBUG_LOW ); 
    GTPRINTREG(DUNIT_DEBUG_HIGH); 
    GTPRINTREG(DUNIT_MMASK     ); 

    printk(KERN_NOTICE "\nDevice Parameters\n"); 
    GTPRINTREG(DEVICE_BANK0_PARAMETERS				); 
    GTPRINTREG(DEVICE_BANK1_PARAMETERS				); 
    GTPRINTREG(DEVICE_BANK2_PARAMETERS				); 
    GTPRINTREG(DEVICE_BANK3_PARAMETERS				); 
    GTPRINTREG(DEVICE_BOOT_BANK_PARAMETERS			); 
    GTPRINTREG(DEVICE_INTERFACE_CONTROL               ); 
    GTPRINTREG(DEVICE_INTERFACE_CROSS_BAR_CONTROL_LOW ); 
    GTPRINTREG(DEVICE_INTERFACE_CROSS_BAR_CONTROL_HIGH); 
    GTPRINTREG(DEVICE_INTERFACE_CROSS_BAR_TIMEOUT     ); 

    printk(KERN_NOTICE "\nDevice interrupt registers\n"); 
    GTPRINTREG(DEVICE_INTERRUPT_CAUSE); 
    GTPRINTREG(DEVICE_INTERRUPT_MASK); 
    GTPRINTREG(DEVICE_ERROR_ADDR	); 
    GTPRINTREG(DEVICE_ERROR_DATA   ); 
    GTPRINTREG(DEVICE_ERROR_PARITY   ); 

    printk(KERN_NOTICE "\nDevice debug registers\n"); 
    GTPRINTREG(DEVICE_DEBUG_LOW );
    GTPRINTREG(DEVICE_DEBUG_HIGH);
    GTPRINTREG(RUNIT_MMASK      );

    printk(KERN_NOTICE "\nPCI Slave Address Decoding registers\n"); 
    GTPRINTREG(PCI_0_CS_0_BANK_SIZE                 ); 
    GTPRINTREG(PCI_1_CS_0_BANK_SIZE                 ); 
    GTPRINTREG(PCI_0_CS_1_BANK_SIZE                 ); 
    GTPRINTREG(PCI_1_CS_1_BANK_SIZE                 ); 
    GTPRINTREG(PCI_0_CS_2_BANK_SIZE                 ); 
    GTPRINTREG(PCI_1_CS_2_BANK_SIZE                 ); 
    GTPRINTREG(PCI_0_CS_3_BANK_SIZE                 ); 
    GTPRINTREG(PCI_1_CS_3_BANK_SIZE                 ); 
    GTPRINTREG(PCI_0_DEVCS_0_BANK_SIZE              ); 
    GTPRINTREG(PCI_1_DEVCS_0_BANK_SIZE              ); 
    GTPRINTREG(PCI_0_DEVCS_1_BANK_SIZE              ); 
    GTPRINTREG(PCI_1_DEVCS_1_BANK_SIZE              ); 
    GTPRINTREG(PCI_0_DEVCS_2_BANK_SIZE              ); 
    GTPRINTREG(PCI_1_DEVCS_2_BANK_SIZE              ); 
    GTPRINTREG(PCI_0_DEVCS_3_BANK_SIZE              ); 
    GTPRINTREG(PCI_1_DEVCS_3_BANK_SIZE              ); 
    GTPRINTREG(PCI_0_DEVCS_BOOT_BANK_SIZE           ); 
    GTPRINTREG(PCI_1_DEVCS_BOOT_BANK_SIZE           );
    GTPRINTREG(PCI_0_P2P_MEM0_BAR_SIZE              );
    GTPRINTREG(PCI_1_P2P_MEM0_BAR_SIZE              );
    GTPRINTREG(PCI_0_P2P_MEM1_BAR_SIZE              );
    GTPRINTREG(PCI_1_P2P_MEM1_BAR_SIZE              );
    GTPRINTREG(PCI_0_P2P_I_O_BAR_SIZE               );
    GTPRINTREG(PCI_1_P2P_I_O_BAR_SIZE               );
    GTPRINTREG(PCI_0_CPU_BAR_SIZE                   );
    GTPRINTREG(PCI_1_CPU_BAR_SIZE                   );
    GTPRINTREG(PCI_0_INTERNAL_SRAM_BAR_SIZE         );
    GTPRINTREG(PCI_1_INTERNAL_SRAM_BAR_SIZE         );
    GTPRINTREG(PCI_0_EXPANSION_ROM_BAR_SIZE         );
    GTPRINTREG(PCI_1_EXPANSION_ROM_BAR_SIZE         );
    GTPRINTREG(PCI_0_BASE_ADDR_REG_ENABLE           );
    GTPRINTREG(PCI_1_BASE_ADDR_REG_ENABLE           );
    GTPRINTREG(PCI_0_CS_0_BASE_ADDR_REMAP			);
    GTPRINTREG(PCI_1_CS_0_BASE_ADDR_REMAP			);
    GTPRINTREG(PCI_0_CS_1_BASE_ADDR_REMAP			);
    GTPRINTREG(PCI_1_CS_1_BASE_ADDR_REMAP			);
    GTPRINTREG(PCI_0_CS_2_BASE_ADDR_REMAP			);
    GTPRINTREG(PCI_1_CS_2_BASE_ADDR_REMAP			);
    GTPRINTREG(PCI_0_CS_3_BASE_ADDR_REMAP			);
    GTPRINTREG(PCI_1_CS_3_BASE_ADDR_REMAP			);
    GTPRINTREG(PCI_0_CS_0_BASE_HIGH_ADDR_REMAP		);
    GTPRINTREG(PCI_1_CS_0_BASE_HIGH_ADDR_REMAP		);
    GTPRINTREG(PCI_0_CS_1_BASE_HIGH_ADDR_REMAP		);
    GTPRINTREG(PCI_1_CS_1_BASE_HIGH_ADDR_REMAP		);
    GTPRINTREG(PCI_0_CS_2_BASE_HIGH_ADDR_REMAP		);
    GTPRINTREG(PCI_1_CS_2_BASE_HIGH_ADDR_REMAP		);
    GTPRINTREG(PCI_0_CS_3_BASE_HIGH_ADDR_REMAP		);
    GTPRINTREG(PCI_1_CS_3_BASE_HIGH_ADDR_REMAP		);
    GTPRINTREG(PCI_0_DEVCS_0_BASE_ADDR_REMAP		);
    GTPRINTREG(PCI_1_DEVCS_0_BASE_ADDR_REMAP		);
    GTPRINTREG(PCI_0_DEVCS_1_BASE_ADDR_REMAP		);
    GTPRINTREG(PCI_1_DEVCS_1_BASE_ADDR_REMAP		);
    GTPRINTREG(PCI_0_DEVCS_2_BASE_ADDR_REMAP		);
    GTPRINTREG(PCI_1_DEVCS_2_BASE_ADDR_REMAP		);
    GTPRINTREG(PCI_0_DEVCS_3_BASE_ADDR_REMAP        );
    GTPRINTREG(PCI_1_DEVCS_3_BASE_ADDR_REMAP        );
    GTPRINTREG(PCI_0_DEVCS_BOOTCS_BASE_ADDR_REMAP   );
    GTPRINTREG(PCI_1_DEVCS_BOOTCS_BASE_ADDR_REMAP   );
    GTPRINTREG(PCI_0_P2P_MEM0_BASE_ADDR_REMAP_LOW   );
    GTPRINTREG(PCI_1_P2P_MEM0_BASE_ADDR_REMAP_LOW   );
    GTPRINTREG(PCI_0_P2P_MEM0_BASE_ADDR_REMAP_HIGH  );
    GTPRINTREG(PCI_1_P2P_MEM0_BASE_ADDR_REMAP_HIGH  );
    GTPRINTREG(PCI_0_P2P_MEM1_BASE_ADDR_REMAP_LOW   );
    GTPRINTREG(PCI_1_P2P_MEM1_BASE_ADDR_REMAP_LOW   );
    GTPRINTREG(PCI_0_P2P_MEM1_BASE_ADDR_REMAP_HIGH  );
    GTPRINTREG(PCI_1_P2P_MEM1_BASE_ADDR_REMAP_HIGH  );
    GTPRINTREG(PCI_0_P2P_I_O_BASE_ADDR_REMAP        );
    GTPRINTREG(PCI_1_P2P_I_O_BASE_ADDR_REMAP        );
    GTPRINTREG(PCI_0_CPU_BASE_ADDR_REMAP_LOW        );
    GTPRINTREG(PCI_1_CPU_BASE_ADDR_REMAP_LOW        );
    GTPRINTREG(PCI_0_CPU_BASE_ADDR_REMAP_HIGH       );
    GTPRINTREG(PCI_1_CPU_BASE_ADDR_REMAP_HIGH       );
    GTPRINTREG(PCI_0_INTEGRATED_SRAM_BASE_ADDR_REMAP);
    GTPRINTREG(PCI_1_INTEGRATED_SRAM_BASE_ADDR_REMAP);
    GTPRINTREG(PCI_0_EXPANSION_ROM_BASE_ADDR_REMAP  );
    GTPRINTREG(PCI_1_EXPANSION_ROM_BASE_ADDR_REMAP  );
    GTPRINTREG(PCI_0_ADDR_DECODE_CONTROL            );
    GTPRINTREG(PCI_1_ADDR_DECODE_CONTROL            );
    GTPRINTREG(PCI_0_HEADERS_RETARGET_CONTROL       );
    GTPRINTREG(PCI_1_HEADERS_RETARGET_CONTROL       );
    GTPRINTREG(PCI_0_HEADERS_RETARGET_BASE          );
    GTPRINTREG(PCI_1_HEADERS_RETARGET_BASE          );
    GTPRINTREG(PCI_0_HEADERS_RETARGET_HIGH          );
    GTPRINTREG(PCI_1_HEADERS_RETARGET_HIGH          );

    printk(KERN_NOTICE "\nPCI Control Register Map\n"); 
    GTPRINTREG(PCI_0_DLL_STATUS_AND_COMMAND   );
    GTPRINTREG(PCI_1_DLL_STATUS_AND_COMMAND   );
    GTPRINTREG(PCI_0_MPP_PADS_DRIVE_CONTROL   );
    GTPRINTREG(PCI_1_MPP_PADS_DRIVE_CONTROL   );
    GTPRINTREG(PCI_0_COMMAND				);
    GTPRINTREG(PCI_1_COMMAND				);
    GTPRINTREG(PCI_0_MODE                     );
    GTPRINTREG(PCI_1_MODE                     );
    GTPRINTREG(PCI_0_RETRY	        		);
    GTPRINTREG(PCI_1_RETRY				       );
    GTPRINTREG(PCI_0_READ_BUFFER_DISCARD_TIMER);
    GTPRINTREG(PCI_1_READ_BUFFER_DISCARD_TIMER);
    GTPRINTREG(PCI_0_MSI_TRIGGER_TIMER        );
    GTPRINTREG(PCI_1_MSI_TRIGGER_TIMER        );
    GTPRINTREG(PCI_0_ARBITER_CONTROL          );
    GTPRINTREG(PCI_1_ARBITER_CONTROL          );
    GTPRINTREG(PCI_0_CROSS_BAR_CONTROL_LOW    );
    GTPRINTREG(PCI_1_CROSS_BAR_CONTROL_LOW    );
    GTPRINTREG(PCI_0_CROSS_BAR_CONTROL_HIGH   );
    GTPRINTREG(PCI_1_CROSS_BAR_CONTROL_HIGH   );
    GTPRINTREG(PCI_0_CROSS_BAR_TIMEOUT        );
    GTPRINTREG(PCI_1_CROSS_BAR_TIMEOUT        );
    GTPRINTREG(PCI_0_SYNC_BARRIER_TRIGGER_REG );
    GTPRINTREG(PCI_1_SYNC_BARRIER_TRIGGER_REG );
    GTPRINTREG(PCI_0_SYNC_BARRIER_VIRTUAL_REG );
    GTPRINTREG(PCI_1_SYNC_BARRIER_VIRTUAL_REG );
    GTPRINTREG(PCI_0_P2P_CONFIG               );
    GTPRINTREG(PCI_1_P2P_CONFIG               );
    GTPRINTREG(PCI_0_ACCESS_CONTROL_BASE_0_LOW );
    GTPRINTREG(PCI_0_ACCESS_CONTROL_BASE_0_HIGH);
    GTPRINTREG(PCI_0_ACCESS_CONTROL_SIZE_0     );
    GTPRINTREG(PCI_0_ACCESS_CONTROL_BASE_1_LOW );
    GTPRINTREG(PCI_0_ACCESS_CONTROL_BASE_1_HIGH);
    GTPRINTREG(PCI_0_ACCESS_CONTROL_SIZE_1     );
    GTPRINTREG(PCI_0_ACCESS_CONTROL_BASE_2_LOW );
    GTPRINTREG(PCI_0_ACCESS_CONTROL_BASE_2_HIGH);
    GTPRINTREG(PCI_0_ACCESS_CONTROL_SIZE_2     );
    GTPRINTREG(PCI_0_ACCESS_CONTROL_BASE_3_LOW );
    GTPRINTREG(PCI_0_ACCESS_CONTROL_BASE_3_HIGH);
    GTPRINTREG(PCI_0_ACCESS_CONTROL_SIZE_3     );
    GTPRINTREG(PCI_0_ACCESS_CONTROL_BASE_4_LOW );
    GTPRINTREG(PCI_0_ACCESS_CONTROL_BASE_4_HIGH);
    GTPRINTREG(PCI_0_ACCESS_CONTROL_SIZE_4     );
    GTPRINTREG(PCI_0_ACCESS_CONTROL_BASE_5_LOW );
    GTPRINTREG(PCI_0_ACCESS_CONTROL_BASE_5_HIGH);
    GTPRINTREG(PCI_0_ACCESS_CONTROL_SIZE_5     );
    GTPRINTREG(PCI_1_ACCESS_CONTROL_BASE_0_LOW );
    GTPRINTREG(PCI_1_ACCESS_CONTROL_BASE_0_HIGH);
    GTPRINTREG(PCI_1_ACCESS_CONTROL_SIZE_0     );
    GTPRINTREG(PCI_1_ACCESS_CONTROL_BASE_1_LOW );
    GTPRINTREG(PCI_1_ACCESS_CONTROL_BASE_1_HIGH);
    GTPRINTREG(PCI_1_ACCESS_CONTROL_SIZE_1     );
    GTPRINTREG(PCI_1_ACCESS_CONTROL_BASE_2_LOW );
    GTPRINTREG(PCI_1_ACCESS_CONTROL_BASE_2_HIGH);
    GTPRINTREG(PCI_1_ACCESS_CONTROL_SIZE_2     );
    GTPRINTREG(PCI_1_ACCESS_CONTROL_BASE_3_LOW );
    GTPRINTREG(PCI_1_ACCESS_CONTROL_BASE_3_HIGH);
    GTPRINTREG(PCI_1_ACCESS_CONTROL_SIZE_3     );
    GTPRINTREG(PCI_1_ACCESS_CONTROL_BASE_4_LOW );
    GTPRINTREG(PCI_1_ACCESS_CONTROL_BASE_4_HIGH);
    GTPRINTREG(PCI_1_ACCESS_CONTROL_SIZE_4     );
    GTPRINTREG(PCI_1_ACCESS_CONTROL_BASE_5_LOW );
    GTPRINTREG(PCI_1_ACCESS_CONTROL_BASE_5_HIGH);
    GTPRINTREG(PCI_1_ACCESS_CONTROL_SIZE_5     );

    printk(KERN_NOTICE "\nPCI Configuration Access Registers\n"); 
    GTPRINTREG(PCI_0_CONFIG_ADDR 					  );  
    GTPRINTREG(PCI_0_CONFIG_DATA_VIRTUAL_REG          ); 
    GTPRINTREG(PCI_1_CONFIG_ADDR 					   ); 
    GTPRINTREG(PCI_1_CONFIG_DATA_VIRTUAL_REG          ); 
    GTPRINTREG(PCI_0_INTERRUPT_ACKNOWLEDGE_VIRTUAL_REG); 
    GTPRINTREG(PCI_1_INTERRUPT_ACKNOWLEDGE_VIRTUAL_REG); 

    printk(KERN_NOTICE "\nPCI Error Report Registers\n"); 
    GTPRINTREG(PCI_0_SERR_MASK		);
    GTPRINTREG(PCI_1_SERR_MASK		);
    GTPRINTREG(PCI_0_ERROR_ADDR_LOW );
    GTPRINTREG(PCI_1_ERROR_ADDR_LOW );
    GTPRINTREG(PCI_0_ERROR_ADDR_HIGH);
    GTPRINTREG(PCI_1_ERROR_ADDR_HIGH);
    GTPRINTREG(PCI_0_ERROR_ATTRIBUTE);
    GTPRINTREG(PCI_1_ERROR_ATTRIBUTE);
    GTPRINTREG(PCI_0_ERROR_COMMAND  );
    GTPRINTREG(PCI_1_ERROR_COMMAND  );
    GTPRINTREG(PCI_0_ERROR_CAUSE    );
    GTPRINTREG(PCI_1_ERROR_CAUSE    );
    GTPRINTREG(PCI_0_ERROR_MASK     );
    GTPRINTREG(PCI_1_ERROR_MASK  );

    printk(KERN_NOTICE "\nPCI Debug Registers \n"); 
    GTPRINTREG(PCI_0_MMASK  ); 
    GTPRINTREG(PCI_1_MMASK);

    printk(KERN_NOTICE "\nMessaging Unit Registers (I20)\n"); 
    GTPRINTREG(I2O_INBOUND_MESSAGE_REG0_PCI_0_SIDE			  );
    GTPRINTREG(I2O_INBOUND_MESSAGE_REG1_PCI_0_SIDE  		  );
    GTPRINTREG(I2O_OUTBOUND_MESSAGE_REG0_PCI_0_SIDE 		  );
    GTPRINTREG(I2O_OUTBOUND_MESSAGE_REG1_PCI_0_SIDE  		  );
    GTPRINTREG(I2O_INBOUND_DOORBELL_REG_PCI_0_SIDE  		  );
    GTPRINTREG(I2O_INBOUND_INTERRUPT_CAUSE_REG_PCI_0_SIDE    );
    GTPRINTREG(I2O_INBOUND_INTERRUPT_MASK_REG_PCI_0_SIDE	  );
    GTPRINTREG(I2O_OUTBOUND_DOORBELL_REG_PCI_0_SIDE 		  );
    GTPRINTREG(I2O_OUTBOUND_INTERRUPT_CAUSE_REG_PCI_0_SIDE   );
    GTPRINTREG(I2O_OUTBOUND_INTERRUPT_MASK_REG_PCI_0_SIDE    );
    GTPRINTREG(I2O_INBOUND_QUEUE_PORT_VIRTUAL_REG_PCI_0_SIDE );
    GTPRINTREG(I2O_OUTBOUND_QUEUE_PORT_VIRTUAL_REG_PCI_0_SIDE);
    GTPRINTREG(I2O_QUEUE_CONTROL_REG_PCI_0_SIDE 			);
    GTPRINTREG(I2O_QUEUE_BASE_ADDR_REG_PCI_0_SIDE 			  );
    GTPRINTREG(I2O_INBOUND_FREE_HEAD_POINTER_REG_PCI_0_SIDE  );
    GTPRINTREG(I2O_INBOUND_FREE_TAIL_POINTER_REG_PCI_0_SIDE  );
    GTPRINTREG(I2O_INBOUND_POST_HEAD_POINTER_REG_PCI_0_SIDE  );
    GTPRINTREG(I2O_INBOUND_POST_TAIL_POINTER_REG_PCI_0_SIDE  );
    GTPRINTREG(I2O_OUTBOUND_FREE_HEAD_POINTER_REG_PCI_0_SIDE );
    GTPRINTREG(I2O_OUTBOUND_FREE_TAIL_POINTER_REG_PCI_0_SIDE );
    GTPRINTREG(I2O_OUTBOUND_POST_HEAD_POINTER_REG_PCI_0_SIDE );
    GTPRINTREG(I2O_OUTBOUND_POST_TAIL_POINTER_REG_PCI_0_SIDE );
    GTPRINTREG(I2O_INBOUND_MESSAGE_REG0_PCI_1_SIDE			 ); 
    GTPRINTREG(I2O_INBOUND_MESSAGE_REG1_PCI_1_SIDE  		 ); 
    GTPRINTREG(I2O_OUTBOUND_MESSAGE_REG0_PCI_1_SIDE 		 ); 
    GTPRINTREG(I2O_OUTBOUND_MESSAGE_REG1_PCI_1_SIDE  		 ); 
    GTPRINTREG(I2O_INBOUND_DOORBELL_REG_PCI_1_SIDE  		 ); 
    GTPRINTREG(I2O_INBOUND_INTERRUPT_CAUSE_REG_PCI_1_SIDE    );
    GTPRINTREG(I2O_INBOUND_INTERRUPT_MASK_REG_PCI_1_SIDE	 ); 
    GTPRINTREG(I2O_OUTBOUND_DOORBELL_REG_PCI_1_SIDE 		 ); 
    GTPRINTREG(I2O_OUTBOUND_INTERRUPT_CAUSE_REG_PCI_1_SIDE   );
    GTPRINTREG(I2O_OUTBOUND_INTERRUPT_MASK_REG_PCI_1_SIDE    );
    GTPRINTREG(I2O_INBOUND_QUEUE_PORT_VIRTUAL_REG_PCI_1_SIDE );
    GTPRINTREG(I2O_OUTBOUND_QUEUE_PORT_VIRTUAL_REG_PCI_1_SIDE);
    GTPRINTREG(I2O_QUEUE_CONTROL_REG_PCI_1_SIDE 			);
    GTPRINTREG(I2O_QUEUE_BASE_ADDR_REG_PCI_1_SIDE 			 ); 
    GTPRINTREG(I2O_INBOUND_FREE_HEAD_POINTER_REG_PCI_1_SIDE  );
    GTPRINTREG(I2O_INBOUND_FREE_TAIL_POINTER_REG_PCI_1_SIDE  );
    GTPRINTREG(I2O_INBOUND_POST_HEAD_POINTER_REG_PCI_1_SIDE  );
    GTPRINTREG(I2O_INBOUND_POST_TAIL_POINTER_REG_PCI_1_SIDE  );
    GTPRINTREG(I2O_OUTBOUND_FREE_HEAD_POINTER_REG_PCI_1_SIDE );
    GTPRINTREG(I2O_OUTBOUND_FREE_TAIL_POINTER_REG_PCI_1_SIDE );
    GTPRINTREG(I2O_OUTBOUND_POST_HEAD_POINTER_REG_PCI_1_SIDE );
    GTPRINTREG(I2O_OUTBOUND_POST_TAIL_POINTER_REG_PCI_1_SIDE );
    GTPRINTREG(I2O_INBOUND_MESSAGE_REG0_CPU0_SIDE			 ); 
    GTPRINTREG(I2O_INBOUND_MESSAGE_REG1_CPU0_SIDE  		  );
    GTPRINTREG(I2O_OUTBOUND_MESSAGE_REG0_CPU0_SIDE 		  );
    GTPRINTREG(I2O_OUTBOUND_MESSAGE_REG1_CPU0_SIDE  		 ); 
    GTPRINTREG(I2O_INBOUND_DOORBELL_REG_CPU0_SIDE  		                 );
    GTPRINTREG(I2O_INBOUND_INTERRUPT_CAUSE_REG_CPU0_SIDE  	  );
    GTPRINTREG(I2O_INBOUND_INTERRUPT_MASK_REG_CPU0_SIDE	  );
    GTPRINTREG(I2O_OUTBOUND_DOORBELL_REG_CPU0_SIDE 		  );
    GTPRINTREG(I2O_OUTBOUND_INTERRUPT_CAUSE_REG_CPU0_SIDE    );
    GTPRINTREG(I2O_OUTBOUND_INTERRUPT_MASK_REG_CPU0_SIDE     );
    GTPRINTREG(I2O_INBOUND_QUEUE_PORT_VIRTUAL_REG_CPU0_SIDE  );
    GTPRINTREG(I2O_OUTBOUND_QUEUE_PORT_VIRTUAL_REG_CPU0_SIDE );
    GTPRINTREG(I2O_QUEUE_CONTROL_REG_CPU0_SIDE 			  );
    GTPRINTREG(I2O_QUEUE_BASE_ADDR_REG_CPU0_SIDE 		      );
    GTPRINTREG(I2O_INBOUND_FREE_HEAD_POINTER_REG_CPU0_SIDE   );
    GTPRINTREG(I2O_INBOUND_FREE_TAIL_POINTER_REG_CPU0_SIDE   );
    GTPRINTREG(I2O_INBOUND_POST_HEAD_POINTER_REG_CPU0_SIDE   );
    GTPRINTREG(I2O_INBOUND_POST_TAIL_POINTER_REG_CPU0_SIDE   );
    GTPRINTREG(I2O_OUTBOUND_FREE_HEAD_POINTER_REG_CPU0_SIDE  );
    GTPRINTREG(I2O_OUTBOUND_FREE_TAIL_POINTER_REG_CPU0_SIDE  );
    GTPRINTREG(I2O_OUTBOUND_POST_HEAD_POINTER_REG_CPU0_SIDE  );
    GTPRINTREG(I2O_OUTBOUND_POST_TAIL_POINTER_REG_CPU0_SIDE  );
    GTPRINTREG(I2O_INBOUND_MESSAGE_REG0_CPU1_SIDE			 ); 
    GTPRINTREG(I2O_INBOUND_MESSAGE_REG1_CPU1_SIDE  		  );
    GTPRINTREG(I2O_OUTBOUND_MESSAGE_REG0_CPU1_SIDE 		  );
    GTPRINTREG(I2O_OUTBOUND_MESSAGE_REG1_CPU1_SIDE  		 ); 
    GTPRINTREG(I2O_INBOUND_DOORBELL_REG_CPU1_SIDE  		  );
    GTPRINTREG(I2O_INBOUND_INTERRUPT_CAUSE_REG_CPU1_SIDE  	 ); 
    GTPRINTREG(I2O_INBOUND_INTERRUPT_MASK_REG_CPU1_SIDE	  );
    GTPRINTREG(I2O_OUTBOUND_DOORBELL_REG_CPU1_SIDE 		  );
    GTPRINTREG(I2O_OUTBOUND_INTERRUPT_CAUSE_REG_CPU1_SIDE    );
    GTPRINTREG(I2O_OUTBOUND_INTERRUPT_MASK_REG_CPU1_SIDE     );
    GTPRINTREG(I2O_INBOUND_QUEUE_PORT_VIRTUAL_REG_CPU1_SIDE  );
    GTPRINTREG(I2O_OUTBOUND_QUEUE_PORT_VIRTUAL_REG_CPU1_SIDE );
    GTPRINTREG(I2O_QUEUE_CONTROL_REG_CPU1_SIDE 			  );
    GTPRINTREG(I2O_QUEUE_BASE_ADDR_REG_CPU1_SIDE 		     ); 
    GTPRINTREG(I2O_INBOUND_FREE_HEAD_POINTER_REG_CPU1_SIDE   );
    GTPRINTREG(I2O_INBOUND_FREE_TAIL_POINTER_REG_CPU1_SIDE   );
    GTPRINTREG(I2O_INBOUND_POST_HEAD_POINTER_REG_CPU1_SIDE   );
    GTPRINTREG(I2O_INBOUND_POST_TAIL_POINTER_REG_CPU1_SIDE   );
    GTPRINTREG(I2O_OUTBOUND_FREE_HEAD_POINTER_REG_CPU1_SIDE  );
    GTPRINTREG(I2O_OUTBOUND_FREE_TAIL_POINTER_REG_CPU1_SIDE  );
    GTPRINTREG(I2O_OUTBOUND_POST_HEAD_POINTER_REG_CPU1_SIDE  );
    GTPRINTREG(I2O_OUTBOUND_POST_TAIL_POINTER_REG_CPU1_SIDE  );

    printk(KERN_NOTICE "\nEthernet Unit Registers\n"); 
    GTPRINTREG(ETH_PHY_ADDR_REG                );
    GTPRINTREG(ETH_SMI_REG                     );
    GTPRINTREG(ETH_UNIT_DEFAULT_ADDR_REG       );
    GTPRINTREG(ETH_UNIT_DEFAULTID_REG          );
    GTPRINTREG(ETH_UNIT_INTERRUPT_CAUSE_REG    );
    GTPRINTREG(ETH_UNIT_INTERRUPT_MASK_REG     );
    GTPRINTREG(ETH_UNIT_INTERNAL_USE_REG       );
    GTPRINTREG(ETH_UNIT_ERROR_ADDR_REG         );
    GTPRINTREG(ETH_BAR_0                       );
    GTPRINTREG(ETH_BAR_1                       );
    GTPRINTREG(ETH_BAR_2                       );
    GTPRINTREG(ETH_BAR_3                       );
    GTPRINTREG(ETH_BAR_4                       );
    GTPRINTREG(ETH_BAR_5                       );
    GTPRINTREG(ETH_SIZE_REG_0                  );
    GTPRINTREG(ETH_SIZE_REG_1                  );
    GTPRINTREG(ETH_SIZE_REG_2                  );
    GTPRINTREG(ETH_SIZE_REG_3                  );
    GTPRINTREG(ETH_SIZE_REG_4                  );
    GTPRINTREG(ETH_SIZE_REG_5                  );
    GTPRINTREG(ETH_HEADERS_RETARGET_BASE_REG   );
    GTPRINTREG(ETH_HEADERS_RETARGET_CONTROL_REG);
    GTPRINTREG(ETH_HIGH_ADDR_REMAP_REG_0       );
    GTPRINTREG(ETH_HIGH_ADDR_REMAP_REG_1       );
    GTPRINTREG(ETH_HIGH_ADDR_REMAP_REG_2       );
    GTPRINTREG(ETH_HIGH_ADDR_REMAP_REG_3       );
    GTPRINTREG(ETH_BASE_ADDR_ENABLE_REG        );

    printk(KERN_NOTICE "\nCUNIT  Registers\n"); 
    GTPRINTREG(CUNIT_BASE_ADDR_REG0              );
    GTPRINTREG(CUNIT_BASE_ADDR_REG1              );
    GTPRINTREG(CUNIT_BASE_ADDR_REG2              );
    GTPRINTREG(CUNIT_BASE_ADDR_REG3              );
    GTPRINTREG(CUNIT_SIZE0                       );
    GTPRINTREG(CUNIT_SIZE1                       );
    GTPRINTREG(CUNIT_SIZE2                       );
    GTPRINTREG(CUNIT_SIZE3                       );
    GTPRINTREG(CUNIT_HIGH_ADDR_REMAP_REG0        );
    GTPRINTREG(CUNIT_HIGH_ADDR_REMAP_REG1        );
    GTPRINTREG(CUNIT_BASE_ADDR_ENABLE_REG        );
    GTPRINTREG(MPSC0_ACCESS_PROTECTION_REG       );
    GTPRINTREG(MPSC1_ACCESS_PROTECTION_REG       );
    GTPRINTREG(CUNIT_INTERNAL_SPACE_BASE_ADDR_REG);

    printk(KERN_NOTICE "\nCUNIT Error Report Registers\n"); 
    GTPRINTREG(CUNIT_INTERRUPT_CAUSE_REG);
    GTPRINTREG(CUNIT_INTERRUPT_MASK_REG );
    GTPRINTREG(CUNIT_ERROR_ADDR         );

    printk(KERN_NOTICE "\nCunit Control Registers\n"); 
    GTPRINTREG(CUNIT_ARBITER_CONTROL_REG );
    GTPRINTREG(CUNIT_CONFIG_REG          );
    GTPRINTREG(CUNIT_CRROSBAR_TIMEOUT_REG);

    printk(KERN_NOTICE "\nCunit Debug Registers\n"); 
    GTPRINTREG(CUNIT_DEBUG_LOW );
    GTPRINTREG(CUNIT_DEBUG_HIGH);
    GTPRINTREG(CUNIT_MMASK     );

    printk(KERN_NOTICE "\nMPSCs Clocks Routing Registers\n"); 
    GTPRINTREG(MPSC_ROUTING_REG         );
    GTPRINTREG(MPSC_RX_CLOCK_ROUTING_REG);
    GTPRINTREG(MPSC_TX_CLOCK_ROUTING_REG);

    printk(KERN_NOTICE "\nDMA Channel Control\n"); 
    GTPRINTREG(DMA_CHANNEL0_CONTROL 	);
    GTPRINTREG(DMA_CHANNEL0_CONTROL_HIGH);
    GTPRINTREG(DMA_CHANNEL1_CONTROL 	);
    GTPRINTREG(DMA_CHANNEL1_CONTROL_HIGH);
    GTPRINTREG(DMA_CHANNEL2_CONTROL 	);
    GTPRINTREG(DMA_CHANNEL2_CONTROL_HIGH);
    GTPRINTREG(DMA_CHANNEL3_CONTROL 	);
    GTPRINTREG(DMA_CHANNEL3_CONTROL_HIGH);

    printk(KERN_NOTICE "\nIDMA Registers\n"); 
    GTPRINTREG(DMA_CHANNEL0_BYTE_COUNT                );
    GTPRINTREG(DMA_CHANNEL1_BYTE_COUNT                );
    GTPRINTREG(DMA_CHANNEL2_BYTE_COUNT                );
    GTPRINTREG(DMA_CHANNEL3_BYTE_COUNT                );
    GTPRINTREG(DMA_CHANNEL0_SOURCE_ADDR               );
    GTPRINTREG(DMA_CHANNEL1_SOURCE_ADDR               );
    GTPRINTREG(DMA_CHANNEL2_SOURCE_ADDR               );
    GTPRINTREG(DMA_CHANNEL3_SOURCE_ADDR               );
    GTPRINTREG(DMA_CHANNEL0_DESTINATION_ADDR          );
    GTPRINTREG(DMA_CHANNEL1_DESTINATION_ADDR          );
    GTPRINTREG(DMA_CHANNEL2_DESTINATION_ADDR          );
    GTPRINTREG(DMA_CHANNEL3_DESTINATION_ADDR          );
    GTPRINTREG(DMA_CHANNEL0_NEXT_DESCRIPTOR_POINTER   );
    GTPRINTREG(DMA_CHANNEL1_NEXT_DESCRIPTOR_POINTER   );
    GTPRINTREG(DMA_CHANNEL2_NEXT_DESCRIPTOR_POINTER   );
    GTPRINTREG(DMA_CHANNEL3_NEXT_DESCRIPTOR_POINTER   );
    GTPRINTREG(DMA_CHANNEL0_CURRENT_DESCRIPTOR_POINTER);
    GTPRINTREG(DMA_CHANNEL1_CURRENT_DESCRIPTOR_POINTER);
    GTPRINTREG(DMA_CHANNEL2_CURRENT_DESCRIPTOR_POINTER);
    GTPRINTREG(DMA_CHANNEL3_CURRENT_DESCRIPTOR_POINTER);

    printk(KERN_NOTICE "\nIDMA Address Decoding Base Address Registers\n"); 
    GTPRINTREG(DMA_BASE_ADDR_REG0);
    GTPRINTREG(DMA_BASE_ADDR_REG1);
    GTPRINTREG(DMA_BASE_ADDR_REG2);
    GTPRINTREG(DMA_BASE_ADDR_REG3);
    GTPRINTREG(DMA_BASE_ADDR_REG4);
    GTPRINTREG(DMA_BASE_ADDR_REG5);
    GTPRINTREG(DMA_BASE_ADDR_REG6);
    GTPRINTREG(DMA_BASE_ADDR_REG7);

    printk(KERN_NOTICE "\nIDMA Address Decoding Size Address Register\n"); 
    GTPRINTREG(DMA_SIZE_REG0);
    GTPRINTREG(DMA_SIZE_REG1);
    GTPRINTREG(DMA_SIZE_REG2);
    GTPRINTREG(DMA_SIZE_REG3);
    GTPRINTREG(DMA_SIZE_REG4);
    GTPRINTREG(DMA_SIZE_REG5);
    GTPRINTREG(DMA_SIZE_REG6);
    GTPRINTREG(DMA_SIZE_REG7);

    printk(KERN_NOTICE "\nIDMA Address Decoding High Address Remap and Access Protection Registers\n"); 
    GTPRINTREG(DMA_HIGH_ADDR_REMAP_REG0          );
    GTPRINTREG(DMA_HIGH_ADDR_REMAP_REG1          );
    GTPRINTREG(DMA_HIGH_ADDR_REMAP_REG2          );
    GTPRINTREG(DMA_HIGH_ADDR_REMAP_REG3          );
    GTPRINTREG(DMA_BASE_ADDR_ENABLE_REG          );
    GTPRINTREG(DMA_CHANNEL0_ACCESS_PROTECTION_REG);
    GTPRINTREG(DMA_CHANNEL1_ACCESS_PROTECTION_REG);
    GTPRINTREG(DMA_CHANNEL2_ACCESS_PROTECTION_REG);
    GTPRINTREG(DMA_CHANNEL3_ACCESS_PROTECTION_REG);
    GTPRINTREG(DMA_ARBITER_CONTROL               );
    GTPRINTREG(DMA_CROSS_BAR_TIMEOUT             );

    printk(KERN_NOTICE "\nIDMA Headers Retarget Registers\n"); 
    GTPRINTREG(DMA_HEADERS_RETARGET_CONTROL);
    GTPRINTREG(DMA_HEADERS_RETARGET_BASE   );

    printk(KERN_NOTICE "\nIDMA Interrupt Register\n"); 
    GTPRINTREG(DMA_INTERRUPT_CAUSE_REG );
    GTPRINTREG(DMA_INTERRUPT_CAUSE_MASK);
    GTPRINTREG(DMA_ERROR_ADDR          );
    GTPRINTREG(DMA_ERROR_SELECT        );

    printk(KERN_NOTICE "\nIDMA Debug Register ( for internal use )\n"); 
    GTPRINTREG(DMA_DEBUG_LOW );
    GTPRINTREG(DMA_DEBUG_HIGH);
    GTPRINTREG(DMA_SPARE     );

    printk(KERN_NOTICE "\nTimer_Counter\n"); 
    GTPRINTREG(TIMER_COUNTER0					);
    GTPRINTREG(TIMER_COUNTER1					);
    GTPRINTREG(TIMER_COUNTER2					);
    GTPRINTREG(TIMER_COUNTER3					);
    GTPRINTREG(TIMER_COUNTER_0_3_CONTROL		);
    GTPRINTREG(TIMER_COUNTER_0_3_INTERRUPT_CAUSE);
    GTPRINTREG(TIMER_COUNTER_0_3_INTERRUPT_MASK );

    printk(KERN_NOTICE "\nWatchdog registers\n"); 
    GTPRINTREG(WATCHDOG_CONFIG_REG);
    GTPRINTREG(WATCHDOG_VALUE_REG );

    printk(KERN_NOTICE "\nI2C Registers\n"); 
    GTPRINTREG(I2C_SLAVE_ADDR         );
    GTPRINTREG(I2C_EXTENDED_SLAVE_ADDR);
    GTPRINTREG(I2C_DATA               );
    GTPRINTREG(I2C_CONTROL            );
    GTPRINTREG(I2C_STATUS_BAUDE_RATE  );
    GTPRINTREG(I2C_SOFT_RESET         );

    printk(KERN_NOTICE "\nGPP Interface Registers\n"); 
    GTPRINTREG(GPP_IO_CONTROL     );
    GTPRINTREG(GPP_LEVEL_CONTROL  );
    GTPRINTREG(GPP_VALUE          );
    GTPRINTREG(GPP_INTERRUPT_CAUSE);
    GTPRINTREG(GPP_INTERRUPT_MASK0);
    GTPRINTREG(GPP_INTERRUPT_MASK1);
    GTPRINTREG(GPP_VALUE_SET      );
    GTPRINTREG(GPP_VALUE_CLEAR    );

    printk(KERN_NOTICE "\nInterrupts\n"); 
    GTPRINTREG(MAIN_INTERRUPT_CAUSE_LOW   );
    GTPRINTREG(MAIN_INTERRUPT_CAUSE_HIGH  );
    GTPRINTREG(CPU_INTERRUPT0_MASK_LOW    );
    GTPRINTREG(CPU_INTERRUPT0_MASK_HIGH   );
    GTPRINTREG(CPU_INTERRUPT0_SELECT_CAUSE);
    GTPRINTREG(CPU_INTERRUPT1_MASK_LOW    );
    GTPRINTREG(CPU_INTERRUPT1_MASK_HIGH   );
    GTPRINTREG(CPU_INTERRUPT1_SELECT_CAUSE);
    GTPRINTREG(INTERRUPT0_MASK_0_LOW      );
    GTPRINTREG(INTERRUPT0_MASK_0_HIGH     );
    GTPRINTREG(INTERRUPT0_SELECT_CAUSE    );
    GTPRINTREG(INTERRUPT1_MASK_0_LOW      );
    GTPRINTREG(INTERRUPT1_MASK_0_HIGH     );
    GTPRINTREG(INTERRUPT1_SELECT_CAUSE    );

    printk(KERN_NOTICE "\nMPP Interface Registers\n"); 
    GTPRINTREG(MPP_CONTROL0);
    GTPRINTREG(MPP_CONTROL1);
    GTPRINTREG(MPP_CONTROL2);
    GTPRINTREG(MPP_CONTROL3);

    printk(KERN_NOTICE "\nSerial Initialization registers \n"); 
    GTPRINTREG(SERIAL_INIT_LAST_DATA);
    GTPRINTREG(SERIAL_INIT_CONTROL  );
    GTPRINTREG(SERIAL_INIT_STATUS   );

    printk(KERN_NOTICE "\nPCI Configuration, Function 0, Registers\n"); 

    GTPRINTCONFIGREG(PCI_DEVICE_AND_VENDOR_ID 							);
    GTPRINTCONFIGREG(PCI_STATUS_AND_COMMAND								);
    GTPRINTCONFIGREG(PCI_CLASS_CODE_AND_REVISION_ID				        );
    GTPRINTCONFIGREG(PCI_BIST_HEADER_TYPE_LATENCY_TIMER_CACHE_LINE 		);
    GTPRINTCONFIGREG(PCI_SCS_0_BASE_ADDR_LOW   					        );
    GTPRINTCONFIGREG(PCI_SCS_0_BASE_ADDR_HIGH   					    );  
    GTPRINTCONFIGREG(PCI_SCS_1_BASE_ADDR_LOW  					        );
    GTPRINTCONFIGREG(PCI_SCS_1_BASE_ADDR_HIGH  					        );
    GTPRINTCONFIGREG(PCI_INTERNAL_REG_MEM_MAPPED_BASE_ADDR_LOW       	);
    GTPRINTCONFIGREG(PCI_INTERNAL_REG_MEM_MAPPED_BASE_ADDR_HIGH      	);
    GTPRINTCONFIGREG(PCI_SUBSYSTEM_ID_AND_SUBSYSTEM_VENDOR_ID			);
    GTPRINTCONFIGREG(PCI_EXPANSION_ROM_BASE_ADDR_REG			        );    
    GTPRINTCONFIGREG(PCI_CAPABILTY_LIST_POINTER                         ); 
    GTPRINTCONFIGREG(PCI_INTERRUPT_PIN_AND_LINE 						);    

    GTPRINTCONFIGREG(PCI_POWER_MANAGEMENT_CAPABILITY                  );   
    GTPRINTCONFIGREG(PCI_POWER_MANAGEMENT_STATUS_AND_CONTROL          );   
    GTPRINTCONFIGREG(PCI_VPD_ADDR                                     );   
    GTPRINTCONFIGREG(PCI_VPD_DATA                                     );   
    GTPRINTCONFIGREG(PCI_MSI_MESSAGE_CONTROL                          );   
    GTPRINTCONFIGREG(PCI_MSI_MESSAGE_ADDR                             );   
    GTPRINTCONFIGREG(PCI_MSI_MESSAGE_UPPER_ADDR                       );   
    GTPRINTCONFIGREG(PCI_MSI_MESSAGE_DATA                             );   
    GTPRINTCONFIGREG(PCI_X_COMMAND                                    );   
    GTPRINTCONFIGREG(PCI_X_STAT                                       );   
    GTPRINTCONFIGREG(PCI_COMPACT_PCI_HOT_SWAP                         );   

    printk(KERN_NOTICE "\nPCI Configuration, Function 1, Registers\n"); 

    GTPRINTCONFIGREG(PCI_SCS_2_BASE_ADDR_LOW   						   );  
    GTPRINTCONFIGREG(PCI_SCS_2_BASE_ADDR_HIGH						   );  
    GTPRINTCONFIGREG(PCI_SCS_3_BASE_ADDR_LOW 						   ); 	
    GTPRINTCONFIGREG(PCI_SCS_3_BASE_ADDR_HIGH					       );  
    GTPRINTCONFIGREG(PCI_INTERNAL_SRAM_BASE_ADDR_LOW          	       );  
    GTPRINTCONFIGREG(PCI_INTERNAL_SRAM_BASE_ADDR_HIGH         	       );  

    printk(KERN_NOTICE "\nPCI Configuration, Function 2, Registers\n"); 

    GTPRINTCONFIGREG(PCI_DEVCS_0_BASE_ADDR_LOW	    			        );  
    GTPRINTCONFIGREG(PCI_DEVCS_0_BASE_ADDR_HIGH 					    );  
    GTPRINTCONFIGREG(PCI_DEVCS_1_BASE_ADDR_LOW 					        ); 	
    GTPRINTCONFIGREG(PCI_DEVCS_1_BASE_ADDR_HIGH      			        );  
    GTPRINTCONFIGREG(PCI_DEVCS_2_BASE_ADDR_LOW 					        );  
    GTPRINTCONFIGREG(PCI_DEVCS_2_BASE_ADDR_HIGH      			        );  

    printk(KERN_NOTICE "\nPCI Configuration, Function 3, Registers\n"); 

    GTPRINTCONFIGREG(PCI_DEVCS_3_BASE_ADDR_LOW	    			        );  
    GTPRINTCONFIGREG(PCI_DEVCS_3_BASE_ADDR_HIGH 					    );  
    GTPRINTCONFIGREG(PCI_BOOT_CS_BASE_ADDR_LOW					        ); 	
    GTPRINTCONFIGREG(PCI_BOOT_CS_BASE_ADDR_HIGH      			        );  
    GTPRINTCONFIGREG(PCI_CPU_BASE_ADDR_LOW 						        );  
    GTPRINTCONFIGREG(PCI_CPU_BASE_ADDR_HIGH      			            );  

    printk(KERN_NOTICE "\nPCI Configuration, Function 4, Registers\n"); 

    GTPRINTCONFIGREG(PCI_P2P_MEM0_BASE_ADDR_LOW  					    );  
    GTPRINTCONFIGREG(PCI_P2P_MEM0_BASE_ADDR_HIGH 				        );  
    GTPRINTCONFIGREG(PCI_P2P_MEM1_BASE_ADDR_LOW   				        ); 	
    GTPRINTCONFIGREG(PCI_P2P_MEM1_BASE_ADDR_HIGH 				        );  
    GTPRINTCONFIGREG(PCI_P2P_I_O_BASE_ADDR                 	            );  
    GTPRINTCONFIGREG(PCI_INTERNAL_REGS_I_O_MAPPED_BASE_ADDR             );  

}


#define GTPRINTMEM(baseAddrEn, gtregwindow, gtregbase, gtregsize, desc, sizeCalc) { \
if (!(baseAddrEn & gtregwindow)) \
{ \
unsigned int base = GTREGREAD(gtregbase); \
unsigned int size = GTREGREAD(gtregsize); \
base = base << 16; \
switch (sizeCalc) \
{ \
case 0: \
/* 16 bit shift */ \
size = ((size << 16) | 0xffff) + 1; \
break; \
case 1: \
/* 12 bits all 1's */ \
size = (size | 0xfff) + 1; \
break; \
case 2: \
/* size is the size */ \
size = size; \
break; \
} \
printk(KERN_NOTICE "%-20.19s %-16s 0x%08x  0x%08x  0x%08x\n", #gtregwindow, desc, base, base + size - 1, size); \
} \
else \
{ \
printk(KERN_NOTICE "%-20.19s %-16s Disabled\n", #gtregwindow, desc); \
} \
}

/*******************************************************************************
* gtCpuMemMapShow - show PCI Memory Map
*
*/  
void gtCpuMemMapShow(void)
{
    unsigned int baseAddrEn;

    printk(KERN_NOTICE "gtCpuMemMapShow called\n"); 

    baseAddrEn = GTREGREAD(BASE_ADDR_ENABLE); 

    printk(KERN_NOTICE "VMIVME-7050 Memory Map, CPU Point of View\n"); 
    printk(KERN_NOTICE "Window               Description      Start Addr  End Addr    Size\n"); 

    GTPRINTMEM(baseAddrEn, CS_0_WINDOW, CS_0_BASE_ADDR, CS_0_SIZE, "Ram", 0);
    GTPRINTMEM(baseAddrEn, CS_1_WINDOW, CS_1_BASE_ADDR, CS_1_SIZE, "N/A", 0);
    GTPRINTMEM(baseAddrEn, CS_2_WINDOW, CS_2_BASE_ADDR, CS_2_SIZE, "N/A", 0);
    GTPRINTMEM(baseAddrEn, CS_3_WINDOW, CS_3_BASE_ADDR, CS_3_SIZE, "N/A", 0);

    GTPRINTMEM(baseAddrEn, DEVCS_0_WINDOW, DEV_CS0_BASE_ADDR, DEV_CS0_SIZE, "Flash", 0);
    GTPRINTMEM(baseAddrEn, DEVCS_1_WINDOW, DEV_CS1_BASE_ADDR, DEV_CS1_SIZE, "NVRAM", 0);
    GTPRINTMEM(baseAddrEn, DEVCS_2_WINDOW, DEV_CS2_BASE_ADDR, DEV_CS2_SIZE, "N/A", 0);
    GTPRINTMEM(baseAddrEn, DEVCS_3_WINDOW, DEV_CS3_BASE_ADDR, DEV_CS3_SIZE, "N/A", 0);

    GTPRINTMEM(baseAddrEn, BOOT_CS_WINDOW, BOOTCS_BASE_ADDR, BOOTCS_SIZE, "Boot CS",0);

    GTPRINTMEM(baseAddrEn, PCI_0_IO_WINDOW,   PCI_0_IO_BASE_ADDR,      PCI_0_IO_SIZE,      "PCI0 I/O Access", 0);
    GTPRINTMEM(baseAddrEn, PCI_0_MEM0_WINDOW, PCI_0_MEMORY0_BASE_ADDR, PCI_0_MEMORY0_SIZE, "PCI0 Mem0 Access", 0);
    GTPRINTMEM(baseAddrEn, PCI_0_MEM1_WINDOW, PCI_0_MEMORY1_BASE_ADDR, PCI_0_MEMORY1_SIZE, "PCI0 Mem1 Access", 0);
    GTPRINTMEM(baseAddrEn, PCI_0_MEM2_WINDOW, PCI_0_MEMORY2_BASE_ADDR, PCI_0_MEMORY2_SIZE, "PCI0 Mem2 Access", 0);
    GTPRINTMEM(baseAddrEn, PCI_0_MEM3_WINDOW, PCI_0_MEMORY3_BASE_ADDR, PCI_0_MEMORY3_SIZE, "PCI0 Mem3 Access", 0);

    GTPRINTMEM(baseAddrEn, PCI_1_IO_WINDOW,   PCI_1_IO_BASE_ADDR,      PCI_1_IO_SIZE,      "PCI1 I/O Access", 0);
    GTPRINTMEM(baseAddrEn, PCI_1_MEM0_WINDOW, PCI_1_MEMORY0_BASE_ADDR, PCI_1_MEMORY0_SIZE, "PCI1 Mem0 Access", 0);
    GTPRINTMEM(baseAddrEn, PCI_1_MEM1_WINDOW, PCI_1_MEMORY1_BASE_ADDR, PCI_1_MEMORY1_SIZE, "PCI1 Mem1 Access", 0);
    GTPRINTMEM(baseAddrEn, PCI_1_MEM2_WINDOW, PCI_1_MEMORY2_BASE_ADDR, PCI_1_MEMORY2_SIZE, "PCI1 Mem2 Access", 0);
    GTPRINTMEM(baseAddrEn, PCI_1_MEM3_WINDOW, PCI_1_MEMORY3_BASE_ADDR, PCI_1_MEMORY3_SIZE, "PCI1 Mem3 Access", 0);

    GTPRINTMEM(baseAddrEn, INTEGRATED_SRAM_WINDOW, INTEGRATED_SRAM_BASE_ADDR, PCI_0_INTERNAL_SRAM_BAR_SIZE, "SRAM", 1);
    GTPRINTMEM(baseAddrEn, INTERNAL_SPACE_WINDOW,  INTERNAL_SPACE_BASE_ADDR,  0x10000, "MV64030 Regs", 2);
}


#define GTPRINTPCI(baseAddrEn, gtregwindow, gtregbase, gtregsize, desc, busNo) { \
if (!(baseAddrEn & (BIT0 << gtregwindow))) \
{ \
unsigned int base; \
unsigned int size = GTREGREAD(gtregsize); \
if (busNo == 0) \
{ \
base = gtPci0ReadConfigReg(gtregbase,PCI_SELF); \
} \
else \
{ \
base = gtPci1ReadConfigReg(gtregbase,PCI_SELF); \
} \
base = base & 0xfffffff0; \
size = (size & 0xfffff000) + 0xfff + 1; \
printk(KERN_NOTICE "%-20.19s %-16s 0x%08x  0x%08x  0x%08x\n", #gtregwindow, desc, base, base + size - 1, size); \
} \
else \
{ \
printk(KERN_NOTICE "%-20.19s %-16s Disabled\n", #gtregwindow, desc); \
} \
}


/*******************************************************************************
* gtPciMemMapShow - show PCI Memory Map
*
*/  
void gtPciMemMapShow(void)
    {
    unsigned int baseAddrEn;
    printk(KERN_NOTICE "gtPciMemMapShow called\n"); 

    baseAddrEn = GTREGREAD(PCI_0_BASE_ADDR_REG_ENABLE); 

    printk(KERN_NOTICE "VMIVME-7050 Memory Map, PCI0 Point of View\n"); 
    printk(KERN_NOTICE "Window               Description      Start Addr  End Addr    Size\n"); 

    GTPRINTPCI(baseAddrEn, PCI_CS0_BAR, PCI_SCS_0_BASE_ADDR_LOW, PCI_0_CS_0_BANK_SIZE, "Ram",0);
    GTPRINTPCI(baseAddrEn, PCI_CS1_BAR, PCI_SCS_1_BASE_ADDR_LOW, PCI_0_CS_1_BANK_SIZE, "N/A",0);
    GTPRINTPCI(baseAddrEn, PCI_CS2_BAR, PCI_SCS_2_BASE_ADDR_LOW, PCI_0_CS_2_BANK_SIZE, "N/A",0);
    GTPRINTPCI(baseAddrEn, PCI_CS3_BAR, PCI_SCS_3_BASE_ADDR_LOW, PCI_0_CS_3_BANK_SIZE, "N/A",0);

    GTPRINTPCI(baseAddrEn, PCI_DEV_CS0_BAR, PCI_DEVCS_0_BASE_ADDR_LOW, PCI_0_DEVCS_0_BANK_SIZE, "Flash",0);
    GTPRINTPCI(baseAddrEn, PCI_DEV_CS1_BAR, PCI_DEVCS_1_BASE_ADDR_LOW, PCI_0_DEVCS_1_BANK_SIZE, "NVRAM",0);
    GTPRINTPCI(baseAddrEn, PCI_DEV_CS2_BAR, PCI_DEVCS_2_BASE_ADDR_LOW, PCI_0_DEVCS_2_BANK_SIZE, "N/A",0);
    GTPRINTPCI(baseAddrEn, PCI_DEV_CS3_BAR, PCI_DEVCS_3_BASE_ADDR_LOW, PCI_0_DEVCS_3_BANK_SIZE, "N/A",0);

    GTPRINTPCI(baseAddrEn, PCI_BOOT_CS_BAR, PCI_BOOT_CS_BASE_ADDR_LOW, PCI_0_DEVCS_BOOT_BANK_SIZE, "Boot CS",0);

    GTPRINTPCI(baseAddrEn, PCI_MEM_MAPPED_INTERNAL_REG_BAR, PCI_INTERNAL_REG_MEM_MAPPED_BASE_ADDR_LOW, BOOTCS_SIZE,                  "MV64030 Regs",0);
    GTPRINTPCI(baseAddrEn, PCI_IO_MAPPED_INTERNAL_REG_BAR,  PCI_INTERNAL_REGS_I_O_MAPPED_BASE_ADDR,    BOOTCS_SIZE,                  "MV64030 Regs",0);
    GTPRINTPCI(baseAddrEn, PCI_P2P_MEM0_BAR,                PCI_P2P_MEM0_BASE_ADDR_LOW,                PCI_0_P2P_MEM0_BAR_SIZE,      "P2P Mem0",0);
    GTPRINTPCI(baseAddrEn, PCI_P2P_MEM1_BAR,                PCI_P2P_MEM1_BASE_ADDR_LOW,                PCI_0_P2P_MEM1_BAR_SIZE,      "P2P Mem1",0);
    GTPRINTPCI(baseAddrEn, PCI_P2P_IO_BAR,                  PCI_P2P_I_O_BASE_ADDR,                     PCI_0_P2P_I_O_BAR_SIZE,       "P2P I/O",0);
    GTPRINTPCI(baseAddrEn, PCI_CPU_BAR,                     PCI_CPU_BASE_ADDR_LOW,                     PCI_0_CPU_BAR_SIZE,           "CPU",0);
    GTPRINTPCI(baseAddrEn, PCI_INTERNAL_SRAM_BAR,           PCI_INTERNAL_SRAM_BASE_ADDR_LOW,           PCI_0_INTERNAL_SRAM_BAR_SIZE, "SRAM",0);

    baseAddrEn = GTREGREAD(PCI_1_BASE_ADDR_REG_ENABLE); 

    printk(KERN_NOTICE "\n");

    printk(KERN_NOTICE "VMIVME-7050 Memory Map, PCI1 Point of View\n"); 
    printk(KERN_NOTICE "Window               Description      Start Addr  End Addr    Size\n"); 

    GTPRINTPCI(baseAddrEn, PCI_CS0_BAR, PCI_SCS_0_BASE_ADDR_LOW, PCI_1_CS_0_BANK_SIZE, "Ram",1);
    GTPRINTPCI(baseAddrEn, PCI_CS1_BAR, PCI_SCS_1_BASE_ADDR_LOW, PCI_1_CS_1_BANK_SIZE, "N/A",1);
    GTPRINTPCI(baseAddrEn, PCI_CS2_BAR, PCI_SCS_2_BASE_ADDR_LOW, PCI_1_CS_2_BANK_SIZE, "N/A",1);
    GTPRINTPCI(baseAddrEn, PCI_CS3_BAR, PCI_SCS_3_BASE_ADDR_LOW, PCI_1_CS_3_BANK_SIZE, "N/A",1);

    GTPRINTPCI(baseAddrEn, PCI_DEV_CS0_BAR, PCI_DEVCS_0_BASE_ADDR_LOW, PCI_1_DEVCS_0_BANK_SIZE, "Flash",1);
    GTPRINTPCI(baseAddrEn, PCI_DEV_CS1_BAR, PCI_DEVCS_1_BASE_ADDR_LOW, PCI_1_DEVCS_1_BANK_SIZE, "NVRAM",1);
    GTPRINTPCI(baseAddrEn, PCI_DEV_CS2_BAR, PCI_DEVCS_2_BASE_ADDR_LOW, PCI_1_DEVCS_2_BANK_SIZE, "N/A",1);
    GTPRINTPCI(baseAddrEn, PCI_DEV_CS3_BAR, PCI_DEVCS_3_BASE_ADDR_LOW, PCI_1_DEVCS_3_BANK_SIZE, "N/A",1);

    GTPRINTPCI(baseAddrEn, PCI_BOOT_CS_BAR, PCI_BOOT_CS_BASE_ADDR_LOW, PCI_1_DEVCS_BOOT_BANK_SIZE, "Boot CS",1);

    GTPRINTPCI(baseAddrEn, PCI_MEM_MAPPED_INTERNAL_REG_BAR, PCI_INTERNAL_REG_MEM_MAPPED_BASE_ADDR_LOW, BOOTCS_SIZE,                  "MV64030 Regs",1);
    GTPRINTPCI(baseAddrEn, PCI_IO_MAPPED_INTERNAL_REG_BAR,  PCI_INTERNAL_REGS_I_O_MAPPED_BASE_ADDR,    BOOTCS_SIZE,                  "MV64030 Regs",1);
    GTPRINTPCI(baseAddrEn, PCI_P2P_MEM0_BAR,                PCI_P2P_MEM0_BASE_ADDR_LOW,                PCI_1_P2P_MEM0_BAR_SIZE,      "P2P Mem0",1);
    GTPRINTPCI(baseAddrEn, PCI_P2P_MEM1_BAR,                PCI_P2P_MEM1_BASE_ADDR_LOW,                PCI_1_P2P_MEM1_BAR_SIZE,      "P2P Mem1",1);
    GTPRINTPCI(baseAddrEn, PCI_P2P_IO_BAR,                  PCI_P2P_I_O_BASE_ADDR,                     PCI_1_P2P_I_O_BAR_SIZE,       "P2P I/O",1);
    GTPRINTPCI(baseAddrEn, PCI_CPU_BAR,                     PCI_CPU_BASE_ADDR_LOW,                     PCI_1_CPU_BAR_SIZE,           "CPU",1);
    GTPRINTPCI(baseAddrEn, PCI_INTERNAL_SRAM_BAR,           PCI_INTERNAL_SRAM_BASE_ADDR_LOW,           PCI_1_INTERNAL_SRAM_BAR_SIZE, "SRAM",1);
    }

#endif
/*******************************************************************************
* pciGetConfigReg - Configuration read routine.
*
* DESCRIPTION:
*		This function interfaces the BSP configuration read routine (located
*		in pci.c module) to WRS PCI read routine as defined in pciConfigLib.c
*		module.
*
* INPUT:
*
*    	bus    - PCI bus segment number (Not PCI bus number !!).
*    	dev    - PCI device number.
*    	funcNo - PCI function number.
*    	offset - offset into the configuration space.
*    	*pData - 32 bit data read from the offset.
*
* OUTPUT:
*		Calling BSP PCI0 read configuration routine.
*
* RETURN:
*		OK.
*
*******************************************************************************/
void pciGetConfigReg(int bus, int dev,int funcNo, int offset,unsigned int *pData)
{
    if (bus == 0)
        {
        /* PCI bus 0 - Universe/IDE */
        *pData = gtPci0ReadConfigReg(((funcNo << 8) | offset), dev);
        }
    else if (bus == 1)
        {
        /* PCI bus 1 - PMC sites */
        *pData = gtPci1ReadConfigReg(((funcNo << 8) | offset), dev);
        }
}

int pciGetConfigRegBySize(int bus, int dev,int funcNo, int offset, int size/*, void *pData*/)
{
    int retval;

    pciGetConfigReg (bus, dev, funcNo, (offset & (int)(~3)), &retval);

    switch (size)
        {
        case 1:
            retval >>= ((offset & 0x03) * 8);
            retval &= 0xff;
            return (unsigned char) retval;
            break;

        case 2:
            retval >>= ((offset & (int)(0x02) ) * 8);
            retval &= 0xffff;
            return (uint16_t) retval;
            break;

        case 4:
            return (uint32_t) retval;
            break;

        default:
            printk(KERN_NOTICE "pciGetConfigRegBySize: Invalid size %d\n", size);
            break;
        }

    return -1;
}

#define PCIPRINTCONFIGREG(bus, dev, func, reg, size) printk(KERN_NOTICE "%-42.41s (0x%04x) = 0x%08x\n",#reg, reg, pciGetConfigRegBySize(bus, dev, func, reg, size)) 

void pciHeaderShow(int bus, int dev, int func)
    {
        printk(KERN_NOTICE "\nPCI bus %d device %d function %d\n", bus, dev, func);
	PCIPRINTCONFIGREG(bus, dev, func, PCI_CFG_VENDOR_ID		,2);
	PCIPRINTCONFIGREG(bus, dev, func, PCI_CFG_DEVICE_ID		,2);
	PCIPRINTCONFIGREG(bus, dev, func, PCI_CFG_COMMAND		,2);
	PCIPRINTCONFIGREG(bus, dev, func, PCI_CFG_STATUS			,2);
	PCIPRINTCONFIGREG(bus, dev, func, PCI_CFG_REVISION		,1);
	PCIPRINTCONFIGREG(bus, dev, func, PCI_CFG_PROGRAMMING_IF	,1);
	PCIPRINTCONFIGREG(bus, dev, func, PCI_CFG_SUBCLASS		,1);
	PCIPRINTCONFIGREG(bus, dev, func, PCI_CFG_CLASS			,1);
	PCIPRINTCONFIGREG(bus, dev, func, PCI_CFG_CACHE_LINE_SIZE,2);
	PCIPRINTCONFIGREG(bus, dev, func, PCI_CFG_LATENCY_TIMER	,1);
	PCIPRINTCONFIGREG(bus, dev, func, PCI_CFG_HEADER_TYPE	,1);
	PCIPRINTCONFIGREG(bus, dev, func, PCI_CFG_BIST			,1);
	PCIPRINTCONFIGREG(bus, dev, func, PCI_CFG_BASE_ADDRESS_0	,4);
	PCIPRINTCONFIGREG(bus, dev, func, PCI_CFG_BASE_ADDRESS_1	,4);
	PCIPRINTCONFIGREG(bus, dev, func, PCI_CFG_BASE_ADDRESS_2	,4);
	PCIPRINTCONFIGREG(bus, dev, func, PCI_CFG_BASE_ADDRESS_3	,4);
	PCIPRINTCONFIGREG(bus, dev, func, PCI_CFG_BASE_ADDRESS_4	,4);
	PCIPRINTCONFIGREG(bus, dev, func, PCI_CFG_BASE_ADDRESS_5	,4);
	PCIPRINTCONFIGREG(bus, dev, func, PCI_CFG_CIS			,4);
	PCIPRINTCONFIGREG(bus, dev, func, PCI_CFG_SUB_VENDER_ID	,2);
	PCIPRINTCONFIGREG(bus, dev, func, PCI_CFG_SUB_SYSTEM_ID	,2);
	PCIPRINTCONFIGREG(bus, dev, func, PCI_CFG_EXPANSION_ROM	,4);
	PCIPRINTCONFIGREG(bus, dev, func, PCI_CFG_DEV_INT_LINE	,1);
	PCIPRINTCONFIGREG(bus, dev, func, PCI_CFG_DEV_INT_PIN	,1);
	PCIPRINTCONFIGREG(bus, dev, func, PCI_CFG_MIN_GRANT		,1);
	PCIPRINTCONFIGREG(bus, dev, func, PCI_CFG_MAX_LATENCY	,1);
    }
void showPciBus0()
{

	/* Universe */
        printk(KERN_NOTICE "Called ShowPciBus0\n"); 
	pciHeaderShow(0,6,0);

	/* IDE */
	pciHeaderShow(0,7,0);

        printk(KERN_NOTICE "End ShowPciBus0\n"); 
}

void showPciBus1()
{

	/* pmc 1 */
	pciHeaderShow(1,1,0);

	/* pmc 2 */
	pciHeaderShow(1,2,0);
}

void diagInit()
{
#define _64K            0x00010000
#define DB64360_BRIDGE_REG_BASE 0xfe040000
#define MV64360_INTERNAL_SPACE_SIZE _64K
	
     gtInternalRegBaseAddr = (unsigned int)ioremap_nocache(DB64360_BRIDGE_REG_BASE,
					MV64360_INTERNAL_SPACE_SIZE);
     printk(KERN_NOTICE "End diagInit %x\n",gtInternalRegBaseAddr); 
}

void diagClose()
{ 
     iounmap((void *)gtInternalRegBaseAddr);
}
