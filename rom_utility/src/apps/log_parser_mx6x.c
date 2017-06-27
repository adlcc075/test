#include "io.h"
#include "platform.h"
#include "log_parser_mx6x.h"
#include "hab_parser.h"
#include "shell/shell.h"

BOOL b_boot_from_nand = false;

static int default_data_parser(chip_e chip, unsigned int data);
static int hab_data_parser(chip_e chip, unsigned int data);

log_major_status_code_t major_status_code[] = {
    {ROM_LOG_BOOTMODE_INTERNAL_FUSE, "ROM_LOG_BOOTMODE_INTERNAL_FUSE", 0, NULL}
    ,
    {ROM_LOG_BOOTMODE_SERIAL, "ROM_LOG_BOOTMODE_SERIAL       ", 0, NULL}
    ,
    {ROM_LOG_BOOTMODE_INTERNAL, "ROM_LOG_BOOTMODE_INTERNAL     ", 0, NULL}
    ,
    {ROM_LOG_BOOTMODE_TEST, "ROM_LOG_BOOTMODE_TEST         ", 0, NULL}
    ,
    {ROM_LOG_SEC_CONFIG_FAB, "ROM_LOG_SEC_CONFIG_FAB        ", 0, NULL}
    ,
    {ROM_LOG_SEC_CONFIG_RETURN, "ROM_LOG_SEC_CONFIG_RETURN     ", 0, NULL}
    ,
    {ROM_LOG_SEC_CONFIG_OPEN, "ROM_LOG_SEC_CONFIG_OPEN       ", 0, NULL}
    ,
    {ROM_LOG_SEC_CONFIG_CLOSED, "ROM_LOG_SEC_CONFIG_CLOSED     ", 0, NULL}
    ,
    {ROM_LOG_DIR_BT_DIS_VALUE0, "ROM_LOG_DIR_BT_DIS_VALUE0     ", 0, NULL}
    ,
    {ROM_LOG_DIR_BT_DIS_VALUE1, "ROM_LOG_DIR_BT_DIS_VALUE1     ", 0, NULL}
    ,
    {ROM_LOG_BT_FUSE_SEL_VALUE0, "ROM_LOG_BT_FUSE_SEL_VALUE0    ", 0, NULL}
    ,
    {ROM_LOG_BT_FUSE_SEL_VALUE1, "ROM_LOG_BT_FUSE_SEL_VALUE1    ", 0, NULL}
    ,
    {ROM_LOG_PRIM_IMAGE_SELECT, "ROM_LOG_PRIM_IMAGE_SELECT     ", 0, NULL}
    ,
    {ROM_LOG_SEC_IMAGE_SELECT, "ROM_LOG_SEC_IMAGE_SELECT      ", 0, NULL}
    ,
    {ROM_LOG_PRIM_BOOTDEVICE_NAND, "ROM_LOG_PRIM_BOOTDEVICE_NAND  ", 0, NULL}
    ,
    {ROM_LOG_PRIM_BOOTDEVICE_USDHC, "ROM_LOG_PRIM_BOOTDEVICE_USDHC ", 0, NULL}
    ,
    {ROM_LOG_PRIM_BOOTDEVICE_SATA, "ROM_LOG_PRIM_BOOTDEVICE_SATA  ", 0, NULL}
    ,
    {ROM_LOG_PRIM_BOOTDEVICE_I2C, "ROM_LOG_PRIM_BOOTDEVICE_I2C   ", 0, NULL}
    ,
    {ROM_LOG_PRIM_BOOTDEVICE_ECSPI, "ROM_LOG_PRIM_BOOTDEVICE_ECSPI ", 0, NULL}
    ,
    {ROM_LOG_PRIM_BOOTDEVICE_NOR, "ROM_LOG_PRIM_BOOTDEVICE_NOR   ", 0, NULL}
    ,
    {ROM_LOG_PRIM_BOOTDEVICE_ONENAND, "ROM_LOG_PRIM_BOOTDEVICE_ONENAN", 0, NULL}
    ,
    {ROM_LOG_REC_BOOTDEVICE_I2C, "ROM_LOG_REC_BOOTDEVICE_I2C    ", 0, NULL}
    ,
    {ROM_LOG_REC_BOOTDEVICE_ECSPI, "ROM_LOG_REC_BOOTDEVICE_ECSPI  ", 0, NULL}
    ,
    {ROM_LOG_REC_BOOTDEVICE_NONE, "ROM_LOG_REC_BOOTDEVICE_NONE   ", 0, NULL}
    ,
    {ROM_LOG_MFG_BOOTDEVICE_USDHC, "ROM_LOG_MFG_BOOTDEVICE_USDHC  ", 0, NULL}
    ,
    {ROM_LOG_DEVICE_INIT_CALL, "ROM_LOG_DEVICE_INIT_CALL      ", 0, NULL}
    ,
    {ROM_LOG_DEVICE_INIT_PASS, "ROM_LOG_DEVICE_INIT_PASS      ", 0, NULL}
    ,
    {ROM_LOG_DEVICE_INIT_FAIL, "ROM_LOG_DEVICE_INIT_FAIL      ", 0, NULL}
    ,
    {ROM_LOG_DEVICE_READ_DATA_CALL, "ROM_LOG_DEVICE_READ_DATA_CALL ", 1, default_data_parser}
    ,
    {ROM_LOG_DEVICE_READ_DATA_PASS, "ROM_LOG_DEVICE_READ_DATA_PASS ", 0, NULL}
    ,
    {ROM_LOG_DEVICE_READ_DATA_FAIL, "ROM_LOG_DEVICE_READ_DATA_FAIL ", 0, NULL}
    ,
    {ROM_LOG_AUTHENTICATION_STATUS, "ROM_LOG_AUTHENTICATION_STATUS ", 1, hab_data_parser}
    ,
    {ROM_LOG_PLUGIN_IMAGE_CALL, "ROM_LOG_PLUGIN_IMAGE_CALL     ", 1, default_data_parser}
    ,
    {ROM_LOG_PLUGIN_IMAGE_PASS, "ROM_LOG_PLUGIN_IMAGE_PASS     ", 0, NULL}
    ,
    {ROM_LOG_PLUGIN_IMAGE_FAIL, "ROM_LOG_PLUGIN_IMAGE_FAIL     ", 0, NULL}
    ,
    {ROM_LOG_PROGRAM_IMAGE_CALL, "ROM_LOG_PROGRAM_IMAGE_CALL    ", 1, default_data_parser}
    ,
    {ROM_LOG_SDP_ENTRY, "ROM_LOG_SDP_ENTRY             ", 0, NULL}
    ,
    {ROM_LOG_SDP_IMAGE_CALL, "ROM_LOG_SDP_IMAGE_CALL        ", 1, default_data_parser}
    ,
};

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

static int default_data_parser(chip_e chip, unsigned int data)
{
    printf("\t\t0x%08x\n", data);

    return 0;
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

int parser(chip_e chip, unsigned int log_buffer_base)
{
    unsigned int *entry = (unsigned int *)log_buffer_base;
    log_major_status_code_p code_tbl_entry;
    unsigned int num_entry = sizeof(major_status_code) / sizeof(major_status_code[0]);
    int i, j;

    printf("Index\tCode\tDescription\n");
    for (i = 0; i < ROM_LOG_BUFFER_ENTRY_NUM; entry++, i++) {
        if (0 == (*entry)) {
            break;
        }
        // Looking for the major code.
        for (j = 0, code_tbl_entry = major_status_code; j < num_entry; j++, code_tbl_entry++) {
            if (*entry == code_tbl_entry->code) {
                break;
            }
        }
        // Then to parse it.
        if (j < num_entry) {
	    if(ROM_LOG_PRIM_BOOTDEVICE_NAND == *entry){
		b_boot_from_nand = true;
	    }
            printf("%d\t0x%08x\t%s\n", i, code_tbl_entry->code, code_tbl_entry->description);
            if (code_tbl_entry->has_data) {
                if (code_tbl_entry->data_parser != NULL) {
                    code_tbl_entry->data_parser(chip, *(++entry));
                    i++;
                }
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
    }

    printf("\nROM log buffer parsed as:\n");
    parser(chip, *((unsigned int *)log_buffer_ptr));

    return 0;
}

SHELL_CMD(log_parse, do_log_parse, 
"log_parse      Parse the rom log buffer                Usage: log_parse" )
