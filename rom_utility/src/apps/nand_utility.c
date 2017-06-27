/*===================================================================================
                                        INCLUDE FILES
==================================================================================*/
#include  "timer/timer.h"
#include  "shell/shell.h"
#include  "debug.h"
#include  "rom_nand_boot.h"
#include  "nand_base.h"
#include  "rom_nand_burn.h"
#include  "nand_read_retry.h"

/*=================================================================================
                                 GLOBAL FUNCTION PROTOTYPES
=================================================================================*/
extern void start_pmccntr(void);
extern unsigned int read_stop_pmccntr(void);

/*=================================================================================
                                 GLOBAL varibles
=================================================================================*/
/* cpu chip serial number */  
extern chip_e chip;    

/* fcb struct */
extern FCBStruct_t fcb;

/* nand help strings */
char nand_help[] =
    "usage: nand <command> [<args>]\n"
    "nand command details:\n"
    "   burn    burn a binary image to nand flash\n"
    "           is_skip: determin wether we need to skip 1KB of firmware\n"
    "   info    print nand informations, include nand id, manufacture, page size etc\n"
    "   readfcb    read fcb page and dump all the fcb informations.\n"
	"              This is very useful when debug nand boot fail issues.\n"
    "   readotp    read hynix nand otp out, mainly for read retry sequence.\n"
    "   read    read some continuous nand pages and dump them, oob excluded\n"
    "           page_start: start page\n"
    "           page_cnt: page numbers to dump\n"
    "   readraw    read some continuous nand pages and dump them, ECC and randomizer disabled\n"
    "           page_start: start page\n"
    "           page_cnt: page numbers to dump\n"
    "   dump    read some continuous nand pages and dump them, oob included\n"
    "           page_start: start page\n"
    "           page_cnt: page numbers to dump\n"
    "   get     read some continuous nand pages and dump them, oob excluded\n"
    "           page_start: start page\n"
    "           page_cnt: page numbers to dump\n"
    "           total_page_sz: total page size, data_size + oob_size\n"
    "           is_ecc: is ecc checking used or not\n"
    "           is_rand: is randomizer used or not\n"
    "           meta_sz: metadata size\n"
    "           data_block_sz: data block0 and blockn size\n"
    "           ecc_level: data block0 and blockn ecc level\n"
    "           num_blockn: number of ecc data blockn, block 0 excluded\n"
    "           is_GF14: 0 if use GF13, 1 if use GF14\n"
    "   erase   nand erase command, bad block will be skip but also recongized one block\n"
    "           block_start: start block address\n"
    "           block_cnt: block number to erase\n"
    "   scrub   nand scrub command, don't recongise bad block\n"
    "           block_start: start block address\n"
    "           block_cnt: block number to erase\n"
    "   rrtest  test read retry feature\n"
    "           block_addr: the address of block to be erased, so that it can enter read retry\n"
    "           erase_cnt: the count of erase command\n"
    "   testburn    for validation use, burn a binary image to nand flash\n"
    "           is_skip: determin wether we need to skip 1KB of firmware\n"
    "           badblock1_addr: validation bad block, if less than 0, think as useless input\n"
    "           badblock2_addr: validation bad block, if less than 0, think as useless input\n"
    "           is_disbbm: whether disable bbm swap or not, 1 means disable\n"
    "           ecc_level: if validate bch62, set 62 here, must set even number here\n"
    "           toggle_speed: validation toggle nand, if less than 0, think as useless input\n"
    "           is_fcb1_error: set fcb finger print in 1st block error, to validate fcb in 2nd block \n"
    "           is_no_dbbt: whether there is no dbbt or not\n"
    "           sync_speed: validation sync nand, if less than 0, think as useless input\n"
    "           is_rand_en: whether enable randomizer or not, in pele1.2 and ult, randomizer enable\n"
    "                       is controlled in FCB, so must fill this item.\n";

/*================================================================================
                          Hash Defines And Macros
=================================================================================*/
typedef int (*FUNC_ARGC0) (void);
typedef int (*FUNC_ARGC1) (int);
typedef int (*FUNC_ARGC2) (int, int);
typedef int (*FUNC_ARGC3) (int, int, int);
typedef int (*FUNC_ARGC4) (int, int, int, int);
typedef int (*FUNC_ARGC5) (int, int, int, int, int);
typedef int (*FUNC_ARGC6) (int, int, int, int, int, int);
typedef int (*FUNC_ARGC7) (int, int, int, int, int, int, int);
typedef int (*FUNC_ARGC8) (int, int, int, int, int, int, int, int);
typedef int (*FUNC_ARGC9) (int, int, int, int, int, int, int, int, int);
typedef int (*FUNC_ARGC10) (int, int, int, int, int, int, int, int, int, int);

/* nand cmd struct */
typedef struct nand_cmd {
    char name[10];
    void *pfunc;
    int argc;
    char help[150];
} nand_cmd_t;

/*==========================================================================
                                     Global FUNCTIONS
==========================================================================*/
/** 
  * cmd_nand_info - print nand information, include nand id, manufacture, 
                    page data size, page oob size, pages per block,
					and whether read retry needed
  */
int cmd_nand_info(void)
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

    /* print manufacture name */
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
        return -1;
    }

    /* fill page size members in fcb struct */
    if (!nand_fcb_fill_page_size(ids)) {
        printf("fcb fill page size failed\n");
        return -1;
    }

    /* print nand infomations */
    printf("page data : %ld\n", fcb.data_page_sz);
    printf("oob : %ld\n", fcb.total_page_sz - fcb.data_page_sz);
    printf("pages per block : %ld\n", fcb.pages_per_block);
    printf("read retry enable : %ld\n", fcb.read_retry_en);

    return 0;
}

/** 
  * cmd_nand_read_fcb - read fcb page and dump all the fcb informations. 
  * This is very useful when debug nand boot fail issues.
  */
int cmd_nand_read_fcb(void)
{
    FCBStruct_t *pFCB;
    U32 search_stride;

    /* init nand_data struct for searching fcb */
    if (!nand_boot_init_param()) {
        return -1;
    }

    /* search fcb */
    search_stride = nand.info.pages_per_block;
    if (!nand_boot_fcb_search(search_stride, nand.page_buffer, (FCBStruct_t **) & pFCB)) {
        printf("fcb not found!\n");
        return -1;
    } else
        printf("fcb found success!\n");

	/* dump the content of FCB */
	nand_fcb_dump(pFCB);

	return 0;
}

/** 
  * cmd_nand_read_otp - read hynix nand otp out, mainly for read retry sequence
  */
int cmd_nand_read_otp(void)
{
    U8 *page_buf = nand.page_buffer;
    static U8 ids[6];           // must be static because we will pass it to subfuctions
	int i;

	/* read nand id first */
    memset(ids, 0, sizeof(ids));
    nand_read_id(0, 0x00, ids);

    /* get OTP and read retry table */
	if ((ids[0] == NAND_MFR_HYNIX) && ((ids[5] & 0x07) == 0x04)) {
		if (!rr_get_rrt_from_nand(ids, page_buf)) {
			return -1;
		}
	} else {
		printf("not hynix nand or not hynix 20nm nand, please check it!\n");
		return 0;
	}

	/* print OTP */
    for (i = 0; i < NAND_HYNIX_OTP_LEN; i++) {
        if ((i != 0) && (i % 16 == 0)) {
            printf("\n");					// change new line when 16 bytes data printed
		} else if ((i != 0) && (i % 4 == 0)) {
            printf("    ");					// print blank when 4 bytes printed (one word)
		}
        printf("%2x ", nand.page_buffer[i]);
    }
    printf("\n");

	return 0;
}

/**
  * cmd_nand_read - read some continuous nand pages and dump them, oob excluded
  * @page_start: start page
  * @page_cnt: page numbers to dump
  *
  * return 0 if succeed, return -1 otherwise
  */
int cmd_nand_read(int page_start, int page_cnt)
{
    U32 page_addr = page_start;
    int i;

	/* get fcb to init nand data first */
    if (!nand_boot_init()) {
     	printf("nand boot init failed, please use nand get ");
        return -1;
    }

    /* read nand pages and print them */
    while (page_cnt--) {

        /* init param when reading FCB pages and other pages */
        if (page_addr < nand.info.pages_per_block * nand.info.boot_search_limit) {    // fcb pages
    		if (!nand_boot_init_param()) {
        		return -1;
    		}
		} else {	// other pages
    		if (!nand_boot_init()) {
        		return -1;
    		}
		}

		/* read one page */
        if (!nand_read_page(0, page_addr, nand.page_buffer)) {
            printf("nand read page error!\n");
            return -1;
        }

        /* dump nand page data */
        printf("page %d dump:\n", page_addr);
        for (i = 0; i < nand.info.data_page_sz; i++) {
            if ((i != 0) && (i % 16 == 0)) {
            	if ((i % 512 == 0))
                	printf("\t%d * 512B", i / 512);	// print data size that have been dumped, 512B unit
                printf("\n");					// change new line when 16 bytes data printed
			} else if ((i != 0) && (i % 4 == 0))
                printf("    ");					// print blank when 4 bytes printed (one word)
            printf("%2x ", nand.page_buffer[i]);
        }
        printf("\n");

        page_addr++;
    }

    return 0;
}

/**
  * cmd_nand_dump - read some continuous nand pages and dump them, oob included, ECC disabled
                    randomizer enable or disable controlled by the item set in FCB
  * @page_start: start page
  * @page_cnt: page numbers to dump
  *
  * return 0 if succeed, return -1 otherwise
  */
int cmd_nand_dump(int page_start, int page_cnt)
{
    U32 page_addr = page_start;
	U32 metadata_size;
	U32 blockn_size;
	U32 blockn_oob_size;
	U32 total_size;
	BOOL ecc_status_save;
    int i, j;

    /* boot nand first */
    if (!nand_boot_init()) {
        printf("nand boot init failed, please use nand get\n");
        return -1;
    }

    /* read nand pages and print them */
    while (page_cnt--) {

        /* init param when reading FCB pages and other pages */
        if (page_addr < nand.info.pages_per_block * nand.info.boot_search_limit) {   // fcb page
    		if (!nand_boot_init_param()) {
        		return -1;
    		}
		} else {	// other pages
    		if (!nand_boot_init()) {
        		return -1;
    		}
		}

		/* read one page */
		ecc_status_save = nand.info.ecc_en;
        nand.info.ecc_en = 0;   // disable hw ecc to read oob
        if (!nand_read_page(0, page_addr, nand.page_buffer)) {
            printf("nand read page error!\n");
			nand.info.ecc_en = ecc_status_save; // restore ecc_en
            return -1;
        }

        /* dump nand page data */
		metadata_size = nand.info.bchcfg.meta_sz;
		blockn_size = nand.info.bchcfg.blockn_sz + 
			nand.info.bchcfg.blockn_ecc_level * (nand.info.bchcfg.bch_type + 13) / 8;
		blockn_oob_size = nand.info.bchcfg.blockn_ecc_level * (nand.info.bchcfg.bch_type + 13) / 8;
		total_size = metadata_size + 
			blockn_size * (nand.info.bchcfg.blockn_num_per_page + 1); 

        printf("page %d dump:\n", page_addr);
		for(i =0; i < metadata_size; i++) {
			if(i == 0)
				printf("metadata:\n");
			printf("%2x ", nand.page_buffer[i]);
		}
		for(i = 0; i < total_size - metadata_size; i++) {
			if (i % blockn_size == 0) {
				printf("\nblock %d:\n", i / blockn_size);
				j = 0;
			}
			if ((i + blockn_oob_size) % blockn_size == 0) {
				printf("\noob %d:\n", (i + blockn_oob_size) / blockn_size - 1);
				j = 0;
			}

			if ((j != 0) && (j % 16 == 0))
				printf("\n");
			else if((j != 0) && (j % 4 == 0)) 
				printf("    ");

           	printf("%2x ", nand.page_buffer[i + metadata_size]);
			j++;
        }
        printf("\n");

        page_addr++;
    }

	nand.info.ecc_en = ecc_status_save; // restore ecc_en
    return 0;
}

/**
  * cmd_nand_read_raw - read some continuous nand pages and dump them, oob excluded
  * @page_start: start page
  * @page_cnt: page numbers to dump
  *
  * return 0 if succeed, return -1 otherwise
  */
int cmd_nand_read_raw(int page_start, int page_cnt)
{
    int i;
    U32 page_addr = page_start;
    static U8 ids[6];           // must be static because we will pass it to subfuctions

	/* read nand id first */
    memset(ids, 0, sizeof(ids));
    nand_read_id(0, 0x00, ids);

    /* init nand_data struct */
    if (!nand_boot_init_param()) {
        return -1;
    }

    /* fill page size members in fcb struct to get total_page_sz */
    if (!nand_fcb_fill_page_size(ids)) {
        printf("fcb fill pages_per_block failed\n");
        return -1;
    }
	nand.info.total_page_sz = fcb.total_page_sz;

	/* disable ECC, randomizer, and read retry */
	nand.info.ecc_en = FALSE;
	nand.info.rand_en = FALSE;
	nand.info.read_retry_en = FALSE;
	nand.info.hamming_en = FALSE;

    /* read nand pages and print them */
    while (page_cnt--) {

        /* read one nand page */
        if (!nand_read_page(0, page_addr, nand.page_buffer)) {
            printf("nand read raw page error!\n");
            return -1;
        }

        /* dump nand page data */
        printf("page %d dump:\n", page_addr);
        for (i = 0; i < fcb.total_page_sz; i++) {
            if ((i != 0) && (i % 16 == 0)) {
            	if ((i % 512 == 0))
                	printf("\t%d * 512B", i / 512);	// print data size that have been dumped, 512B unit
                printf("\n");					// change new line when 16 bytes data printed
			} else if ((i != 0) && (i % 4 == 0))
                printf("    ");					// print blank when 4 bytes printed (one word)
            printf("%2x ", nand.page_buffer[i]);
        }
        printf("\n");

        page_addr++;
    }

    return 0;
}

/**
  * cmd_nand_get - read some continuous nand pages and dump them, oob excluded
  * @page_start: start page
  * @page_cnt: page numbers to dump
  * @total_page_sz: total page size, data_size + oob_size
  * @is_ecc: is ecc checking used or not
  * @is_rand: is randomizer used or not
  * @meta_sz: metadata size
  * @data_block_sz: data block0 and blockn size
  * @ecc_level: data block0 and blockn ecc level
  * @num_blockn: number of ecc data blockn, block 0 excluded
  * @is_GF14: 0 if use GF13, 1 if use GF14
  *
  * return 0 if succeed, return -1 otherwise
  */
int cmd_nand_get(int page_start, int page_cnt, int total_page_sz, 
                 int is_ecc, int is_rand, int meta_sz, int data_block_sz, 
				 int ecc_level, int num_blockn, int is_GF14 )
{
    U32 page_addr = page_start;
    int i;

    /* init nand_data struct */
    if (!nand_boot_init_param()) {
        return -1;
    }

	/* set other nand_data parameters by input args */
	nand.info.rand_en = is_rand;
	nand.info.rand_type = 2;
	nand.info.total_page_sz = total_page_sz;
	nand.info.ecc_en = is_ecc;
	nand.info.bchcfg.meta_sz = meta_sz; 
	nand.info.bchcfg.block0_sz = data_block_sz;
	nand.info.bchcfg.blockn_sz = data_block_sz;
	nand.info.bchcfg.block0_ecc_level = ecc_level;
	nand.info.bchcfg.blockn_ecc_level = ecc_level;
	nand.info.bchcfg.blockn_num_per_page = num_blockn;
	nand.info.bchcfg.bch_type = is_GF14;
	nand.info.bchcfg.total_page_sz = total_page_sz;

    /* read nand pages and print them */
    while (page_cnt--) {

		/* read one page */
        if (!nand_read_page(0, page_addr, nand.page_buffer)) {
            printf("nand read page error!\n");
            return -1;
        }

        /* dump nand page data */
        printf("page %d dump:\n", page_addr);
        for (i = 0; i < nand.info.data_page_sz; i++) {
            if ((i != 0) && (i % 16 == 0)) {
            	if ((i % 512 == 0))
                	printf("\t%d * 512B", i / 512);	// print data size that have been dumped, 512B unit
                printf("\n");					// change new line when 16 bytes data printed
			} else if ((i != 0) && (i % 4 == 0))
                printf("    ");					// print blank when 4 bytes printed (one word)
            printf("%2x ", nand.page_buffer[i]);
        }
        printf("\n");

        page_addr++;
    }

    return 0;
}

/** 
  * cmd_nand_erase - nand erase command, bad block will be skip but also recongized one block
  * @block_start: start block address
  * @block_cnt: block number to erase
  *
  * return 0 if succeed, return -1 otherwise
  */
int cmd_nand_erase(int block_start, int block_cnt)
{
    static U8 ids[6];           // must be static because we will pass it to subfuctions
    U32 block_addr = block_start;

	/* read nand id first */
    memset(ids, 0, sizeof(ids));
    nand_read_id(0, 0x00, ids);

    /* fill page size members in fcb struct to get pages_per_block */
    if (!nand_fcb_fill_page_size(ids)) {
        printf("fcb fill pages_per_block failed\n");
        return -1;
    }
	nand.info.pages_per_block = fcb.pages_per_block;

    /* erase nand blocks */
    while (block_cnt--) {
        /* erase one nand block */
        if (!nand_erase_block(0, block_addr)) {
            printf("nand erase block error!\n");
            return -1;
        }

        block_addr++;
    }

    /* dump erase information */
    printf("NAND erase complete\n");

    return 0;
}

/** 
  * cmd_nand_scrub - nand scrub command, don't skip bad block
  * @block_start: start block address
  * @block_cnt: block number to erase
  *
  * return 0 if succeed, return -1 otherwise
  */
int cmd_nand_scrub(int block_start, int block_cnt)
{
    static U8 ids[6];           // must be static because we will pass it to subfuctions
    U32 block_addr = block_start;

	/* read nand id first */
    memset(ids, 0, sizeof(ids));
    nand_read_id(0, 0x00, ids);

    /* fill page size members in fcb struct to get pages_per_block */
    if (!nand_fcb_fill_page_size(ids)) {
        printf("fcb fill pages_per_block failed\n");
        return -1;
    }
	nand.info.pages_per_block = fcb.pages_per_block;

    /* erase nand blocks */
    while (block_cnt--) {
        /* erase one nand block */
        if (!nand_erase_block(0, block_addr)) {
            printf("bad block found: %d\n", block_addr);
        }

        block_addr++;
    }

    /* dump scrub information */
    printf("NAND scrub complete\n");

    return 0;
}

/**
  * cmd_nand_markbad - nand markbad, mark block as bad block by changing bbm as 0
  * @ block_addr: block addr to markbad
  *
  * return 0 if succeed, otherwise return -1
  */
int cmd_nand_markbad(int block_addr)
{
	if(!nand_mark_block_bad(0, block_addr))
		return -1;

	printf("block %d markbad finished!\n", block_addr);
	return 0;
}

/**
  * cmd_nand_burn - nand burn, include fcb, dbbt and firmware burning
  * @ is_skip: wether we need to skip 1KB of firmware
  */
int cmd_nand_burn(int is_skip)
{
    int block_start;
	int i;

    /* init nand_data struct for searching fcb */
    if (!nand_boot_init_param()) {
        return -1;
    }

    /* burn fcb */
    if (!rom_nand_fcb_burn(nand.page_buffer)) {
        printf("FCB burn error!\n");
        return -1;
    } else
        printf("FCB burn success!\n");

    /* boot nand before other operation, this change is to align with other commands */
    if (!nand_boot_init()) {
        printf("nand boot init failed!\n");
        return -1;
    }

    /* scrub block first, otherwise block will be recongized bad if block has been written because of randomizer */
    block_start = fcb.DBBTSearchAreaStartAddress / fcb.pages_per_block;
    for (i = block_start; i < block_start + 90; i++) {
        if (!nand_erase_block(0, i)) {
            printf("bad block found: %d\n", i);
        }
    }

    /* burn dbbt */
    if (!rom_nand_dbbt_burn(nand.page_buffer)) {
        printf("dbbt burn failed!\n");
        return -1;
    } else
        printf("dbbt burn success!\n");

    /* burn firmware */
    if (!rom_nand_firmware_burn((BOOL) is_skip, nand.page_buffer)) {
        printf("firmware burn failed!\n");
        return -1;
    } else
        printf("firmware burn success!\n");

    return 0;
}

/**
  * cmd_nand_test_burn - for validation use, burn a binary image to nand flash
  * @is_skip: determin wether we need to skip 1KB of firmware
  * @badblock1_addr: validation bad block, if less than 0, think as useless input
  * @badblock2_addr: validation bad block, if less than 0, think as useless input
  * @is_disbbm: whether disable bbm swap or not, 1 means disable
  * @ecc_level: if validate bch62, set 62 here, must set even number here
  * @toggle_speed: validation toggle nand, if less than 0, think as useless input
  * @is_fcb1_error: set fcb finger print in 1st block error, to validate fcb in 2nd block
  * @is_no_dbbt: whether there is no dbbt or not
  * @sync_speed: validation sync nand, if less than 0, think as useless input
  * @is_rand_en: whether enable randomizer or not, in pele1.2 and ult, randomizer enable
                 is controlled in FCB, so must fill this item.
  *
  * return 0 if succeed, otherwise return -1
  */
int cmd_nand_test_burn(int is_skip, int badblock1_addr, int badblock2_addr,
                       int is_disbbm, int ecc_level, int toggle_speed, 
					   int is_fcb1_error, int is_no_dbbt, int sync_speed, int is_rand_en)
{
    int block_start;
	int i;

    /* init nand_data struct for searching fcb */
    if (!nand_boot_init_param()) {
        return -1;
    }

    /* burn fcb */
    if (!rom_nand_fcb_test_burn(nand.page_buffer, (BOOL) is_disbbm, ecc_level, 
			toggle_speed, (BOOL) is_fcb1_error, sync_speed, (BOOL) is_rand_en)) {
        printf("FCB burn error!\n");
        //return -1;
    } else
        printf("FCB burn success!\n");

    /* boot nand before other operation, this change is to align with other commands */
    if (!nand_boot_init()) {
        printf("nand boot init failed!\n");
        return -1;
    }

    /* scrub block first, otherwise block will be recongized bad if block has been written because of randomizer */
    block_start = fcb.DBBTSearchAreaStartAddress / fcb.pages_per_block;
    for (i = block_start; i < block_start + 20; i++) {
        if (!nand_erase_block(0, i)) {
            printf("bad block found: %d\n", i);
        }
    }

    /* burn dbbt, mark bad block first if need */
    if ((badblock1_addr >= 0)
        && (!nand_mark_block_bad(0, badblock1_addr))) {
        printf("nand mark block %d bad failed!\n", badblock1_addr);
        return -1;
    }
    if ((badblock2_addr >= 0)
        && (!nand_mark_block_bad(0, badblock2_addr))) {
        printf("nand mark block %d bad failed!\n", badblock2_addr);
        return -1;
    }
	if(!((BOOL) is_no_dbbt)) {
    	if ((!rom_nand_dbbt_burn(nand.page_buffer))) {
        	printf("dbbt burn failed!\n");
        	return -1;
    	} else
        	printf("dbbt burn success!\n");
	}

    /* burn firmware */
    if (!rom_nand_firmware_burn((BOOL) is_skip, nand.page_buffer)) {
        printf("firmware burn failed!\n");
        return -1;
    } else
        printf("firmware burn success!\n");

    return 0;
}

/**
  * cmd_nand_rr_test - test read retry feature
  * @block_addr: the address of block to be erased, so that it can enter read retry 
  * @erase_cnt: the count of erase command
  *
  * return 0 when succeed, otherwise return -1
  */

int cmd_nand_rr_test(int block_addr, int erase_cnt)
{
	U32 pages_per_block = nand.info.pages_per_block;
	U32 page_size = nand.info.data_page_sz;
	U32 page_addr = block_addr * pages_per_block;
	int i, j;

    /* boot nand first */
    if (!nand_boot_init()) {
        printf("nand boot init failed!\n");
        return -1;
    }

	/* check whether the block is bad */
	if (is_block_bad_by_bbm(0, block_addr)) {
		printf("block %d is bad, can't do read retry test!\n", block_addr);
		return -1;
	}

    /* read retry test */
	for (i = 1; i <= erase_cnt; i++) {
		/* print the erase count every 100 times */
		if (i % 100 == 0)
			printf("erase time: %d\n", i);

        /* erase the nand block */
        if (!nand_erase_block(0, block_addr)) {
			printf("erase block %d failed!\n", block_addr);
			return -1;
		}

		/* write and read the first 40 pages, as uboot is always occupy 40 pages */
		for (j = 0; j < 40; j++) {
        	memset(nand.page_buffer, 0x5A, page_size);
			page_addr = block_addr * pages_per_block + j;
        	nand_write_page(0, page_addr, nand.page_buffer);

        	memset(nand.page_buffer, 0, page_size);
			if (!nand_read_page(0, page_addr, nand.page_buffer)) {
				printf("nand read page error, no need to do further read retry test!\n");
				return 0;
			}
		}

	}

	return 0;
}

/* nand command array */
static nand_cmd_t cmd[] = {
    {"info",	cmd_nand_info, 		0, 	"nand info"},
    {"readfcb",	cmd_nand_read_fcb, 	0, 	"nand readfcb"},
    {"readotp",	cmd_nand_read_otp, 	0, 	"nand readotp"},
    {"read", 	cmd_nand_read, 		2, 	"nand read page_start page_cnt"},
    {"dump", 	cmd_nand_dump, 		2, 	"nand dump page_start page_cnt"},
    {"readraw", cmd_nand_read_raw, 	2, 	"nand readraw page_start page_cnt"},
	{"get", 	cmd_nand_get, 		10,
	 "nand get page_start page_cnt total_page_sz is_ecc is_rand meta_sz\n"
	 "data_block_sz ecc_level num_blockn is_GF14\n"
	 "** num_blockn is only num of blockn **"},
    {"erase", 	cmd_nand_erase, 	2, 	"nand erase block_start block_cnt"},
    {"scrub", 	cmd_nand_scrub, 	2, 	"nand scrub block_start block_cnt"},
    {"markbad", cmd_nand_markbad, 	1, 	"nand markbad block_addr"},
    {"rrtest", 	cmd_nand_rr_test, 	2, 	"nand rrtest block_addr, erase_cnt"},
    {"burn", 	cmd_nand_burn, 		1, 	"nand burn is_skip"},
    {"testburn", cmd_nand_test_burn, 10,
     "nand testburn is_skip badblock1_addr badblock2_addr is_disbbm\n" 
	 "ecc_level toggle_speed is_fcb1_error is_no_dbbt sync_speed is_rand_en"},
};

/** 
  * do_nand - nand comand parase
  * @argc: argument numbers
  * @argvs: argument strings array
  *
  * return 0 if succeed, return -1 otherwise
  */
int do_nand(unsigned int argc, char *argvs[])
{
    int i, j;
    void *pfunc;
    int arg[10];

    start_pmccntr();

    //init_clock(32768);	// no epit in 7D, so use gpt instead
	GPT_init();

    read_stop_pmccntr();

    printf("\n");

	/* get the boot configuration by different cpu chips */
	nand_boot_chip_cfg(chip);

    /* search cmd from cmd array if the input name matched */
    for (i = 0; i < ARRAY_SIZE(cmd); i++) {
        /* compare to judge whether name matched */
        if (0 == strncmp(argvs[1], cmd[i].name, strlen(argvs[1]))) {
            /* check whether argc matched */
            if (argc != cmd[i].argc + 2) {
                printf("input argument number don't match \n%s\n", cmd[i].help);
                return -1;
            } else {
                pfunc = cmd[i].pfunc;

                /* check input argument, if not integer, return -1 */
                for (j = 0; j < cmd[i].argc; j++) {
                    if (0 != strtol(argvs[j + 2], &arg[j])) {
                        printf("input argument error \n%s\n", cmd[i].help);
                        return -1;
                    }
                }

                /* call function as the cmd argument */
                switch (cmd[i].argc) {
                case 0:
                    return (*(FUNC_ARGC0) pfunc) ();
                case 1:
                    return (*(FUNC_ARGC1) pfunc) (arg[0]);
                case 2:
                    return (*(FUNC_ARGC2) pfunc) (arg[0], arg[1]);
                case 3:
                    return (*(FUNC_ARGC3) pfunc) (arg[0], arg[1], arg[2]);
                case 4:
                    return (*(FUNC_ARGC4) pfunc) (arg[0], arg[1], arg[2], arg[3]);
                case 5:
                    return (*(FUNC_ARGC5) pfunc) (arg[0], arg[1], arg[2], arg[3], arg[4]);
                case 6:
                    return (*(FUNC_ARGC6) pfunc) (arg[0], arg[1], arg[2], arg[3], arg[4], arg[5]);
                case 7:
                    return (*(FUNC_ARGC7) pfunc) (arg[0], arg[1], arg[2], arg[3], arg[4], arg[5], arg[6]);
                case 8:
                    return (*(FUNC_ARGC8) pfunc) (arg[0], arg[1], arg[2], arg[3], arg[4], arg[5], arg[6], arg[7]);
                case 9:
                    return (*(FUNC_ARGC9) pfunc) (arg[0], arg[1], arg[2], arg[3], arg[4], arg[5], arg[6], arg[7], arg[8]);
                case 10:
                    return (*(FUNC_ARGC10) pfunc) (arg[0], arg[1], arg[2], arg[3], arg[4], arg[5], arg[6], arg[7], arg[8], arg[9]);
                default:
                    printf("input argument number too many\n%s\n", cmd[i].help);
                    return -1;
                }
            }
        }
    }

    printf("command not found\n%s\n", nand_help);
    return -1;
}

SHELL_CMD(nand, do_nand, nand_help)
