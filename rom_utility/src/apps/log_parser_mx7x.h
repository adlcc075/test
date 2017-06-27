#ifndef LOG_PAESER_H
#define LOG_PAESER_H

#include "platform.h"

#define NULL 0

#define ROM_LOG_BUFFER_PTR	(0x1E0)
#define ROM_LOG_BUFFER_ENTRY_NUM	64

typedef union {
    UINT32 U;
    struct {
        UINT32 err_app:8;
        UINT32 err_drv:4;
        UINT32 err_low_drv:20;
    } B;
} boot_rom_error_code_u;

#define ERR_APP_SHIFT           (24)
#define ERR_DRIVER_TYPE_SHIFT   (20)
#define ERR_LOW_DRIVER_SHIFT    (0)
#define ERR_APP_MASK            (0xFFU << ERR_APP_SHIFT)
#define ERR_DRIVER_TYPE_MASK    (0xFU << ERR_DRIVER_TYPE_SHIFT)
#define ERR_DRIVER_MASK         (0xFFFFFU << ERR_LOW_DRIVER_SHIFT)

#define ERR_BASE_SDHC   (0 << ERR_DRIVER_TYPE_SHIFT)
#define ERR_BASE_NAND   (1 << ERR_DRIVER_TYPE_SHIFT)
#define ERR_BASE_QSPI   (2 << ERR_DRIVER_TYPE_SHIFT)
#define ERR_BASE_WEIM   (3 << ERR_DRIVER_TYPE_SHIFT)
#define ERR_BASE_SPI    (4 << ERR_DRIVER_TYPE_SHIFT)
#define ERR_BASE_I2C    (5 << ERR_DRIVER_TYPE_SHIFT)
#define ERR_BASE_SATA   (6 << ERR_DRIVER_TYPE_SHIFT)
#define ERR_BASE_USB    (7 << ERR_DRIVER_TYPE_SHIFT)
#define ERR_BASE_MISC   (15 << ERR_DRIVER_TYPE_SHIFT)

#define ROM_LOG_BOOTMODE_INTERNAL_FUSE      (0x10U << ERR_APP_SHIFT)
#define ROM_LOG_BOOTMODE_SERIAL             (0x11U << ERR_APP_SHIFT)
#define ROM_LOG_BOOTMODE_INTERNAL           (0x12U << ERR_APP_SHIFT)
#define ROM_LOG_BOOTMODE_TEST               (0x13U << ERR_APP_SHIFT)
#define ROM_LOG_SEC_CONFIG_FAB              (0x20U << ERR_APP_SHIFT)
#define ROM_LOG_SEC_CONFIG_RETURN           (0x21U << ERR_APP_SHIFT)
#define ROM_LOG_SEC_CONFIG_OPEN             (0x22U << ERR_APP_SHIFT)
#define ROM_LOG_SEC_CONFIG_CLOSED           (0x23U << ERR_APP_SHIFT)
#define ROM_LOG_DIR_BT_DIS_VALUE0           (0x30U << ERR_APP_SHIFT)
#define ROM_LOG_DIR_BT_DIS_VALUE1           (0x31U << ERR_APP_SHIFT)
#define ROM_LOG_BT_FUSE_SEL_VALUE0          (0x40U << ERR_APP_SHIFT)
#define ROM_LOG_BT_FUSE_SEL_VALUE1          (0x41U << ERR_APP_SHIFT)
#define ROM_LOG_PRIM_IMAGE_SELECT           (0x50U << ERR_APP_SHIFT)
#define ROM_LOG_SEC_IMAGE_SELECT            (0x51U << ERR_APP_SHIFT)
#define ROM_LOG_PRIM_BOOTDEVICE_NAND        (0x60U << ERR_APP_SHIFT)
#define ROM_LOG_PRIM_BOOTDEVICE_USDHC       (0x61U << ERR_APP_SHIFT)
#define ROM_LOG_PRIM_BOOTDEVICE_SATA        (0x62U << ERR_APP_SHIFT)
#define ROM_LOG_PRIM_BOOTDEVICE_I2C         (0x63U << ERR_APP_SHIFT)
#define ROM_LOG_PRIM_BOOTDEVICE_ECSPI       (0x64U << ERR_APP_SHIFT)
#define ROM_LOG_PRIM_BOOTDEVICE_NOR         (0x65U << ERR_APP_SHIFT)
#define ROM_LOG_PRIM_BOOTDEVICE_ONENAND     (0x66U << ERR_APP_SHIFT)
#define ROM_LOG_PRIM_BOOTDEVICE_QSPI        (0x67U << ERR_APP_SHIFT)
#define ROM_LOG_REC_BOOTDEVICE_I2C          (0x70U << ERR_APP_SHIFT)
#define ROM_LOG_REC_BOOTDEVICE_ECSPI        (0x71U << ERR_APP_SHIFT)
#define ROM_LOG_REC_BOOTDEVICE_NONE         (0x72U << ERR_APP_SHIFT)
#define ROM_LOG_MFG_BOOTDEVICE_USDHC        (0x73U << ERR_APP_SHIFT)
#define ROM_LOG_DEVICE_INIT_CALL            (0x80U << ERR_APP_SHIFT)
#define ROM_LOG_DEVICE_INIT_PASS            (0x81U << ERR_APP_SHIFT)
#define ROM_LOG_DEVICE_INIT_FAIL            (0x8FU << ERR_APP_SHIFT)
#define ROM_LOG_DEVICE_READ_DATA_CALL       (0x90U << ERR_APP_SHIFT)
#define ROM_LOG_DEVICE_READ_DATA_PASS       (0x91U << ERR_APP_SHIFT)
#define ROM_LOG_DEVICE_READ_DATA_FAIL       (0x9FU << ERR_APP_SHIFT)
#define ROM_LOG_AUTHENTICATION_STATUS       (0xA0U << ERR_APP_SHIFT)
#define ROM_LOG_PLUGIN_IMAGE_CALL           (0xB0U << ERR_APP_SHIFT)
#define ROM_LOG_PLUGIN_IMAGE_PASS           (0xB1U << ERR_APP_SHIFT)
#define ROM_LOG_PLUGIN_IMAGE_FAIL           (0xBFU << ERR_APP_SHIFT)
#define ROM_LOG_PROGRAM_IMAGE_CALL          (0xC0U << ERR_APP_SHIFT)
#define ROM_LOG_SDP_ENTRY                   (0xD0U << ERR_APP_SHIFT)
#define ROM_LOG_SDP_IMAGE_CALL              (0xD1U << ERR_APP_SHIFT)
#define ROM_LOG_ROMCP_PATCH                 (0xE0U << ERR_APP_SHIFT)

#define HAS_DATA		(0x01 << 0)
#define HAS_TIMESTAMP	(0x01 << 1)
#define HAS_SUB_ERR		(0x01 << 2)
#define HAS_NONE		(0x00)

/*********************************************
	USDHC error code 
 *******************************************/
/* SDHC error code bit map */
#define SDHC_HOST_ERR_SHIFT         (0)
#define SDHC_HOST_ERR_BITS          (4)
#define SDHC_HOST_ERR_MASK          (0xF << SDHC_HOST_ERR_SHIFT)

#define SDHC_COMM_ERR_SHIFT         (SDHC_HOST_ERR_SHIFT + SDHC_HOST_ERR_BITS) // 4
#define SDHC_COMM_ERR_BITS          (6)
#define SDHC_COMM_ERR_MASK          (0x3F << SDHC_COMM_ERR_SHIFT)

#define SDHC_PROTOCOL_ERR_SHIFT     (SDHC_COMM_ERR_SHIFT + SDHC_COMM_ERR_BITS) // 10
#define SDHC_PROTOCOL_ERR_BITS      (6)
#define SDHC_PROTOCOL_ERR_MASK      (0x3F << SDHC_PROTOCOL_ERR_SHIFT)

#define SDHC_INTFACE_ERR_SHIFT      (SDHC_PROTOCOL_ERR_SHIFT + SDHC_PROTOCOL_ERR_BITS) //16
#define SDHC_INTFACE_ERR_BITS       (4)
#define SDHC_INTFACE_ERR_MASK       (0xF << SDHC_INTFACE_ERR_SHIFT)

/*Define error codes for low level driver layer */
enum sdhc_drv_errcode {
    RSTA_CLEAR_ERR = 1,         // 1
    CIHB_CLEAR_ERR,             // 2
    CDIHB_CLEAR_ERR,            // 3
    CHECK_REPONSE_ERR,          // 4
    WAIT_BRR_ERR,               // 5
    WAIT_BWR_ERR,               // 6
    CHECK_DATA_ERR,             // 7
    CHECK_DATA_CRC_ERR,         // 8
    WAIT_BLK_GAP_ERR,           // 9
    WAIT_DLINE_FREE_ERR,        // 10
    WAIT_TP_ERR,                // 11
};

/*Define error codes for sd/mmc protocol layer */
enum sdhc_protocol_errcode {
    SD_INIT_ERR = 1,            // 1
    SD_VOL_VALIDATION_ERR,      // 2
    SD_GET_RCA_ERR,             // 3
    SD_SET_BOOT_PARTT_ERR,      // 4
    SD_SET_BUS_WIDTH_ERR,       // 5
    SD_SET_HIGH_SPEED_ERR,      // 6
    SD_SWITCH_VOLTAGE_ERR,      // 7
    SD_SWITCH_MODE_ERR,         // 8
    SD_SAMPLING_TUNING_ERR,     // 9
    MMC_INIT_ERR,               // 10
    MMC_READ_ERR,               // 11
    MMC_VOL_VALIDATION_ERR,     // 12
    MMC_SET_RCA_ERR,            // 13
    MMC_GET_CSD_ERR,            // 14
    MMC_SET_BOOT_PARTT_ERR,     // 15
    MMC_GET_ECSD_ERR,           // 16
    MMC_SET_BUS_WIDTH_ERR,      // 17
    MMC_SET_HIGH_SPEED_ERR,     // 18
    EMMC_BOOT_INIT_ERR,         // 19
    EMMC_READ_ERR,              // 20
    EMMC_PREIDLE_ERR,           // 21
    CARD_SW_RESET_ERR,          // 22
    CARD_GET_CID_ERR,           // 23
    CARD_SET_TRANS_MODE_ERR,    // 24
    CARD_CHECK_STATUS_ERR,      // 25
    CARD_VIRTUAL_READ_ERR,      // 26
    CARD_NOT_SUPPORT_1_8V,      // 27
    CARD_SWITCH_VOL_ERR,        // 28
    SD_VOL_NOT_SUPPORT,         // 29
};

/*Define error codes for sd/mmc interface layer */
enum sdhc_interface_errcode {
    CARD_INIT_ERR = 1,          // 1
    CARD_MFG_INIT_ERR,          // 2
    CARD_READ_ERR,              // 3
    CARD_NOT_READY,             // 4
};

#define ERROR_NUM_GET(err, mask, shift) \
			(((err) & (mask)) >> (shift))

/*********************************************
	NAND error code 
 *******************************************/
/* NAND error code bit map */
#define NAND_HAL_ERR_SHIFT				(0)
#define NAND_INTERFACE_ERR_SHIFT		(8)
#define NAND_BOOT_ERR_SHIFT				(16)

#define NAND_HAL_ERR_MASK          		(0xFF << NAND_HAL_ERR_SHIFT)
#define NAND_INTERFACE_ERR_MASK        	(0xFF << NAND_INTERFACE_ERR_SHIFT)
#define NAND_BOOT_ERR_MASK          	(0xF << NAND_BOOT_ERR_SHIFT)

/* NAND error code */
typedef enum {
    /* bch */
    ERR_BCH_ECC_LEVEL_EXCEED = 0x01,
    ERR_BCH_ECC_STATUS_TIMEOUT,
    ERR_BCH_ECC_STATUS_ALLONES,
    ERR_BCH_ECC_STATUS_UNCORRECTABLE,

    /* gpmi */
    ERR_GPMI_POLL_DEBUG_DISMATCH = 0x40,
    ERR_GPMI_ENABLE_FAIL,

    /* dma */
    ERR_DMA_STATUS_TIMEOUT = 0x80,
    ERR_DMA_STATUS_BUSY,
    ERR_DMA_WAIT,

    /* hal nand */
    ERR_HAL_NAND_SEND_CMD_FAIL = 0xC0,
    ERR_HAL_NAND_SEND_ADDR_FAIL,
    ERR_HAL_NAND_READ_RAW_DATA_FAIL,
    ERR_HAL_NAND_WRITE_RAW_DATA_FAIL,

} ERR_HAL_e;

typedef enum {
    ERR_INTERFACE_RESET = 1,
    ERR_INTERFACE_INIT,
    ERR_INTERFACE_READ_PAGE_SEND_DMA,
    ERR_INTERFACE_READ_PAGE_INNER,
    ERR_INTERFACE_READ_ID,
    ERR_INTERFACE_READ_RETRY_TAG,
    ERR_INTERFACE_RR_SEQ_NOT_EXSIT,
    ERR_INTERFACE_RR_REG_NUMBER_EXCEED,
    ERR_INTERFACE_READ_RETRY_RUN,
    ERR_INTERFACE_READ_RETRY_SUFFIX,
    ERR_INTERFACE_ASYNC_2_SYNC,
	ERR_INTERFACE_LEGACY_2_TOGGLE,

} ERR_INTERFACE_e;

typedef enum {
    ERR_BOOT_FCB_BLOCK_NOT_FOUND = 1,
    ERR_BOOT_DBBT_BLOCK_NOT_FOUND,
    ERR_BOOT_DBBT_DATA_READ,
    ERR_BOOT_INTERFACE_INIT,
    ERR_BOOT_DOWNLOAD_FIRMWARE,
    ERR_BOOT_READ_RETRY_SEQ_NOT_FOUND,
    ERR_BOOT_INTERFACE_SET_SYNC,
	ERR_BOOT_INTERFACE_SET_TOGGLE,
	ERR_BOOT_INTERFACE_SET_TOGGLE_FROM_LEGACY,

} ERR_BOOT_e;


typedef struct {
    unsigned int code;
    char *description;
    unsigned int flag;          // bit0 - has date, bit1 - has timestamp, bit2 - has sub err, other bits - reserved
} log_app_err_code_t, *log_app_err_code_p;

typedef struct {
	unsigned int code;
	char *description;
	int (*drv_err_parser)(chip_e chip, unsigned int err);
}log_sub_err_code_t, *log_sub_err_code_p;

typedef struct {
    unsigned code;
    char *description;
} log_data_hab_subcode_t, *log_data_hab_subcode_p;

typedef struct {
    unsigned code;
    char *description;
} log_data_subcode_t, *log_data_subcode_p;

int parser(chip_e chip, unsigned int log_buffer_base);

#endif
