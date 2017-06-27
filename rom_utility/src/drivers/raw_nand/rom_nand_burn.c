/*==================================================================================================
    Copyright (C) 2006-2011  Freescale Semiconductor, Inc. All Rights Reserved
    THIS SOURCE CODE IS CONFIDENTIAL AND PROPRIETARY AND MAY NOT
    BE USED OR DISTRIBUTED WITHOUT THE WRITTEN PERMISSION OF
    Freescale Semiconductor, Inc.
*===================================================================================================*/

/*
Revision History:
                            Modification     Tracking
Author                          Date          Number     Description of Changes
-------------------------   ------------    ----------   -------------------------------------------
Terry Xie                 27/4/2014    ENGR00151908
*/

/*===================================================================================
                                        INCLUDE FILES
==================================================================================*/
#include <io.h>
#include "rom_nand_boot.h"
#include "rom_nand_burn.h"
#include "hamming.h"
#include "nand_base.h"
#include "nand_read_retry.h"
#include "hal_dma.h"
#include "hal_gpmi.h"
#include "hal_bch.h"
#include "hal_nand.h"

/* nand configuration table, maybe some kinds of nand can't be found here */
extern const rom_nand_cfg_t rom_nand_cfg_table[];

/* nand count in rom_nand_cfg_table */
extern U32 nand_cnt_in_table;

/* the fcb struct to program into nand FCB area */
FCBStruct_t fcb;

/* judge whether nand is smart nand */
static BOOL is_smart_nand = FALSE;

/* the dbbt struct to program into nand dbbt area */
static dbbt_block_t dbbt;

/* buf to store onfi nand parameter page */
static U8 parameter_page_buf[NAND_PARAMETER_PAGE_LEN];

/**
 * nand_onfi_read_parameter_page - read parameter page to get nand parameters, only for onfi nand 
 * @cs: nand chip select
 */
static BOOL nand_get_parameter_from_parameter_page(U32 cs)
{
    char *onfi_device_name;
    int i;

    memset(&parameter_page_buf, 0, NAND_PARAMETER_PAGE_LEN);

    /* read parameter page */
    hal_nand_wait_for_ready(cs);

    hal_nand_send_cmd(cs, NAND_READ_PARAMETER_PAGE_CMD);
    hal_nand_send_addr(cs, NAND_READ_PARAMETER_PAGE_ONFI_ADDR, 1);
    hal_nand_wait_for_ready(cs);

    hal_nand_read_raw_data(cs, parameter_page_buf, NAND_PARAMETER_PAGE_LEN, FALSE);
    hal_nand_wait_for_ready(cs);

    /* validation for parameter page datas */
    if (parameter_page_buf[0] != 'O' || parameter_page_buf[1] != 'N' || parameter_page_buf[2] != 'F'
        || parameter_page_buf[3] != 'I') {
        printf("parameter page signature is not ONFI!\n");
        return FALSE;
    }

    /* print nand chip name */
    onfi_device_name = (char *)(parameter_page_buf + NAND_PARAMETER_PAGE_DEVICE_NANE_POS);
    printf("nand chip device name: ");
    for (i = 0; i < ONFI_DEVICE_NANE_LENGTH; i++)
        printf("%c", onfi_device_name[i]);
    printf("\n");

    /* config nand using the parameters read from parameter page */
    fcb.data_page_sz = *((U32 *) (parameter_page_buf + NAND_PARAMETER_PAGE_DATA_BYTES_POS));
    fcb.total_page_sz =
        *((U16 *) (parameter_page_buf + NAND_PARAMETER_PAGE_SPARE_BYTES_POS)) + fcb.data_page_sz;
    fcb.pages_per_block = *((U32 *) (parameter_page_buf + NAND_PARAMETER_PAGE_PAGES_PER_BLOCK_POS));
    fcb.read_retry_en =
        1;// (*((U8 *)(parameter_page_buf + NAND_PARAMETER_PAGE_READ_RETRY_OPTIONS_POS)) == 0) ? 0 : 1;

    return TRUE;
};

/**
 * nand_get_parameter_from_table - get nand parameter configuration from rom_nand_cfg_table
 * @ids: nand chip id, used to determin which nand we are using
 */
static BOOL nand_get_parameter_from_table(U8 * ids)
{
    const U8 *ids_in_table;
    int i, idx;

    for (i = 0; i < nand_cnt_in_table; i++) {
        /* match the nand in table using id */
        ids_in_table = rom_nand_cfg_table[i].ids;
        for (idx = 0; idx < ARRAY_SIZE(ids_in_table); idx++) {
            if (ids_in_table[idx] != ids[idx])
                break;
        }
        if (idx != ARRAY_SIZE(ids_in_table))
            continue;

        /* nand matched, then fill page size in fcb struct using parameters in the nand configuration table */
        fcb.data_page_sz = rom_nand_cfg_table[i].page_size;
        fcb.total_page_sz = rom_nand_cfg_table[i].page_size + rom_nand_cfg_table[i].spare_size;
        fcb.pages_per_block = rom_nand_cfg_table[i].block_pages;

        /* nand options */
        fcb.Onfi_sync_enable = (((rom_nand_cfg_table[i].option & SYNC_MODE) == 0) ? 0 : 1);
        fcb.read_retry_en = (((rom_nand_cfg_table[i].option & READ_RETRY_ENABLE) == 0) ? 0 : 1);
		is_smart_nand = (((rom_nand_cfg_table[i].option & SMART_NAND) == 0) ? FALSE: TRUE);

        return TRUE;
    }
    return FALSE;
}

/**
 * nand_fcb_fill_page_size - fill fcb.data_page_sz, fcb.total_page_sz, fcb.pages_per_block
 * onfi nand use parameter page, others use rom_nand_cfg_table

 * @ids: nand chip id, used to determin which nand we are using
 */
BOOL nand_fcb_fill_page_size(U8 * ids)
{
    /* onfi nand get parameters from nand parameter page */
    if (ids[0] == NAND_MFR_MICRON || ids[0] == NAND_MFR_HYNIX || ids[0] == NAND_MFR_SANDISK
			|| ids[0] == NAND_MFR_SPANSION) {
        if (!nand_get_parameter_from_parameter_page(0)) {
            printf
                ("read parameter page failed! try to get page size from nand configuration table\n");
            if (!nand_get_parameter_from_table(ids)) {
                printf("get parameter from table failed!\n");
                return FALSE;
            }
        }
    } else if (ids[0] == NAND_MFR_SAMSUNG || ids[0] == NAND_MFR_TOSHIBA) {
        printf("samsung and toshiba get page size from nand configuration table directly\n");
        if (!nand_get_parameter_from_table(ids)) {
            printf("get parameter from table failed!\n");
            return FALSE;
        }
    } else {
        printf("can not fill the page size of nand whose manufacture is not supported by us\n");
        return FALSE;
    }

    return TRUE;
}

/**
 * nand_fcb_fill_ecc - fill fcb.meta_sz, fcb.block0_sz, fcb.blockn_sz, fcb.bch_type, fcb.blockn_num_per_page
 * @ids: nand chip id, used to determin which nand we are using
 */
static void nand_fcb_fill_ecc(U8 * ids)
{
    int idx;
    U32 min_ecc_needed;
    U32 ecc_level_array_samsung[8] = { 1, 2, 4, 8, 16, 24, 40, 60 };

	/* metadata size */
    fcb.meta_sz = 10;

	/* ecc block size and bch type, number of ecc blockn */
    if (fcb.data_page_sz <= 4096) {
        fcb.block0_sz = 512; 
		fcb.blockn_sz = 512;
        fcb.bch_type = BCH_GF13_SUPPORTED;
    } else {
        fcb.block0_sz = 1024;
        fcb.blockn_sz = 1024;
        fcb.bch_type = BCH_GF14_SUPPORTED;
    }
    fcb.blockn_num_per_page = (fcb.data_page_sz - fcb.block0_sz) / fcb.blockn_sz;

	/* ecc level, for toshiba smart nand, no ecc needed, for samsung, ecc level info contains in id */
    if (is_smart_nand) {
		min_ecc_needed = 0;
	} else if (ids[0] == NAND_MFR_SAMSUNG) {
        idx = (ids[4] & 0x70) >> 4;
        min_ecc_needed = ecc_level_array_samsung[idx];
	} else {
        min_ecc_needed = (fcb.total_page_sz - fcb.data_page_sz - fcb.meta_sz) * 8 /
            ((fcb.bch_type + 13) * (fcb.blockn_num_per_page + 1));
        min_ecc_needed = (min_ecc_needed > ECC_LEVEL_MAX ? ECC_LEVEL_MAX : min_ecc_needed); // ecc level can not exceed the max ecc level
        min_ecc_needed = min_ecc_needed / 2 * 2;    // BCH can only support even ecc levels
    }

    fcb.blockn_ecc_level = min_ecc_needed / 2;  // boot code multiplied 2, so we divide 2 here
    fcb.block0_ecc_level = min_ecc_needed / 2;
}

/**
 * nand_fcb_fill_bad_block - fill fcb.DBBTSearchAreaStartAddress, fcb.bbm_physical_off, fcb.bbm_spare_offset,
 		fcb.BadBlockMarkerByte, fcb.BadBlockMarkerStartBit, fcb.disbbm, fcb.DISBB_Search
 * @ids: nand chip id, used to determin which nand we are using
 */
static void nand_fcb_fill_bad_block(void)
{
    int bbm_data_pos;
    int ecc_block0_bits = fcb.block0_ecc_level * 2 * (fcb.bch_type + 13);    // we divide 2 before in function nand_fcb_fill_ecc
    int ecc_blockN_bits = fcb.blockn_ecc_level * 2 * (fcb.bch_type + 13);
    int ecc_block0_bytes = ecc_block0_bits / 8;
    int ecc_blockN_bytes = ecc_blockN_bits / 8;
    U32 total_bbm_data_bits;    // total number of data block bits before bbm phisical position

    fcb.DBBTSearchAreaStartAddress = fcb.pages_per_block * FCB_COPIES_NUM;

    fcb.bbm_physical_off = fcb.data_page_sz;
    fcb.bbm_spare_offset = 0;

    /* fcb.BadBlockMarkerByte, must avoid phisical bbm in ecc block */
    bbm_data_pos = fcb.data_page_sz - fcb.meta_sz - (ecc_block0_bytes + fcb.block0_sz)
        - (ecc_blockN_bytes + fcb.blockn_sz) * (fcb.blockn_num_per_page - 1);
    // if phisical bbm is in the penultimate ecc block, we must enlarge metadata to make bbm in the penultimate data block
    if ((bbm_data_pos >= -ecc_blockN_bytes) && (bbm_data_pos < 0)) {
        fcb.meta_sz += (ecc_blockN_bytes + bbm_data_pos + 1);
        total_bbm_data_bits = (fcb.data_page_sz * 8 - fcb.meta_sz * 8 - ecc_block0_bits - ecc_blockN_bits * (fcb.blockn_num_per_page - 2));   // bbm locates at penultimate data block
    } else
        total_bbm_data_bits = (fcb.data_page_sz * 8 - fcb.meta_sz * 8 - ecc_block0_bits - ecc_blockN_bits * (fcb.blockn_num_per_page - 1));   // bbm locates at last data block
    fcb.BadBlockMarkerByte = total_bbm_data_bits / 8;
    fcb.BadBlockMarkerStartBit = total_bbm_data_bits % 8;

    fcb.disbbm = 0;
    fcb.DISBB_Search = 0;
}

/**
 * nand_fcb_fill_firmware - fill fcb.Firmware1_startingSector, fcb.Firmware2_startingSector
 */
static void nand_fcb_fill_firmware(void)
{
    U32 firmware_max_pages;

    /* firmware image positions */
    fcb.Firmware1_startingSector =
        fcb.DBBTSearchAreaStartAddress + fcb.pages_per_block * DBBT_COPIES_NUM;
    //fcb.Firmware1_startingSector = 36 * fcb.pages_per_block;
    firmware_max_pages = (FIRMWARE_MAX_BYTES + fcb.data_page_sz - 1) / fcb.data_page_sz;
    fcb.Firmware2_startingSector = fcb.Firmware1_startingSector
        + ((firmware_max_pages + fcb.pages_per_block - 1) / fcb.pages_per_block +
           FIRMWARE_INTERLEAV_BLOCKS) * fcb.pages_per_block;
    //fcb.SectorsInFirmware2 = fcb.SectorsInFirmware1;
}

/**
 * nand_fcb_fill_timings - fill nand async, sync and toggle mode timings
 * @toggle_enable: whether enable toggle mode
 * @toggle_speed: toggle mode speed, determin the speed parameters
 * @Onfi_sync_enable: whether enable onfi sync mode
 * @onfi_sync_speed: onfi sync speed, determin the speed parameters
 *
 * return true if succeed, otherwise return false
 */
static BOOL nand_fcb_fill_timings(U32 toggle_enable, U32 toggle_speed,
                                  U32 Onfi_sync_enable, U32 onfi_sync_speed)
{
    U32 onfi_sync_timing[][7] = {
        {2, 2, 4, 2, 4, 6, 0x0500}, // sync timing mode 0, 20MHz GPMI clk
        {2, 2, 4, 2, 4, 6, 0x0500}, // sync timing mode 1, 33MHz GPMI clk
        {2, 3, 4, 2, 4, 6, 0x0500}, // sync timing mode 2, 50MHz GPMI clk
        {3, 3, 4, 2, 4, 6, 0x0500}, // sync timing mode 3, 66MHz GPMI clk
        {3, 4, 4, 2, 4, 6, 0x0500}, // sync timing mode 4, 83MHz GPMI clk
        {3, 5, 4, 2, 4, 6, 0x0500}, // sync timing mode 5, 100MHz GPMI clk
    };
    U32 toggle_timing[][7] = {
        {2, 1, 4, 3, 4, 6, 0x0500}, // toggle timing mode 0, 33MHz GPMI clk
        {2, 2, 4, 3, 4, 6, 0x0500}, // toggle timing mode 1, 40MHz GPMI clk
        {2, 2, 4, 3, 4, 6, 0x0500}, // toggle timing mode 2, 66MHz GPMI clk
    };

    /* nand m_NANDTiming (80, 60, 25, 6) */
    fcb.m_NANDTiming.m_u8DataSetup = 80;
    fcb.m_NANDTiming.m_u8DataHold = 60;
    fcb.m_NANDTiming.m_u8AddressSetup = 25;
    fcb.m_NANDTiming.m_u8Reserved = 6;

    /* toggle mode timing */
    if (toggle_enable) {
        if (toggle_speed > 2) {
            printf("toggle speed exceed the max speed of 3\n");
            return FALSE;
        } else {
            fcb.togglemodeNANDData.m_u32TogglemodeSpeed = toggle_speed;
            fcb.togglemodeNANDData.m_u32ReadLatency = toggle_timing[toggle_speed][0];
            fcb.togglemodeNANDData.m_u32PreambleDelay = toggle_timing[toggle_speed][1];
            fcb.togglemodeNANDData.m_u32CEDelay = toggle_timing[toggle_speed][2];
            fcb.togglemodeNANDData.m_u32PostambleDelay = toggle_timing[toggle_speed][3];
            fcb.togglemodeNANDData.m_u32CmdAddPause = toggle_timing[toggle_speed][4];
            fcb.togglemodeNANDData.m_u32DataPause = toggle_timing[toggle_speed][5];
            fcb.togglemodeNANDData.m_u32BusyTimeout = toggle_timing[toggle_speed][6];
        }
    }

    /* onfi sync mode timing */
    fcb.Onfi_sync_enable = Onfi_sync_enable;
    if (fcb.Onfi_sync_enable) {
        /* fcb.Onfi_sync_speed */
        if (onfi_sync_speed > 5) {
            printf("sync speed exceed the max speed of 5\n");
            return FALSE;
        } else {
            fcb.Onfi_sync_speed = onfi_sync_speed;

            /* fcb.Onfi_syncNANDData */
            fcb.Onfi_syncNANDData.read_latency = onfi_sync_timing[onfi_sync_speed][0];
            fcb.Onfi_syncNANDData.ce_delay = onfi_sync_timing[onfi_sync_speed][1];
            fcb.Onfi_syncNANDData.preamble_delay = onfi_sync_timing[onfi_sync_speed][2];
            fcb.Onfi_syncNANDData.postamble_delay = onfi_sync_timing[onfi_sync_speed][3];
            fcb.Onfi_syncNANDData.cmdadd_pause = onfi_sync_timing[onfi_sync_speed][4];
            fcb.Onfi_syncNANDData.data_pause = onfi_sync_timing[onfi_sync_speed][5];
            fcb.Onfi_syncNANDData.busy_timeout = onfi_sync_timing[onfi_sync_speed][6];
        }
    }

    return TRUE;
}

/**
  * nand_fcb_fill_read_retry_seq - fill read retry sequence into fcb
  * @ids: nand ids
  *
  * return true if succeed, otherwise return false
  */
static BOOL nand_fcb_fill_read_retry_seq(U8 ids[], U8 * page_buf)
{
    if (fcb.read_retry_en == 0) {
        printf("read retry is not enable, no need to fill retry sequence\n");
        return TRUE;
    }

    fcb.read_retry_seq_len = rr_seq_get_len(ids);
	/* 
	   as burning read retry sequence in FCB will bring a lot of customer support, 
	   so we don't want to burn read retry sequence in FCB, 
	   the sequence in ROM can be used, but ROM code will read the read retry items in FCB,
	   this will override the sequence get from ROM.
	   The method is to burn read_retry_en as 1, and read_retry_length as 0, 
	   so that read retry sequence will not be override.
	*/
    //fcb.read_retry_seq_len = 0;
    if (fcb.read_retry_seq_len == 0) {
        return TRUE;
    } else if (fcb.read_retry_seq_len >= MAX_READ_RETRY_SEQ_LEN) {
        printf("read retry sequence length %d exceeds the max length of %d\n",
               fcb.read_retry_seq_len, MAX_READ_RETRY_SEQ_LEN);
        return FALSE;
    }

    switch (ids[0]) {
    case NAND_MFR_MICRON:
        memcpy(fcb.read_retry_seq, rr_seq_micron_20nm, fcb.read_retry_seq_len);
        break;
    case NAND_MFR_SANDISK:
        if ((ids[5] & 0x07) == 0x07)
            memcpy(fcb.read_retry_seq, rr_seq_sandisk_19nm, fcb.read_retry_seq_len);
        else if ((ids[5] & 0x07) == 0x00)
            memcpy(fcb.read_retry_seq, rr_seq_sandisk_1ynm, fcb.read_retry_seq_len);
        break;
    case NAND_MFR_TOSHIBA:
        //if ((ids[5] & 0x07) == 0x07)
        //    memcpy(fcb.read_retry_seq, rr_seq_toshiba_19nm, fcb.read_retry_seq_len);
        //else if ((ids[5] & 0x07) == 0x00)
            memcpy(fcb.read_retry_seq, rr_seq_toshiba_a19nm, fcb.read_retry_seq_len);
        break;
    case NAND_MFR_HYNIX:
        if ((ids[5] & 0x07) == 0x04) {
            if (!rr_get_rrt_from_nand(ids, page_buf))
                return FALSE;
            else
                memcpy(fcb.read_retry_seq, rr_seq_hynix_20nm, fcb.read_retry_seq_len);
        } else if ((ids[5] & 0x07) == 0x03)
            memcpy(fcb.read_retry_seq, rr_seq_hynix_26nm, fcb.read_retry_seq_len);
        break;
    default:
        printf("can not find retry sequence because we donot support this vendor!\n");
        return FALSE;
    }

    return TRUE;
}

static BOOL nand_fcb_fill(U8 * page_buf)
{
    static U8 ids[6];           // must be static because we will pass it to subfuctions
    int i;

    memset((void *)&fcb, 0, sizeof(fcb));   // init all fcb members to 0 firstly

    /* read id from nand */
    memset(ids, 0, sizeof(ids));
    nand_read_id(0, 0x00, ids);
    for (i = 0; i < 6; i++) {
        printf("0x%02x ", ids[i]);
    }
    printf("\n");

    switch (ids[0]) {
    case NAND_MFR_SAMSUNG:
        printf("Samsung nand\n");
        break;
    case NAND_MFR_TOSHIBA:
        printf("Toshiba nand\n");
        break;
    case NAND_MFR_MICRON:
        printf("Micron nand\n");
        break;
    case NAND_MFR_HYNIX:
        printf("Hynix nand\n");
        break;
    case NAND_MFR_SANDISK:
        printf("SanDisk nand\n");
        break;
    case NAND_MFR_SPANSION:
        printf("Spansion nand\n");
        break;
    default:
        printf("This manufacture is not supported by us so far\n");
        return FALSE;
    }

    /* fill fingerprint and version */
    fcb.FingerPrint = 0x20424346;   // FCB
    fcb.Version = 0x01000000;

    /* fill page size members in fcb struct */
    if (!nand_fcb_fill_page_size(ids)) {
        printf("fcb fill page size failed\n");
        return FALSE;
    }

    /* fill ecc members in fcb struct */
    nand_fcb_fill_ecc(ids);

    /* fill bad block members in fcb struct */
    nand_fcb_fill_bad_block();

    /* fill firmware members in fcb struct */
    nand_fcb_fill_firmware();

    /* fill some other members in fcb struct, for normal burn, disable toggle and sync */
    nand_fcb_fill_timings(0, 0, 0, 0);

	/* fill randomizer enable */
	fcb.rand_en = 0;

    /* fill read retry sequence */
    if (!nand_fcb_fill_read_retry_seq(ids, page_buf)) {
        printf("fcb fill read retry sequence failed\n");
        return FALSE;
    }

    return TRUE;
}

static BOOL nand_fcb_fill_test_fcb(BOOL is_disbbm, int ecc_level, int toggle_speed, 
                                   int sync_speed, BOOL is_rand_en)
{
    /* fcb.disbbm */
    if (is_disbbm)
        fcb.disbbm = 1;

    /* fcb.blockn_ecc_level, fcb.block0_ecc_level */
    if ((ecc_level > 0)
		&& ((ecc_level > 2 * fcb.block0_ecc_level) || (ecc_level > 2 * fcb.blockn_ecc_level))) {
        /* set flashlayout parameter again */
        fcb.data_page_sz /= 2;
        fcb.blockn_num_per_page = (fcb.blockn_num_per_page - 1) / 2;
        fcb.block0_ecc_level = ecc_level / 2;   // must fill 31 if bch62
        fcb.blockn_ecc_level = ecc_level / 2;   // must fill 31 if bch62

        fcb.disbbm = 1;
    }

    /* fcb.togglemodeNANDData */
    if (toggle_speed >= 0) {
        if (!nand_fcb_fill_timings(1, toggle_speed, 0, 0)) {
            printf("nand fcb fill timings failed\n");
            return FALSE;
        }
    }

	/* fcb sync timing */
	if (sync_speed >= 0) {
		if (!nand_fcb_fill_timings(0, 0, 1, sync_speed)) {
			printf("nand fcb fill timings failed\n");
			return FALSE;
		}
	}

	/* fill randomizer enable */
	if (is_rand_en)
		fcb.rand_en = 1;

    return TRUE;
}

/**
 * nand_fcb_dump - dump fcb members, for debug use
 * @pfcb: fcb pointer
 */
void nand_fcb_dump(FCBStruct_t * pfcb)
{
    printf("Checksum =\t\t0x%lx\n", pfcb->Reserved);
    printf("FingerPrint =\t\t0x%lx\n", pfcb->FingerPrint);
    printf("Version =\t\t0x%lx\n", pfcb->Version);
    printf("NANDTiming.DataSetup =\t\t%ld\n", pfcb->m_NANDTiming.m_u8DataSetup);
    printf("NANDTiming.DataHold =\t\t%ld\n", pfcb->m_NANDTiming.m_u8DataHold);
    printf("NANDTiming.AddressSetup =\t\t%ld\n", pfcb->m_NANDTiming.m_u8AddressSetup);
    printf("NANDTiming.DSAMPLE_TIME =\t\t%ld\n", pfcb->m_NANDTiming.m_u8Reserved);
    printf("PageDataSize =\t\t%ld\n", pfcb->data_page_sz);
    printf("total_page_sz =\t\t%ld\n", pfcb->total_page_sz);
    printf("pages_per_block =\t\t%ld\n", pfcb->pages_per_block);
    printf("blockn_ecc_level =\t\t%ld\n", pfcb->blockn_ecc_level);
    printf("block0_sz =\t\t%ld\n", pfcb->block0_sz);
    printf("blockn_sz =\t\t%ld\n", pfcb->blockn_sz);
    printf("block0_ecc_level =\t\t%ld\n", pfcb->block0_ecc_level);
    printf("meta_sz =\t\t%ld\n", pfcb->meta_sz);
    printf("blockn_num_per_page =\t\t%ld\n", pfcb->blockn_num_per_page);
    printf("Firmware1_startingPage =\t\t%ld\n", pfcb->Firmware1_startingSector);
    printf("Firmware2_startingPage =\t\t%ld\n", pfcb->Firmware2_startingSector);
    printf("PagesInFirmware1 =\t\t%ld\n", pfcb->SectorsInFirmware1);
    printf("PagesInFirmware2 =\t\t%ld\n", pfcb->SectorsInFirmware2);
    printf("DBBTSearchAreaStartAddress =\t\t%ld\n", pfcb->DBBTSearchAreaStartAddress);
    printf("BadBlockMarkerByte =\t\t%ld\n", pfcb->BadBlockMarkerByte);
    printf("BadBlockMarkerStartBit =\t\t%ld\n", pfcb->BadBlockMarkerStartBit);
    printf("bbm_physical_off =\t\t%ld\n", pfcb->bbm_physical_off);
    printf("TogglemodeSpeed =\t\t%ld\n", pfcb->togglemodeNANDData.m_u32TogglemodeSpeed);
    printf("TogglemodeNANDData =\t\t%d, %d, %d, %d, %d, %d, 0x%x\n",
           pfcb->togglemodeNANDData.m_u32ReadLatency, pfcb->togglemodeNANDData.m_u32PreambleDelay,
           pfcb->togglemodeNANDData.m_u32CEDelay, pfcb->togglemodeNANDData.m_u32PostambleDelay,
           pfcb->togglemodeNANDData.m_u32CmdAddPause, pfcb->togglemodeNANDData.m_u32DataPause,
           pfcb->togglemodeNANDData.m_u32BusyTimeout);
    printf("bch_type =\t\t%ld\n", pfcb->bch_type);
    printf("disbbm =\t\t%ld\n", pfcb->disbbm);
    printf("bbm_spare_offset =\t\t%ld\n", pfcb->bbm_spare_offset);
    printf("Onfi_sync_enable =\t\t%ld\n", pfcb->Onfi_sync_enable);
    printf("Onfi_sync_speed =\t\t%ld\n", pfcb->Onfi_sync_speed);
    printf("Onfi_syncNANDData =\t\t%d, %d, %d, %d, %d, %d, 0x%x\n",
           pfcb->Onfi_syncNANDData.read_latency, pfcb->Onfi_syncNANDData.ce_delay,
           pfcb->Onfi_syncNANDData.preamble_delay, pfcb->Onfi_syncNANDData.postamble_delay,
           pfcb->Onfi_syncNANDData.cmdadd_pause, pfcb->Onfi_syncNANDData.data_pause,
           pfcb->Onfi_syncNANDData.busy_timeout);
    printf("DISBB_Search =\t\t%ld\n", pfcb->DISBB_Search);
    printf("rand_en =\t\t%ld\n", pfcb->rand_en);
    printf("read_retry_en =\t\t%ld\n", pfcb->read_retry_en);
    printf("read_retry_seq_len =\t\t%ld\n", pfcb->read_retry_seq_len);
}

/**
 * write fcb struct into nand page, we have two methods: NAND_FCB_HAMING_CHECK and ecc
 * @page_buf: used when write nand page
 */
static BOOL nand_fcb_write_page(U8 * page_buf)
{
    int i;
	U32 size;
    U32 page_size = nand.info.data_page_sz;
    nand.info.pages_per_block = fcb.pages_per_block;    // nand.info.pages_per_block uesd in nand_erase_block

	/* burning size depends on whether read retry must be burned */
	if (GET(READ_RETRY, nand.bootcfg->option) == BURN_WITH_READ_RETRY)
    	size = sizeof(fcb);
	else
    	size = 280;

	/* burn fcb */
    for (i = 0; i < FCB_COPIES_NUM; i++) {
        /* erase block for nand write page */
        nand_erase_block(0, i);

        /* write fcb struct to pages in diffrent block */
        memset(page_buf, 0, page_size);
		if (GET(FCB_CHECK, nand.bootcfg->option) == FCB_CHECK_HAMMING)
        	encode_hamming_code_13_8((U8 *) & fcb, size, page_buf, page_size);
		else
        	memcpy(page_buf, (U8 *) & fcb, size);

        nand_write_page(0, i * fcb.pages_per_block, page_buf);
        memset(page_buf, 0, page_size);
        if (!nand_read_page(0, i * fcb.pages_per_block, page_buf))
            return FALSE;       // only for test
    }
    return TRUE;
}

/**
 * write fcb struct into nand page, we have two methods: NAND_FCB_HAMING_CHECK and ecc
 * @page_buf: used when write nand page
 */
static BOOL nand_fcb_write_page_test(U8 * page_buf, BOOL is_fcb1_error)
{
    int i;
	U32 size;
	U32 finger_print_save;
    U32 page_size = nand.info.data_page_sz;
    nand.info.pages_per_block = fcb.pages_per_block;    // nand.info.pages_per_block uesd in nand_erase_block

	/* burning size depends on whether read retry must be burned */
	if (GET(READ_RETRY, nand.bootcfg->option) == BURN_WITH_READ_RETRY)
    	size = sizeof(fcb);
	else
    	size = 280;

	/* burn fcb */
    for (i = 0; i < FCB_COPIES_NUM; i++) {
		/* add error to fcb for testing */
		if((i == 0) && is_fcb1_error) {
			finger_print_save = fcb.FingerPrint;
			fcb.FingerPrint = 0;
		}

        /* erase block for nand write page */
        nand_erase_block(0, i);

        /* write fcb struct to pages in diffrent block */
        memset(page_buf, 0, page_size);
		if (GET(FCB_CHECK, nand.bootcfg->option) == FCB_CHECK_HAMMING)
        	encode_hamming_code_13_8((U8 *) & fcb, size, page_buf, page_size);
		else
        	memcpy(page_buf, (U8 *) & fcb, size);

        nand_write_page(0, i * fcb.pages_per_block, page_buf);
        memset(page_buf, 0, page_size);
        if (!nand_read_page(0, i * fcb.pages_per_block, page_buf))
            return FALSE;       // only for test

		/* restore fcb after adding errors */
		fcb.FingerPrint = finger_print_save;
    }
    return TRUE;
}

/**
 * rom_nand_fcb_burn - burn fcb
 * @page_buf: used to write nand page
 */
BOOL rom_nand_fcb_burn(U8 * page_buf)
{
    /* fill fcb automatically */
    if (!nand_fcb_fill(page_buf)) {
        printf("nand fcb fill failed\n");
        return FALSE;
    }

    /* dump the information of fcb */
    nand_fcb_dump(&fcb);

    /* use the struct fcb we filled to burn into the nand */
    if (!nand_fcb_write_page(page_buf)) {
        printf("nand fcb write page failed\n");
        return FALSE;
    }

    return TRUE;
}

/**
 * rom_nand_fcb_test_burn - burn fcb using some test parameters by user
 * @page_buf: used to write nand page
 */
BOOL rom_nand_fcb_test_burn(U8 * page_buf, BOOL is_disbbm, int ecc_level, int toggle_speed,
							BOOL is_fcb1_error, int sync_speed, BOOL is_rand_en)
{
    /* fill fcb automatically first */
    if (!nand_fcb_fill(page_buf)) {
        printf("nand fcb fill failed\n");
        return FALSE;
    }

    /* fill fcb parameter by user test define */
    if (!nand_fcb_fill_test_fcb(is_disbbm, ecc_level, toggle_speed, sync_speed, is_rand_en)) {
        printf("nand fcb fill test fcb failed\n");
        return FALSE;
    }

    /* dump the information of fcb */
    nand_fcb_dump(&fcb);

    /* use the struct fcb we filled to burn into the nand */
    if (!nand_fcb_write_page_test(page_buf, is_fcb1_error)) {
        printf("nand fcb write page failed\n");
        return FALSE;
    }

    return TRUE;
}

/**
 * rom_nand_dbbt_burn - burn dbbt
 * @page_buf: used to write nand page
 */
BOOL rom_nand_dbbt_burn(U8 * page_buf)
{
    int i;
    U32 dbbt_start_block = fcb.DBBTSearchAreaStartAddress / fcb.pages_per_block;
    U32 page_addr;
    U32 page_size = fcb.data_page_sz;

    /* search bad block marker and fill dbbt entry */
    memset((U8 *) (&dbbt), 0, sizeof(dbbt));
    for (i = dbbt_start_block; i < ROM_DBBT_MAX_BLOCK_NUM; i++) {   // we needn't to check fcb blocks
        if (is_block_bad_by_bbm(0, i)) {
            printf("bad block found: block %d\n", i);
            dbbt.entry.bad_block_number[dbbt.entry.NumOfEntries] = i;
            dbbt.entry.NumOfEntries++;
//            nand_erase_block(0, i);
        }
    }

    /* fill dbbt head */
    dbbt.head.FingerPrint = 0x54424244; // in ascii "DBBT"
    dbbt.head.Version = 0x01000000;
    dbbt.head.DBBTNumOfPages =
        (dbbt.entry.NumOfEntries * 4 + fcb.data_page_sz - 1) / fcb.data_page_sz;

    /* burn dbbt and its copies */
    for (i = 0; i < DBBT_COPIES_NUM; i++) {
        /* erase block for nand write page */
        nand_erase_block(0, i + dbbt_start_block);

        /* write dbbt head */
        memset(page_buf, 0, page_size);
        memcpy(page_buf, (U8 *) (&(dbbt.head)), sizeof(dbbt.head));
        page_addr = (i + dbbt_start_block) * fcb.pages_per_block;   // page 0 for dbbt head
        nand_write_page(0, page_addr, page_buf);
        memset(page_buf, 0, page_size);
        if (!nand_read_page(0, page_addr, page_buf))
            return FALSE;       // only for test

        /* write dbbt entry */
        if (dbbt.entry.NumOfEntries > 0) {
            memset(page_buf, 0, page_size);
            memcpy(page_buf, (U8 *) (&(dbbt.entry)), sizeof(dbbt.entry));
            page_addr = (i + dbbt_start_block) * fcb.pages_per_block + 4;   // page 4 for dbbt entry
            nand_write_page(0, page_addr, page_buf);
            memset(page_buf, 0, page_size);
            if (!nand_read_page(0, page_addr, page_buf))
                return FALSE;   // only for test
        }
    }

    return TRUE;
}

/**
 * rom_nand_firmware_burn - burn firmware
 * @page_buf: used to write nand page
 */
BOOL rom_nand_firmware_burn(BOOL is_skip, U8 * page_buf)
{
    U8 *firmware_addr;
    U32 firmware_length;
    U32 page_size = fcb.data_page_sz;
    U32 pages_per_block = fcb.pages_per_block;
    U32 page_addr;
    U32 left_pages;
    U32 block_addr;
	U32 firmware_ddr_addr = nand.bootcfg->ddr_addr;
    int sel;

    /* get firmware real length from image that is load into ddr by TRACE32 */
    if (is_skip)
        firmware_length = *((U32 *) (firmware_ddr_addr + IVT_OFFSET + FIRMWARE_LENGTH_OFFSET));
    else
        firmware_length = *((U32 *) (firmware_ddr_addr + FIRMWARE_LENGTH_OFFSET));

    /* burn firmware1 and firmware2 into nand */
    for (sel = 0; sel < 2; sel++) {
        if (sel == FIRMWARE_FIRST) {
            page_addr = fcb.Firmware1_startingSector;
        } else if (sel == FIRMWARE_SECOND) {
            page_addr = fcb.Firmware2_startingSector;
        }
        firmware_addr =
            (is_skip ? ((U8 *) firmware_ddr_addr) : ((U8 *) (firmware_ddr_addr - IVT_OFFSET)));
        left_pages = (firmware_length + fcb.data_page_sz - 1) / fcb.data_page_sz;

        /* firmware padding for 1kB if iamge has no 1kB skip area */
        if (!is_skip)
            memset(firmware_addr, 0, IVT_OFFSET);

        /* burn one firmware */
        while (left_pages > 0) {
            /* check if it is bad block when current page is the first page of one block */
            if ((page_addr & (pages_per_block - 1)) == 0) {
                block_addr = page_addr / pages_per_block;
                if ((!fcb.disbbm) && (!fcb.DISBB_Search)) {
                    while (is_block_bad_by_bbm(0, block_addr)) { // use bbm
                        page_addr += pages_per_block;
                        block_addr = page_addr / pages_per_block;
                    }
                }
                nand_erase_block(0, block_addr);    // erase first, then writable
            }

            /* copy firmware from ddr and then write it to nand */
            memset(page_buf, 0, page_size);
            memcpy(page_buf, firmware_addr, page_size);
            nand_write_page(0, page_addr, page_buf);

            /* read firmware page, only for test */
            memset(page_buf, 0, page_size);
            if (!nand_read_page(0, page_addr, page_buf))
                return FALSE;

            /* go to the next page */
            page_addr++;
            firmware_addr += page_size;
            left_pages--;
        }
    }

    return TRUE;
}
