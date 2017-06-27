#include "io.h"
#include "qspi_private.h"
#include "qspi.h"

#define LUT_QR 0                // Quad Read
#define LUT_WE 1                // Write Enable
#define LUT_RS 2                // Read Status (2 bytes)
#define LUT_PP 3                // Page Program
#define LUT_WC 4                // Write Config
#define LUT_RC 5                // Read Config (1 bytes)
#define LUT_BE 6                // Bulk Erase
#define LUT_WD 7                // Write Disable
#define LUT_READID 8            // Read ID
#define LUT_SE 9                // sector Erase
#define LUT_RNV 10              //read non-volatile register
#define LUT_WNV 11              //write non-volatile register

#define QSPI_LUTKEY_VALUE   0x5AF05AF0
#define QSPI_LCKER_LOCK     0x1
#define QSPI_LCKER_UNLOCK   0x2

#define PAGE_SIZE 256

#define QSPI1_MEM_BASE_ADDR 0x60000000
#define QSPI2_MEM_BASE_ADDR 0x70000000

extern uint32_t qspi_get_index(uint32_t instance);
extern uint32_t qspi_map_memory_base(uint32_t instance);


void qspi_host_reset(QSPI_MemMapPtr instance)
{
    uint32_t i;

    instance->MCR |= 0x3;  // Reset AHB & serial clock domain
    for (i = 0; i < 20; i++) ;
    instance->MCR &= ~(0x3);  // Release Reset (AHB & serial clock domain)

    instance->MCR |= (1 << 10);
    instance->MCR |= (1 << 11);

    // ISD3FB, ISD2FB, ISD3FA, ISD2FA = 1; ENDIAN = 0x3; END_CFG=0x3
    instance->MCR |= 0x000F000C;
    instance->MCR &= ~(0x20000000);
}

void qspi_config(QSPI_MemMapPtr instance, uint32_t ddr_en, uint32_t dqs_en, uint32_t dqs_loopback_en)
{
    uint32_t i;
    uint32_t index;

    index = qspi_get_index((uint32_t)instance);
    if (QUAD_SPI_INVAL == index) {
        printf("invalid instance\n");
        return;
    }

    instance->MCR |= 3;         // Reset AHB & serial clock domain
    for (i = 0; i < 20; i++) ;
    instance->MCR &= ~(3);      // Release Reset (AHB & serial clock domain)

    instance->MCR |= (1 << 14); // MDIS=1

    instance->SMPR = 0x00000000;    // sampled by sfif_clk_b; half cycle delay;
    instance->FLSHCR = 0x00000202;

    //Give the default source address:
    if (QUAD_SPI_2 == index)
        instance->SFAR = QSPI2_FLASH_A1_BASE;
    else
        instance->SFAR = QSPI1_FLASH_A1_BASE;

    // config the ahb buffer
    instance->BUF0IND = 0x00000020;
    instance->BUF1IND = 0x00000040;
    instance->BUF2IND = 0x00000060;
    instance->BUF0CR = 0x00001001; // CPU A9
    instance->BUF1CR = 0x00001006;  // CPU M4
    instance->BUF2CR = 0x00001003;  // SDMA
    instance->BUF3CR = 0x80001002;  // other masters

    // config the flash mmap
    if (QUAD_SPI_2 == index) {
        instance->SFA1AD = (QSPI2_FLASH_A1_TOP & 0xfffffc00);
        instance->SFA2AD = (QSPI2_FLASH_A2_TOP & 0xfffffc00);
        instance->SFB1AD = (QSPI2_FLASH_B1_TOP & 0xfffffc00);
        instance->SFB2AD = (QSPI2_FLASH_B2_TOP & 0xfffffc00);
    } else {
        instance->SFA1AD = (QSPI1_FLASH_A1_TOP & 0xfffffc00);
        instance->SFA2AD = (QSPI1_FLASH_A2_TOP & 0xfffffc00);
        instance->SFB1AD = (QSPI1_FLASH_B1_TOP & 0xfffffc00);
        instance->SFB2AD = (QSPI1_FLASH_B2_TOP & 0xfffffc00);
    }

    // ISD3FB, ISD2FB, ISD3FA, ISD2FA = 1; ENDIAN = 0x3; END_CFG=0x3
    instance->MCR |= 0x000F000C;
    instance->MCR &= ~(0x20000000);

    // DQS output invert
    if (dqs_en & !ddr_en & dqs_loopback_en)
        instance->MCR |= (1 << 30);
    else
        instance->MCR &= ~(1 << 30);

    if (dqs_loopback_en)
        instance->MCR |= (1 << 24);
    else
        instance->MCR &= ~(1 << 24);

    if (ddr_en)
        instance->MCR |= (1 << 7);
    else
        instance->MCR &= ~(1 << 7);

    if (dqs_en)
        instance->MCR |= (1 << 6);
    else
        instance->MCR &= ~(1 << 6);

    instance->MCR &= ~(1 << 14); // MDIS
}

static void qspi_unlock_lut(QSPI_MemMapPtr instance)
{
	instance->LUTKEY = QSPI_LUTKEY_VALUE;
	instance->LCKCR = QSPI_LCKER_UNLOCK;
}

static void qspi_lock_lut(QSPI_MemMapPtr instance)
{
	instance->LUTKEY = QSPI_LUTKEY_VALUE;
	instance->LCKCR = QSPI_LCKER_LOCK;
}

static void qspi_setupLUT(QSPI_MemMapPtr instance)
{
    qspi_unlock_lut(instance);
#if 0
    // SEQID 0 - single Read
    instance->LUT[0] = 0x08180403;
    instance->LUT[1] = 0x24001c08;
    //instance->LUT[1] = 0x1c040c0e;
    instance->LUT[2] = 0x00000000;
    instance->LUT[3] = 0;
#endif
    // SEQID 1 - Write enable
    instance->LUT[4] = 0x24000406;
    instance->LUT[5] = 0;
    instance->LUT[6] = 0;
    instance->LUT[7] = 0;

    // SEQID 2 - Read Status
    instance->LUT[8] = 0x1c010405;
    instance->LUT[9] = 0x00002400;
    instance->LUT[10] = 0;
    instance->LUT[11] = 0;

    // SEQID 3 - Page Program
    instance->LUT[12] = 0x08180402;
    instance->LUT[13] = 0x24002004;
    instance->LUT[14] = 0;
    instance->LUT[15] = 0;

    // SEQID 4 - Write Config/Status
    instance->LUT[16] = 0x20020401;
    instance->LUT[17] = 0;
    instance->LUT[18] = 0;
    instance->LUT[19] = 0;

    // SEQID 5 - Read Config
    instance->LUT[20] = 0x1c010415;
    instance->LUT[21] = 0x00002400;
    instance->LUT[22] = 0;
    instance->LUT[23] = 0;

    // SEQID 6 - Bulk Erase
    instance->LUT[24] = 0x240004c7;
    instance->LUT[25] = 0;
    instance->LUT[26] = 0;
    instance->LUT[27] = 0;

    // SEQID 7 - Write Disable
    instance->LUT[28] = 0x24000404;
    instance->LUT[29] = 0;
    instance->LUT[30] = 0;
    instance->LUT[31] = 0;

    //SEQID 8 - Read identification - REMS
    instance->LUT[32] = 0x1c04049F;
    instance->LUT[33] = 0x00002400;
    instance->LUT[34] = 0;
    instance->LUT[35] = 0;

    //SEQID 9 - erase sector
    instance->LUT[36] = 0x081804d8;
    instance->LUT[37] = 0x00002400;
    instance->LUT[38] = 0;
    instance->LUT[39] = 0;

    //SEQID 10 - read non-volatile register
    instance->LUT[36] = 0x1c0204b5;
    instance->LUT[37] = 0x00002400;
    instance->LUT[38] = 0;
    instance->LUT[39] = 0;

    //SEQID 11 - write non-volatile register
    instance->LUT[36] = 0x200204b1;
    instance->LUT[37] = 0x00002400;
    instance->LUT[38] = 0;
    instance->LUT[39] = 0;

    qspi_lock_lut(instance);
}

void qspi_wait_cmd(QSPI_MemMapPtr instance)
{
    while (instance->SR & (1 << 1));   // Bit 1 - IP_ACC

    // Wait for IP Command Transaction Finished Flag
    while (!(instance->FR & (1 << 0))) ;  // Bit 0 - TFF
}

void qspi_clear_tx_buf(QSPI_MemMapPtr instance)
{
    instance->MCR |= (1 << 11);
}

void qspi_clear_rx_buf(QSPI_MemMapPtr instance)
{
    instance->MCR |= (1 << 10);
}

void qspi_wait_fifo_ready(QSPI_MemMapPtr instance)
{
    while (!(instance->SR & (1 << 16))) ;   // Bit 1 - RXWE
}

void qspi_wait_idle(QSPI_MemMapPtr instance)
{
    while (instance->SR & (1 << 0)) ;   // Bit 0 - BUSY
}

void qspi_write_enable(QSPI_MemMapPtr instance)
{
    uint32_t status = 0;

    while (!(status & 0x2)) {
        instance->IPCR = (LUT_WE << 24);
        qspi_wait_cmd(instance);
        instance->FR |= (1 << 0);   // Write 1 to Clear TFF
        status = qspi_read_status(instance);
    }
}

void qspi_write_disable(QSPI_MemMapPtr instance)
{
    instance->IPCR = (LUT_WD << 24);
    qspi_wait_cmd(instance);
    instance->FR |= (1 << 0);   // Write 1 to Clear TFF
}

void qspi_write_status(QSPI_MemMapPtr instance, uint32_t status)
{
    instance->TBDR = status;
    instance->TBDR = 0;
    instance->TBDR = 0;
    instance->TBDR = 0;

    instance->IPCR = (LUT_WC << 24);
    qspi_wait_cmd(instance);
    instance->FR |= (1 << 0);   // Write 1 to Clear TFF
}

uint32_t qspi_read_id(QSPI_MemMapPtr instance)
{
    uint32_t status;

    instance->IPCR = (LUT_READID << 24);
    qspi_wait_cmd(instance);
    status = instance->RBDR[0];
    instance->FR |= (1 << 0);   // Write 1 to Clear TFF

    return status;
}

void qspi_select_ip_rx_buf(QSPI_MemMapPtr instance)
{
    instance->RBCT |= (1 << 8);
}

void qspi_select_ahb_rx_buf(QSPI_MemMapPtr instance)
{
    instance->RBCT &= ~(1 << 8);
}

uint32_t qspi_read_status(QSPI_MemMapPtr instance)
{
    uint32_t status_1;

    // Water Mark
    qspi_clear_rx_buf(instance);

    instance->IPCR = ((LUT_RS << 24) | 2);
    qspi_wait_cmd(instance);

    while(!(instance->RBSR & 0x3f00));
    status_1 = instance->RBDR[0];

    //printf("qspi status register: 0x%x\n", status_1);

    instance->FR |= (1 << 0);   // Write 1 to Clear TFF
    instance->FR |= (1 << 16);   // RBDF

    qspi_clear_rx_buf(instance);
    return status_1;
}

uint32_t qspi_read_config(QSPI_MemMapPtr instance)
{
    uint32_t status_1;

    // Water Mark
    qspi_clear_rx_buf(instance);

    instance->IPCR = ((LUT_RC << 24) | 2);
    qspi_wait_cmd(instance);

    while(!(instance->RBSR & 0x3f00));
    status_1 = instance->RBDR[0];

    //printf("qspi config register: 0x%x\n", status_1);

    instance->FR |= (1 << 0);   // Write 1 to Clear TFF
    instance->FR |= (1 << 16);   // RBDF

    qspi_clear_rx_buf(instance);
    return status_1;
}

void qspi_enable_quad_bit(QSPI_MemMapPtr instance)
{
    uint32_t index;

    index = qspi_get_index((uint32_t)instance);
    if (QUAD_SPI_INVAL == index) {
        printf("invalid instance\n");
        return;
    }

    // Serial Flash Address
    if (QUAD_SPI_2 == index)
        instance->SFAR = QSPI2_FLASH_A1_BASE;
    else
        instance->SFAR = QSPI1_FLASH_A1_BASE;

    // Write Enable Command
    qspi_write_enable(instance);

    // Write Status Register
    qspi_write_status(instance, 0x00020000);

    // Wait utill not BUSY
    while (qspi_read_status(instance) & 0x1) ;
}

void qspi_chip_erase(QSPI_MemMapPtr instance)
{
    uint32_t index;
    uint32_t base;

    index = qspi_get_index((uint32_t)instance);
    if (QUAD_SPI_INVAL == index) {
        printf("invalid instance\n");
        return;
    }

    // Serial Flash Address
    if (QUAD_SPI_2 == index)
        base = QSPI2_FLASH_A1_BASE;
    else
        base = QSPI1_FLASH_A1_BASE;


    qspi_select_ip_rx_buf(instance);
    qspi_write_enable(instance);
    instance->IPCR = (LUT_BE << 24);
    qspi_wait_cmd(instance);
    instance->FR |= (1 << 0);   // Write 1 to Clear TFF
    while (qspi_read_status(instance) & 0x1);
    qspi_write_disable(instance);
    qspi_select_ahb_rx_buf(instance);

    printf("erase 1\n");

    instance->SFAR = base + 0x8000000;

    qspi_select_ip_rx_buf(instance);
    qspi_write_enable(instance);
    instance->IPCR = (LUT_BE << 24);
    qspi_wait_cmd(instance);
    instance->FR |= (1 << 0);   // Write 1 to Clear TFF
    while (qspi_read_status(instance) & 0x1);
    qspi_write_disable(instance);
    qspi_select_ahb_rx_buf(instance);

    printf("erase 2\n");
}

void qspi_sector_erase(QSPI_MemMapPtr instance, uint32_t addr)
{
    qspi_select_ip_rx_buf(instance);

    instance->SFAR = addr;
    
    qspi_write_enable(instance);

    instance->IPCR = (LUT_SE << 24);
    qspi_wait_cmd(instance);
    instance->FR |= (1 << 0);   // Write 1 to Clear TFF
    while (qspi_read_status(instance) & 0x1);
    qspi_write_disable(instance);
    qspi_select_ahb_rx_buf(instance);
}

void qspi_write_non_volatile(QSPI_MemMapPtr instance, uint32_t status)
{
    // Write Enable Command
    qspi_write_enable(instance);

    instance->TBDR = status;
    instance->TBDR = 0;
    instance->TBDR = 0;
    instance->TBDR = 0;

    instance->IPCR = ((LUT_WNV<< 24) | 0x2);
    qspi_wait_cmd(instance);
    instance->FR |= (1 << 0);   // Write 1 to Clear TFF

    // Wait utill not BUSY
    while (qspi_read_status(instance) & 0x1) ;

    printf("0x60000400 = 0x%x\n", *(volatile unsigned int *)0x60000400);
}

#define INIT_WIRTE_SIZE 16

uint32_t qspi_program(QSPI_MemMapPtr instance, uint32_t off, uint32_t src, uint32_t size)
{
    uint32_t i, j;
    uint32_t* src_data = (uint32_t *)src;
    uint32_t page_cnt = 0, wlen = 0, base = 0, rlen = size, tlen = 0;

    qspi_select_ip_rx_buf(instance);
    qspi_host_reset(instance);

    base = qspi_map_memory_base((uint32_t)instance);
    if (!base) {
        printf("get base address failure\n");
        return 1;
    }

    base += off;

    page_cnt = size / PAGE_SIZE;
    if (size % PAGE_SIZE)
        page_cnt += 1;

    wlen = (size / 4) * 4;

    for (i = 0; i < page_cnt; i++) {
        instance->SFAR = base + i * PAGE_SIZE;

        qspi_write_enable(instance);

        if (rlen >= PAGE_SIZE)
            wlen = PAGE_SIZE;
        else
            wlen = rlen;

        if (rlen >= INIT_WIRTE_SIZE)
            tlen = INIT_WIRTE_SIZE;
        else
            tlen = rlen;

        for (j = 0; j < tlen/sizeof(uint32_t); j++) {
            instance->TBDR = *src_data;
            src_data++;
        }

        instance->IPCR = ((LUT_PP << 24) | wlen);

        for (j = 0;
             j < (wlen / sizeof(uint32_t) - tlen/sizeof(uint32_t));
             j++) {
            while (instance->SR & (1 << 27));
            instance->TBDR = *src_data;
            src_data++;
        }

        qspi_wait_idle(instance);

        while (qspi_read_status(instance) & (1 << 0));

        rlen -= wlen;

        qspi_clear_tx_buf(instance);

        qspi_write_disable(instance);
    }

    qspi_select_ahb_rx_buf(instance);

    return 0;
}

void qspi_init(QSPI_MemMapPtr instance)
{
    uint32_t ret = 0;
    
    qspi_config(instance, 0, 0, 0);
    qspi_setupLUT(instance);

    //qspi_write_non_volatile(instance, 0x0000ffff);
    ret = qspi_read_config(instance);
    printf("qspi config register: 0x%x\n", ret);
    ret = qspi_read_status(instance);
    printf("qspi status register: 0x%x\n", ret);
    
}
