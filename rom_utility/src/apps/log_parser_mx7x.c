#include "io.h"
#include "platform.h"
#include "log_parser_mx7x.h"
#include "hab_parser.h"
#include "shell/shell.h"

BOOL b_boot_from_nand = false;

log_app_err_code_t app_err_code_tbl[] = {
	{ROM_LOG_BOOTMODE_INTERNAL_FUSE  , "ROM_LOG_BOOTMODE_INTERNAL_FUSE ", HAS_NONE},
	{ROM_LOG_BOOTMODE_SERIAL         , "ROM_LOG_BOOTMODE_SERIAL        ", HAS_NONE},
	{ROM_LOG_BOOTMODE_INTERNAL       , "ROM_LOG_BOOTMODE_INTERNAL      ", HAS_NONE},
	{ROM_LOG_BOOTMODE_TEST           , "ROM_LOG_BOOTMODE_TEST          ", HAS_NONE},
	{ROM_LOG_SEC_CONFIG_FAB          , "ROM_LOG_SEC_CONFIG_FAB         ", HAS_NONE},
	{ROM_LOG_SEC_CONFIG_RETURN       , "ROM_LOG_SEC_CONFIG_RETURN      ", HAS_NONE},
	{ROM_LOG_SEC_CONFIG_OPEN         , "ROM_LOG_SEC_CONFIG_OPEN        ", HAS_NONE},
	{ROM_LOG_SEC_CONFIG_CLOSED       , "ROM_LOG_SEC_CONFIG_CLOSED      ", HAS_NONE},
	{ROM_LOG_DIR_BT_DIS_VALUE0       , "ROM_LOG_DIR_BT_DIS_VALUE0      ", HAS_NONE},
	{ROM_LOG_DIR_BT_DIS_VALUE1       , "ROM_LOG_DIR_BT_DIS_VALUE1      ", HAS_NONE},
	{ROM_LOG_BT_FUSE_SEL_VALUE0      , "ROM_LOG_BT_FUSE_SEL_VALUE0     ", HAS_NONE},
	{ROM_LOG_BT_FUSE_SEL_VALUE1      , "ROM_LOG_BT_FUSE_SEL_VALUE1     ", HAS_NONE},
	{ROM_LOG_PRIM_IMAGE_SELECT       , "ROM_LOG_PRIM_IMAGE_SELECT      ", HAS_NONE},
	{ROM_LOG_SEC_IMAGE_SELECT        , "ROM_LOG_SEC_IMAGE_SELECT       ", HAS_NONE},
	{ROM_LOG_PRIM_BOOTDEVICE_NAND    , "ROM_LOG_PRIM_BOOTDEVICE_NAND   ", HAS_NONE},
	{ROM_LOG_PRIM_BOOTDEVICE_USDHC   , "ROM_LOG_PRIM_BOOTDEVICE_USDHC  ", HAS_NONE},
	{ROM_LOG_PRIM_BOOTDEVICE_SATA    , "ROM_LOG_PRIM_BOOTDEVICE_SATA   ", HAS_NONE},
	{ROM_LOG_PRIM_BOOTDEVICE_I2C     , "ROM_LOG_PRIM_BOOTDEVICE_I2C    ", HAS_NONE},
	{ROM_LOG_PRIM_BOOTDEVICE_ECSPI   , "ROM_LOG_PRIM_BOOTDEVICE_ECSPI  ", HAS_NONE},
	{ROM_LOG_PRIM_BOOTDEVICE_NOR     , "ROM_LOG_PRIM_BOOTDEVICE_NOR    ", HAS_NONE},
	{ROM_LOG_PRIM_BOOTDEVICE_ONENAND , "ROM_LOG_PRIM_BOOTDEVICE_ONENAND", HAS_NONE},
	{ROM_LOG_PRIM_BOOTDEVICE_QSPI    , "ROM_LOG_PRIM_BOOTDEVICE_QSPI   ", HAS_NONE},
	{ROM_LOG_REC_BOOTDEVICE_I2C      , "ROM_LOG_REC_BOOTDEVICE_I2C     ", HAS_NONE},
	{ROM_LOG_REC_BOOTDEVICE_ECSPI    , "ROM_LOG_REC_BOOTDEVICE_ECSPI   ", HAS_NONE},
	{ROM_LOG_REC_BOOTDEVICE_NONE     , "ROM_LOG_REC_BOOTDEVICE_NONE    ", HAS_NONE},
	{ROM_LOG_MFG_BOOTDEVICE_USDHC    , "ROM_LOG_MFG_BOOTDEVICE_USDHC   ", HAS_NONE},
	{ROM_LOG_DEVICE_INIT_CALL        , "ROM_LOG_DEVICE_INIT_CALL       ", HAS_TIMESTAMP },
	{ROM_LOG_DEVICE_INIT_PASS        , "ROM_LOG_DEVICE_INIT_PASS       ", HAS_TIMESTAMP },
	{ROM_LOG_DEVICE_INIT_FAIL        , "ROM_LOG_DEVICE_INIT_FAIL       ", HAS_TIMESTAMP|HAS_SUB_ERR },
	{ROM_LOG_DEVICE_READ_DATA_CALL   , "ROM_LOG_DEVICE_READ_DATA_CALL  ", HAS_TIMESTAMP|HAS_DATA },
	{ROM_LOG_DEVICE_READ_DATA_PASS   , "ROM_LOG_DEVICE_READ_DATA_PASS  ", HAS_TIMESTAMP  },
	{ROM_LOG_DEVICE_READ_DATA_FAIL   , "ROM_LOG_DEVICE_READ_DATA_FAIL  ", HAS_TIMESTAMP|HAS_SUB_ERR },
	{ROM_LOG_AUTHENTICATION_STATUS   , "ROM_LOG_AUTHENTICATION_STATUS  ", HAS_TIMESTAMP|HAS_DATA },
	{ROM_LOG_PLUGIN_IMAGE_CALL       , "ROM_LOG_PLUGIN_IMAGE_CALL      ", HAS_DATA },
	{ROM_LOG_PLUGIN_IMAGE_PASS       , "ROM_LOG_PLUGIN_IMAGE_PASS      ", HAS_NONE },
	{ROM_LOG_PLUGIN_IMAGE_FAIL       , "ROM_LOG_PLUGIN_IMAGE_FAIL      ", HAS_NONE },
	{ROM_LOG_PROGRAM_IMAGE_CALL      , "ROM_LOG_PROGRAM_IMAGE_CALL     ", HAS_NONE },
	{ROM_LOG_SDP_ENTRY               , "ROM_LOG_SDP_ENTRY              ", HAS_DATA },
	{ROM_LOG_SDP_IMAGE_CALL          , "ROM_LOG_SDP_IMAGE_CALL         ", HAS_NONE },
	{ROM_LOG_ROMCP_PATCH             , "ROM_LOG_ROMCP_PATCH            ", HAS_NONE },
};

log_data_subcode_t nand_hal_tbl[] = {
	{ ERR_BCH_ECC_LEVEL_EXCEED, "ECC level exceed" }
	,
	{ ERR_BCH_ECC_STATUS_TIMEOUT, "ECC status timeout" }
	,
	{ ERR_BCH_ECC_STATUS_ALLONES, "ECC status all ones" }
	,
	{ ERR_BCH_ECC_STATUS_UNCORRECTABLE, "ECC status uncorrectable" }
	,
	{ ERR_GPMI_POLL_DEBUG_DISMATCH, "GPMI poll debug dismatch" }
	,
	{ ERR_GPMI_ENABLE_FAIL, "GPMI enable fail" }
	,
	{ ERR_DMA_STATUS_TIMEOUT, "DMA status timeout" }
	,
	{ ERR_DMA_STATUS_BUSY, "DMA status busy" }
	,
	{ ERR_DMA_WAIT, "DMA wait error" }
	,
	{ ERR_HAL_NAND_SEND_CMD_FAIL, "nand send CMD fail" }
	,
	{ ERR_HAL_NAND_SEND_ADDR_FAIL, "nand send address fail" }
	,
	{ ERR_HAL_NAND_READ_RAW_DATA_FAIL, "nand read raw data fail" }
	,
	{ ERR_HAL_NAND_WRITE_RAW_DATA_FAIL, "nand write raw data fail" }
	,
};

log_data_subcode_t nand_interface_tbl[] = {
	{ ERR_INTERFACE_RESET, "reset failed" }
	, 
	{ ERR_INTERFACE_INIT, "init failed" }
	,
	{ ERR_INTERFACE_READ_PAGE_SEND_DMA, "read page send DMA failed" }
	,
	{ ERR_INTERFACE_READ_PAGE_INNER, "read page inner failed" }
	,
	{ ERR_INTERFACE_READ_ID, "read id failed" }
	,
	{ ERR_INTERFACE_READ_RETRY_TAG, "read retry tag error" }
	,
	{ ERR_INTERFACE_RR_SEQ_NOT_EXSIT, "read retry SEQ not exsit" }
	,
	{ ERR_INTERFACE_RR_REG_NUMBER_EXCEED, "read retry register number exceed" }
	,
	{ ERR_INTERFACE_READ_RETRY_RUN, "read retry run failed" }
	,
	{ ERR_INTERFACE_READ_RETRY_SUFFIX, "read retry suffix failed" }
	,
	{ ERR_INTERFACE_ASYNC_2_SYNC, "Async change to Sync failed" }
	,
	{ ERR_INTERFACE_LEGACY_2_TOGGLE, "legacy change to toggle failed" }
	,
};

log_data_subcode_t nand_boot_tbl[] = {
	{ ERR_BOOT_FCB_BLOCK_NOT_FOUND, "FCB block not found" }
	,
	{ ERR_BOOT_DBBT_BLOCK_NOT_FOUND, "DBBT block not found" }
	,
	{ ERR_BOOT_DBBT_DATA_READ, "DBBT data read failed" }
	,
	{ ERR_BOOT_INTERFACE_INIT, "interface init failed" }
	,
	{ ERR_BOOT_DOWNLOAD_FIRMWARE, "download firmware failed" }
	,
	{ ERR_BOOT_READ_RETRY_SEQ_NOT_FOUND, "read retry sequence not found" }
	,
	{ ERR_BOOT_INTERFACE_SET_SYNC, "interface set Sync failed" }
	,
	{ ERR_BOOT_INTERFACE_SET_TOGGLE, "interface set toggle failed" }
	,
	{ ERR_BOOT_INTERFACE_SET_TOGGLE_FROM_LEGACY, "interface set toggle from legacy failed" }
	,
};

#ifdef HAB_LOG_PARSER_ENABLED
log_data_hab_subcode_t hab_status_tbl[] = {
    {HAB_STS_ANY, "Match any status in hab_rvt.report_event()"}
    ,
    {HAB_FAILURE, "Operation failed"}
    ,
    {HAB_WARNING, "Operation completed with warning"}
    ,
    {HAB_SUCCESS, "Operation completed successfully"}
    ,
};

log_data_hab_subcode_t hab_reason_tbl[] = {
    {HAB_RSN_ANY, "Match any reason in hab_rvt.report_event()"}
    ,
    {HAB_ENG_FAIL, "Engine failure"}
    ,
    {HAB_INV_ADDRESS, "Invalid address: access denied"}
    ,
    {HAB_INV_ASSERTION, "Invalid assertion"}
    ,
    {HAB_INV_CALL, "Function called out of sequence"}
    ,
    {HAB_INV_CERTIFICATE, "Invalid certificate"}
    ,
    {HAB_INV_COMMAND, "Invalid command: command malformed"}
    ,
    {HAB_INV_CSF, "Invalid @ref csf"}
    ,
    {HAB_INV_DCD, "Invalid @ref dcd"}
    ,
    {HAB_INV_INDEX, "Invalid index: access denied"}
    ,
    {HAB_INV_IVT, "Invalid @ref ivt"}
    ,
    {HAB_INV_KEY, "Invalid key"}
    ,
    {HAB_INV_RETURN, "Failed callback function"}
    ,
    {HAB_INV_SIGNATURE, "Invalid signature"}
    ,
    {HAB_INV_SIZE, "Invalid data size"}
    ,
    {HAB_MEM_FAIL, "Memory failure"}
    ,
    {HAB_OVR_COUNT, "Expired poll count"}
    ,
    {HAB_OVR_STORAGE, "Exhausted storage region"}
    ,
    {HAB_UNS_ALGORITHM, "Unsupported algorithm"}
    ,
    {HAB_UNS_COMMAND, "Unsupported command"}
    ,
    {HAB_UNS_ENGINE, "Unsupported engine"}
    ,
    {HAB_UNS_ITEM, "Unsupported configuration item"}
    ,
    {HAB_UNS_KEY, "Unsupported key type or parameters"}
    ,
    {HAB_UNS_PROTOCOL, "Unsupported protocol"}
    ,
    {HAB_UNS_STATE, "Unsuitable state"}
    ,
};

log_data_hab_subcode_t hab_context_tbl[] = {
    {HAB_CTX_ANY, "Match any context in hab_rvt.report_event()"}
    ,
    {HAB_CTX_FAB, "rom Event logged in hab_fab_test()"}
    ,
    {HAB_CTX_ENTRY, "Event logged in hab_rvt.entry()"}
    ,
    {HAB_CTX_TARGET, "Event logged in hab_rvt.check_target()"}
    ,
    {HAB_CTX_AUTHENTICATE, "Event logged in hab_rvt.authenticate_image()"}
    ,
    {HAB_CTX_DCD, "Event logged in hab_rvt.run_dcd()"}
    ,
    {HAB_CTX_CSF, "Event logged in hab_rvt.run_csf()"}
    ,
    {HAB_CTX_COMMAND, "Event logged executing @ref csf or @ref dcd command"}
    ,
    {HAB_CTX_AUT_DAT, "Authenticated data block"}
    ,
    {HAB_CTX_ASSERT, "Event logged in hab_rvt.assert()"}
    ,
    {HAB_CTX_EXIT, "Event logged in hab_rvt.exit()"}
    ,
};


static log_data_hab_subcode_p find_hab_subcode_entry(unsigned int subcode,
                                              log_data_hab_subcode_p tbl, unsigned int tbl_size)
{
    int i;
    log_data_hab_subcode_p tbl_entry;

    for (i = 0, tbl_entry = tbl; i < tbl_size; i++, tbl_entry++) {
        if (tbl_entry->code == subcode) {
            break;
        }
    }

    if (i < tbl_size)
        return tbl_entry;
    else
        return NULL;
}

static int hab_data_parser(chip_e chip, unsigned int data)
{
    unsigned char status = data & 0xff;
    unsigned char reason = (data >> 8) & 0xff;
    unsigned char context = (data >> 16) & 0xff;
    log_data_hab_subcode_p entry;

    printf("\thab data:\n");
    entry = find_hab_subcode_entry(status, hab_status_tbl, ARRAY_SIZE(hab_status_tbl));
    printf("\t\tstatus:0x%02x\t%s\n", status,
           entry != NULL ? entry->description : "Invalid hab_status code");

    entry = find_hab_subcode_entry(reason, hab_reason_tbl, ARRAY_SIZE(hab_reason_tbl));
    printf("\t\tstatus:0x%02x\t%s\n", reason,
           entry != NULL ? entry->description : "Invalid hab_reason code");

    entry = find_hab_subcode_entry(context, hab_context_tbl, ARRAY_SIZE(hab_context_tbl));
    printf("\t\tcontext:0x%02x\t%s\n", context,
           entry != NULL ? entry->description : "Invalid hab_context code");

    return 0;
}
#endif

static int usdhc_err_parser(chip_e chip, unsigned int err)
{
	printf("TODO");

	return 0;
}

static int nand_err_parser(chip_e chip, unsigned int err)
{
	int errno;
	int i;

	printf("\nnand error code parser:\n");

	/* parse nand boot error */
	errno = ERROR_NUM_GET(err, NAND_BOOT_ERR_MASK, NAND_BOOT_ERR_SHIFT);
	for (i = 0; i < ARRAY_SIZE(nand_boot_tbl); i++) {
		if (nand_boot_tbl[i].code == errno) {
			printf("nand boot %s\n", nand_boot_tbl[i].description);
			break;
		}
	}

	/* parse nand interface error */
	errno = ERROR_NUM_GET(err, NAND_INTERFACE_ERR_MASK, NAND_INTERFACE_ERR_SHIFT);
	for (i = 0; i < ARRAY_SIZE(nand_interface_tbl); i++) {
		if (nand_interface_tbl[i].code == errno) {
			printf("nand interface %s\n", nand_interface_tbl[i].description);
			break;
		}
	}

	/* parse nand HAL error */
	errno = ERROR_NUM_GET(err, NAND_HAL_ERR_MASK, NAND_HAL_ERR_SHIFT);
	for (i = 0; i < ARRAY_SIZE(nand_hal_tbl); i++) {
		if (nand_hal_tbl[i].code == errno) {
			printf("%s\n", nand_hal_tbl[i].description);
			break;
		}
	}

	return 0;
}

static int qspi_err_parser(chip_e chip, unsigned int err)
{
	printf("TODO");

	return 0;
}

static int weim_err_parser(chip_e chip, unsigned int err)
{
	printf("TODO");

	return 0;
}

static int spi_err_parser(chip_e chip, unsigned int err)
{
	printf("TODO");

	return 0;
}

log_sub_err_code_t sub_err_tbl[] = {
	{ERR_BASE_SDHC,	"SDHC", usdhc_err_parser},
	{ERR_BASE_NAND, "NAND", nand_err_parser},
	{ERR_BASE_QSPI, "QSPI", qspi_err_parser},
	{ERR_BASE_WEIM, "WEIM", weim_err_parser},
	{ERR_BASE_SPI,  "SPI",	spi_err_parser},
	{ERR_BASE_USB,  "USB",  NULL},
};

static int sub_err_code_parser(chip_e chip, unsigned int err)
{
	int i = 0;
	for(; i < sizeof(sub_err_tbl)/sizeof(sub_err_tbl[0]); i++){
		if((err&ERR_DRIVER_TYPE_MASK) == sub_err_tbl[i].code){
			break;
		}
	}

	if(i < sizeof(sub_err_tbl)/sizeof(sub_err_tbl[0])){
		if(sub_err_tbl[i].drv_err_parser != NULL){
			sub_err_tbl[i].drv_err_parser(chip, err);
		}
	}

	return 0;
}

int parser(chip_e chip, unsigned int log_buffer_base)
{
    unsigned int *entry = (unsigned int *)log_buffer_base;
    log_app_err_code_p code_tbl_entry;
    unsigned int num_entry = sizeof(app_err_code_tbl) / sizeof(app_err_code_tbl[0]);
    int i, j;

    printf("Index\tCode\tDescription\tSubErr\tLowErr\n");
    for (i = 0; i < ROM_LOG_BUFFER_ENTRY_NUM; entry++, i++) {
        if (0 == (*entry)) {
            break;
        }
        // Looking for the app error code.
        for (j = 0, code_tbl_entry = app_err_code_tbl; j < num_entry; j++, code_tbl_entry++) {
            if (((*entry) & ERR_APP_MASK) == code_tbl_entry->code) {
                break;
            }
        }
        // Then to parse it.
        if (j < num_entry) {
			if(ROM_LOG_PRIM_BOOTDEVICE_NAND == *entry){
				b_boot_from_nand = true;
			}

            printf("%d\t0x%08x\t%s", i, *entry, code_tbl_entry->description);
			if(code_tbl_entry->flag & HAS_SUB_ERR){
				sub_err_code_parser(chip, *entry);		
			}
			printf("\n");

            if(code_tbl_entry->flag & HAS_DATA){
                printf("\tData payload:\t0x%08x\n", *(++entry));
                i++;
            }

			if(code_tbl_entry->flag & HAS_TIMESTAMP){
				printf("\tTimestamp:\t0x%08x\n", *(++entry));
				i++;
			}
        } else {
            printf("Invalid status code 0x%08x\n", *entry);
        }
    }

    return 0;
}

extern chip_e chip;

int do_log_parse(unsigned int argc, char *argvs[])
{
    unsigned int log_buffer_ptr = (unsigned int )ROM_LOG_BUFFER_PTR;

    if(CHIP_MX6DQ_TO1_5 == chip || CHIP_MX6SDL_TO1_1 == chip || CHIP_MX6SL_TO1_0 == chip){
		log_buffer_ptr += 4;
    }else if(CHIP_MX6SDL_TO1_2 == chip){
		log_buffer_ptr += 8;
    }else if(CHIP_MX6SX_TO1_0 == chip){
		log_buffer_ptr = (unsigned int)0x1E0;
    }else if(CHIP_MX7D_TO1_0){
		log_buffer_ptr = (unsigned int)0x1E0;
	}

    printf("\nROM log buffer parsed as:\n");
    parser(chip, *((unsigned int *)log_buffer_ptr));

    return 0;
}

SHELL_CMD(log_parse, do_log_parse, 
"log_parse      Parse the rom log buffer                Usage: log_parse" )
