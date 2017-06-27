#ifndef HAL_GPMI_H
#define HAL_GPMI_H

/*==================================================================================================
     Header Name: hal_gpmi.h
     General Description: Provide some defines for gpmi drivers.
====================================================================================================

                         Copyright: 2004-2014 FREESCALE, INC.
                   All Rights Reserved. This file contains copyrighted material.
                   Use of this file is restricted by the provisions of a
                   Freescale Software License Agreement, which has either
                   accompanied the delivery of this software in shrink wrap
                   form or been expressly executed between the parties.

Revision History:
                  Modification    Tracking
Author (core ID)      Date         Number     Description of Changes
-----------------  -----------   ----------   ------------------------------------------
Terry Xie           2/Jan/2014                Initial Implementation
-----------------  ------------  ----------   ------------------------------------------
====================================================================================================
*===================================================================================
                                        INCLUDE FILES
==================================================================================*/
#include <io.h>

/*================================================================================
                          Hash Defines And Macros
=================================================================================*/
/************************************************************/
/*            GPMI clocks                                   */
/************************************************************/
#define GPMI_DEFAULT_CLK_PERIOD     (42)
#define NAND_GPMI_TIMING0(AddSetup, DataSetup, DataHold) \
           (BF_GPMI_TIMING0_ADDRESS_SETUP(AddSetup) | \
            BF_GPMI_TIMING0_DATA_HOLD(DataHold) | \
            BF_GPMI_TIMING0_DATA_SETUP(DataSetup))

#define NORMAL_CLOCKS_SEARCH_CYCLES  (100/5)

/* timeout */
#define DEFAULT_FLASH_BUSY_TIMEOUT  (58)    //!< (10msec / 41.6ns) / 4096 = 58
#define FLASH_BUSY_TIMEOUT          10000000    //!< Busy Timeout time in nsec. (10msec)
/* Divide 4096 before being passed to gpmi_find_cycle for gpmi_timing1.device_busy_timeout update,
   since device_busy_timeout value is the number of GPMI_CLK mutiplied by 4096, and the
   gpmi_find_cycle can return faster with the pre-dividing. */
#define FLASH_BUSY_TIMEOUT_DIV_4096 ((FLASH_BUSY_TIMEOUT + 4095) / 4096)
// Calculate max search cycles based upon a 200MHz clock - 5ns.
#define BUSY_TIMEOUT_SEARCH_CYCLES  (FLASH_BUSY_TIMEOUT_DIV_4096/5)
#define WAIT_FOR_READY_TIMEOUT  30000

/************************************************************/
/*            NAND flash timings                            */
/************************************************************/

#define GPMI_DEBUG_WAIT_FOR_READY(channel)  (0x1 << (BP_GPMI_DEBUG_WAIT_FOR_READY_END + channel))
#define GPMI_DEBUG_SENSE(channel)           (0x1 << (BP_GPMI_DEBUG_DMA_SENSE + channel))
#define GPMI_DEBUG_DMAREQ(channel)          (0x1 << (BP_GPMI_DEBUG_DMAREQ + channel))
#define GPMI_DEBUG_CMD_END(channel)         (0x1 << (BP_GPMI_DEBUG_CMD_END + channel))
#define CCM_CLK_GPMI_OFFSET 0xac
#define CCM_CLKSEQ_BYPASS_OFFSET 0x90
#define CCM_CLK_GPMI CCM_BASE_ADDR+CCM_CLK_GPMI_OFFSET
#define CCM_CLKSEQ_BYPASS CCM_BASE_ADDR+CCM_CLKSEQ_BYPASS_OFFSET
#define CCM_BASE_ADDR			        0x53FD4000

/*==================================================================================================
                                 STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/
/* Data structure used to configure NAND timings */
typedef struct _NAND_Timing {
    U8 m_u8DataSetup;
    U8 m_u8DataHold;
    U8 m_u8AddressSetup;
    U8 m_u8Reserved;
// These are for application use only and not for ROM.
    U8 m_u8NandTimingState;
    U8 m_u8REA;
    U8 m_u8RLOH;
    U8 m_u8RHOH;
} NAND_Timing_t;

/* Data structure used to configure sync-mode NAND DDR NAND */
typedef struct _OnfisyncFCBData {
    U32 read_latency;
    U32 ce_delay;
    U32 preamble_delay;
    U32 postamble_delay;
    U32 cmdadd_pause;
    U32 data_pause;
    U32 busy_timeout;
} OnfisyncFCBData;

/* Data structure used to configure toggle-mode NAND DDR NAND */
typedef struct _TMNANDFCBData {
    U32 m_u32ReadLatency;
    U32 m_u32PreambleDelay;
    U32 m_u32CEDelay;
    U32 m_u32PostambleDelay;
    U32 m_u32CmdAddPause;
    U32 m_u32DataPause;
    U32 m_u32TogglemodeSpeed;   /* 0 for 33, 1 for 40 and 2 for 66MHz */
    U32 m_u32BusyTimeout;
} TMNANDFCBData;

/*=================================================================================
                                 Global FUNCTION PROTOTYPES
=================================================================================*/
void gpmi_reset(void);
void gpmi_set_timeout(U32 timeout);
void gpmi_set_nand_timing_in_cycle(U32 addr_setup_cycles,
                                   U32 data_setup_cycles, U32 data_hold_cycles, U32 busy_timeout);
void gpmi_set_nand_timing(NAND_Timing_t * nand_timing, U32 gpmi_period_ns);
void gpmi_set_nand_sync_timing(OnfisyncFCBData * sync_timing);
void gpmi_nand_sync_config(void);
void gpmi_set_nand_toggle_timing(TMNANDFCBData * toggle_timing, NAND_Timing_t * nand_timing);
void gpmi_nand_toggle_config(void);
BOOL gpmi_enable(void);
U32 gpmi_poll_debug(U32 mask, U32 match, U32 timeout);

#endif
