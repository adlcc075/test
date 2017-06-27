#include "io.h"
#include "platform.h"
#include "shell/shell.h"
#include "nand_parser.h"

#define  NAND_PRINT_MEMBER(fmt, args...)	\
	do{				\
		print_pre_symbol();	\
		printf(fmt, ## args);	\
	}while(0);

#define ENTER_STRUCT		struct_depth++;
#define EXIT_STRUCT		struct_depth--;

static unsigned int struct_depth = 0;
static int print_pre_symbol(void)
{
    unsigned int i = 0;

    for (; i < struct_depth; i++) {
        printf("    ");
    }

    printf("|---");

    return 0;
}

static int nand_apbh_dma_gpmi3_parser(apbh_dma_gpmi3_t * dma_gpmi3)
{
    ENTER_STRUCT;

    NAND_PRINT_MEMBER("nxt		0x%08x\n", dma_gpmi3->nxt);
    NAND_PRINT_MEMBER("cmd		0x%08x\n", dma_gpmi3->cmd);
    NAND_PRINT_MEMBER("bar		0x%08x\n", dma_gpmi3->bar);

    NAND_PRINT_MEMBER("gpmi_ctrl0	0x%08x\n",
                      dma_gpmi3->apbh_dma_gpmi3_u.apbh_dma_gpmi3_ctrl.gpmi_ctrl0);
    NAND_PRINT_MEMBER("gpmi_compare	0x%08x\n",
                      dma_gpmi3->apbh_dma_gpmi3_u.apbh_dma_gpmi3_ctrl.gpmi_compare);
    NAND_PRINT_MEMBER("gpmi_eccctrl	0x%08x\n",
                      dma_gpmi3->apbh_dma_gpmi3_u.apbh_dma_gpmi3_ctrl.gpmi_eccctrl);

    EXIT_STRUCT;

    return 0;

}

static int nand_apbh_dma_gpmi1_parser(apbh_dma_gpmi1_t * dma_gpmi1)
{
    ENTER_STRUCT;

    NAND_PRINT_MEMBER("nxt		0x%08x\n", dma_gpmi1->nxt);
    NAND_PRINT_MEMBER("cmd		0x%08x\n", dma_gpmi1->cmd);
    NAND_PRINT_MEMBER("bar		0x%08x\n", dma_gpmi1->bar);
    NAND_PRINT_MEMBER("gpmi_ctrl0	0x%08x\n",
                      dma_gpmi1->apbh_dma_gpmi1_u.apbh_dma_gpmi1_ctrl.gpmi_ctrl0);

    EXIT_STRUCT;

    return 0;
}

static int nand_apbh_dma_gpmi6_parser(apbh_dma_gpmi6_t * dma_gpmi6)
{
    ENTER_STRUCT;

    NAND_PRINT_MEMBER("nxt		0x%08x\n", dma_gpmi6->nxt);
    NAND_PRINT_MEMBER("cmd		0x%08x\n", dma_gpmi6->cmd);
    NAND_PRINT_MEMBER("bar		0x%08x\n", dma_gpmi6->bar);

    NAND_PRINT_MEMBER("gpmi_ctrl0	0x%08x\n",
                      dma_gpmi6->apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_ctrl0);
    NAND_PRINT_MEMBER("gpmi_compare	0x%08x\n",
                      dma_gpmi6->apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_compare);
    NAND_PRINT_MEMBER("gpmi_eccctrl	0x%08x\n",
                      dma_gpmi6->apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_eccctrl);
    NAND_PRINT_MEMBER("gpmi_ecccount	0x%08x\n",
                      dma_gpmi6->apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_ecccount);
    NAND_PRINT_MEMBER("gpmi_payload	0x%08x\n",
                      dma_gpmi6->apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_payload);
    NAND_PRINT_MEMBER("gpmi_auxiliary	0x%08x\n",
                      dma_gpmi6->apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_auxiliary);

    EXIT_STRUCT;

    return 0;
}

static int nand_apbh_dma_parser(apbh_dma_t * apbh_dma)
{
    ENTER_STRUCT;

    NAND_PRINT_MEMBER("nxt	0x%08x\n", apbh_dma->nxt);
    NAND_PRINT_MEMBER("cmd	0x%08x\n", apbh_dma->cmd);
    NAND_PRINT_MEMBER("bar	0x%08x\n", apbh_dma->bar);

    EXIT_STRUCT;

    return 0;
}

static int nand_dma_read_seed_parser(NAND_read_seed_t * read_seed)
{
    int i = 0;
    reg8_t *p = read_seed->tx_cle1_addr_dma_buffer.tx_cle1_addr_buf;

    ENTER_STRUCT;

    NAND_PRINT_MEMBER("uiAddressSize		0x%08x\n", read_seed->uiAddressSize);
    NAND_PRINT_MEMBER("uiReadSize		0x%08x\n", read_seed->uiReadSize);
    NAND_PRINT_MEMBER("uiECCMask		0x%08x\n", read_seed->uiECCMask);
    NAND_PRINT_MEMBER("bEnableHWECC		0x%02x\n", read_seed->bEnableHWECC);

    NAND_PRINT_MEMBER("tx_cle1_addr_buf[8]	");
    for (i = 0; i < (MAX_COLUMNS + MAX_ROWS + 1); i++, p++) {
        printf("0x%02x ", *p);
    }
    printf("\n");

    NAND_PRINT_MEMBER("tx_cle2_buf[1]		0x%02x\n",
                      read_seed->tx_cle2_addr_dma_buffer.tx_cle2_buf[0]);
    NAND_PRINT_MEMBER("pDataBuffer		0x%08x\n", read_seed->pDataBuffer);
    NAND_PRINT_MEMBER("pAuxBuffer		0x%08x\n", read_seed->pAuxBuffer);

    EXIT_STRUCT;

    return 0;
}

static int nand_dma_read_parser(NAND_dma_read_t * dma_read)
{
    ENTER_STRUCT;

    NAND_PRINT_MEMBER("apbh_dma_gpmi3_t tx_cle1_addr_dma\n");
    nand_apbh_dma_gpmi3_parser(&dma_read->tx_cle1_addr_dma);

    NAND_PRINT_MEMBER("pbh_dma_gpmi1_t tx_cle2_dma\n");
    nand_apbh_dma_gpmi1_parser(&dma_read->tx_cle2_dma);

    NAND_PRINT_MEMBER("apbh_dma_gpmi1_t wait_dma\n");
    nand_apbh_dma_gpmi1_parser(&dma_read->wait_dma);

    NAND_PRINT_MEMBER("apbh_dma_gpmi1_t sense_dma\n");
    nand_apbh_dma_gpmi1_parser(&dma_read->sense_dma);

    // This structure member was not used by ROM
    //NAND_PRINT_MEMBER("apbh_dma_gpmi1_t wait_dma1\n");
    //nand_apbh_dma_gpmi1_parser(&dma_read->wait_dma1);

    // This structure member was not used by ROM
    //NAND_PRINT_MEMBER("apbh_dma_gpmi1_t sense_dma1\n");
    //nand_apbh_dma_gpmi1_parser(&dma_read->sense_dma1);

    NAND_PRINT_MEMBER("apbh_dma_gpmi6_t rx_data_dma\n");
    nand_apbh_dma_gpmi6_parser(&dma_read->rx_data_dma);

    NAND_PRINT_MEMBER("apbh_dma_gpmi3_t rx_wait4done_dma\n");
    nand_apbh_dma_gpmi3_parser(&dma_read->rx_wait4done_dma);

    NAND_PRINT_MEMBER("apbh_dma_t success_dma\n");
    nand_apbh_dma_parser(&dma_read->success_dma);

    NAND_PRINT_MEMBER("apbh_dma_t timeout_dma\n");
    nand_apbh_dma_parser(&dma_read->timeout_dma);

    NAND_PRINT_MEMBER("NAND_read_seed_t NAND_DMA_Read_Seed\n");
    nand_dma_read_seed_parser(&dma_read->NAND_DMA_Read_Seed);

    EXIT_STRUCT;

    return 0;
}

static int nand_sector_desc_parser(NANDSectorDescriptorStruct_t * sector_desc)
{
    ENTER_STRUCT;

    NAND_PRINT_MEMBER("u32total_page_sz	0x%08x\n", sector_desc->u32total_page_sz);
    //NAND_PRINT_MEMBER("u16SectorSize      0x%08x\n", sector_desc->u16SectorSize); //Not used by ROM

    EXIT_STRUCT;

    return 0;
}

static int nand_descriptor_parser(NAND_Descriptor_t * nand_descriptor)
{
    ENTER_STRUCT;

    NAND_PRINT_MEMBER("NANDSectorDescriptorStruct_t stc_NANDSectorDescriptor\n");
    nand_sector_desc_parser(&nand_descriptor->stc_NANDSectorDescriptor);

    // This structure member was not used by ROM
    //NAND_PRINT_MEMBER("NANDCommandCodesStruct_t stc_NANDDeviceCommandCodes\n");
    //nand_cmd_codes_parser(&nand_descriptor->stc_NANDDeviceCommandCodes);

    NAND_PRINT_MEMBER("btNumRowBytes		0x%02x\n", nand_descriptor->btNumRowBytes);
    NAND_PRINT_MEMBER("btNumColBytes		0x%02x\n", nand_descriptor->btNumColBytes);
    //NAND_PRINT_MEMBER("btNANDDataBusWidth 0x%02x\n", nand_descriptor->btNANDDataBusWidth);    //Not used by ROM
    //NAND_PRINT_MEMBER("uiNANDTotalBlocks  0x%08x\n", nand_descriptor->uiNANDTotalBlocks);     //Not used by ROM
    //NAND_PRINT_MEMBER("u32NumLBAPerRead       0x%08x\n", nand_descriptor->u32NumLBAPerRead);  //Not used by ROM
    //NAND_PRINT_MEMBER("bBA_NAND           0x%02x\n", nand_descriptor->bBA_NAND);      //Not used by ROM

    EXIT_STRUCT;

    return 0;
}

static int nand_config_parser(nand_info_t * nand_config)
{
    ENTER_STRUCT;

    NAND_PRINT_MEMBER("data_page_sz            	0x%08x\n", nand_config->data_page_sz);
    NAND_PRINT_MEMBER("total_page_sz           	0x%08x\n", nand_config->total_page_sz);
    NAND_PRINT_MEMBER("blockn_ecc_level        	0x%08x\n", nand_config->blockn_ecc_level);
    NAND_PRINT_MEMBER("block0_sz           	0x%08x\n", nand_config->block0_sz);
    NAND_PRINT_MEMBER("blockn_sz           	0x%08x\n", nand_config->blockn_sz);
    NAND_PRINT_MEMBER("block0_ecc_level        	0x%08x\n", nand_config->block0_ecc_level);
    NAND_PRINT_MEMBER("meta_sz           	0x%08x\n", nand_config->meta_sz);
    NAND_PRINT_MEMBER("bch_type                  0x%08x\n", nand_config->bch_type);
    NAND_PRINT_MEMBER("blockn_num_per_page      0x%08x\n", nand_config->blockn_num_per_page);

    // This structure member was not used by ROM
    //NAND_PRINT_MEMBER("NandDmaTime_t  zNandDmaTime\n");
    //nand_dma_time_parser(&nand_config->zNandDmaTime);

    NAND_PRINT_MEMBER("bbm_byte_off_in_data	0x%08x\n",
                      nand_config->bbm_byte_off_in_data);
    NAND_PRINT_MEMBER("bbm_bit_off 		0x%08x\n", nand_config->bbm_bit_off);
    NAND_PRINT_MEMBER("bbm_physical_off   0x%08x\n", nand_config->bbm_physical_off);
    NAND_PRINT_MEMBER("bbm_spare_offset	0x%08x\n", nand_config->bbm_spare_offset);
    NAND_PRINT_MEMBER("OnfiSync_Enable          0x%08x\n", nand_config->OnfiSync_Enable);
    NAND_PRINT_MEMBER("OnfiSync_Speed           0x%08x\n", nand_config->OnfiSync_Speed);
    NAND_PRINT_MEMBER("DISBB_Search             0x%08x\n", nand_config->DISBB_Search);
    NAND_PRINT_MEMBER("bEnableHWECC             0x%02x\n", nand_config->bEnableHWECC);

    NAND_PRINT_MEMBER("NAND_dma_read_t DmaReadDma\n");
    nand_dma_read_parser(&nand_config->DmaReadDma);

    NAND_PRINT_MEMBER("NAND_Descriptor_t zNANDDescriptor\n");
    nand_descriptor_parser(&nand_config->zNANDDescriptor);

    NAND_PRINT_MEMBER("efNANDReadCmdCode1       0x%08x\n", nand_config->efNANDReadCmdCode1);
    NAND_PRINT_MEMBER("efNANDReadCmdCode2       0x%08x\n", nand_config->efNANDReadCmdCode2);
    NAND_PRINT_MEMBER("pages_per_block       	0x%08x\n", nand_config->pages_per_block);

    /*
       AddressCycles is the value of the ROW address cycle fuse, remove this from parser to avoid confusion.
     */
    //NAND_PRINT_MEMBER("AddressCycles      0x%08x\n", nand_config->AddressCycles);
    NAND_PRINT_MEMBER("boot_search_limit          0x%08x\n", nand_config->boot_search_limit);

    EXIT_STRUCT;

    return 0;
}

int nand_parser(NandBootData_t * nand)
{
    int i;
    NAND_PRINT_MEMBER("nand_info_t  NandConf \n");
    nand_config_parser(&nand->NandConf);

    NAND_PRINT_MEMBER("disbbm        		0x%08x\n", nand->disbbm);
    NAND_PRINT_MEMBER("dbbtNotFound  		0x%08x\n", nand->dbbtNotFound);
    NAND_PRINT_MEMBER("Firmware1_startingSector	0x%08x\n", nand->Firmware1_startingSector);
    NAND_PRINT_MEMBER("Firmware2_startingSector	0x%08x\n", nand->Firmware2_startingSector);
    NAND_PRINT_MEMBER("BadBlocksNum 		0x%08x\n", nand->BadBlocksNum);
    NAND_PRINT_MEMBER("BadBlocksTable 		0x%08x\n", nand->BadBlocksTable);

    if (0 != nand->BadBlocksNum) {
        printf("\nBad NAND Block : ");
        for (i = 0; i < nand->BadBlocksNum; i++) {
            if (0 == i % 16) {
                printf("\n\t");
            }
            printf("%d\t", nand->BadBlocksTable[i]);
        }
    }
    printf("\n");

    return 0;
}

extern chip_e chip;

int do_nand_parser(unsigned int argc, char *argvs[])
{
    NandBootData_t *boot_data = NULL;
    if (chip < CHIP_MX6DQ_MAX) {    // for all mx6dq TOs 
        boot_data = (NandBootData_t *) 0x900900;
    } else if (chip < CHIP_MX6SDL_MAX) {    // for all mx6sdl TOs
        boot_data = (NandBootData_t *) 0x900924;
    } else {
        printf("TO not supported.\n");
    }

    nand_parser(boot_data);

    return 0;
}

SHELL_CMD(nand_parse, do_nand_parser,
          "nand_parse\tParse the nand boot data\tUsage: nand_parse")
