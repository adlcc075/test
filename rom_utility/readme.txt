/******************************************************************************************************************
************************************* How to compile the elf image ************************************************
******************************************************************************************************************/

The elf file is output/mx6x/bin/rom_utility_mx6x.elf.
You may need to load this elf into Lauterbach to run.
USB print also supported in this version.


/******************************************************************************************************************
********************************* How to use ROM utility for nand flash *******************************************
******************************************************************************************************************/

Press F7 to run in TRACE32 after elf has been load and enter nand commands.
If you just input "nand" and then press enter button, all the nand commands details will display.
If you just input "nand" and the command name, for example "nand read", the corresponding command
usage will display.

Take note that before using the commands read dump and rrtest, FCB must exist and read correctly from the nand flash.
We will modify this soon to make these commands not depend on FCB.

All the micron nand which have parameter page can be supported, we get nand information from there. 
For Samsung, Toshiba, Sandisk and hynix, we form a table in src/drivers/raw_nand/rom_nand_cfg_table.c.
As the limit size of OCRAM, only a few nands have been put into that table and can be supported. 
If you get a nand flash can't be supported, you can add it into the table by yourself.

below is nand command details:
    "   burn    burn a binary image to nand flash\n"
    "           is_skip: determin wether we need to skip 1KB of firmware\n"
    "   info    print nand informations, include nand id, manufacture, page size etc\n"
    "   readfcb    read fcb page and dump all the fcb informations.\n"
	"              This is very useful when debug nand boot fail issues.\n"
    "   readotp    read hynix nand otp out, mainly for read retry sequence.\n"
    "   read    read some continuous nand pages and dump them, oob excluded, FCB must exist\n"
    "           page_start: start page\n"
    "           page_cnt: page numbers to dump\n"
    "   readraw    read some continuous nand pages and dump them, ECC and randomizer disabled\n"
    "           page_start: start page\n"
    "           page_cnt: page numbers to dump\n"
    "   dump    read some continuous nand pages and dump them, oob included, FCB must exist\n"
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
    "   rrtest  test read retry feature, FCB must exist\n"
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
    "           is_rand_en: whether enable randomizer or not, in pele1.2 and ult, randomizer enable\n"
    "                       is controlled in FCB, so must fill this item.\n";

/******************************************************************************************************************
*********************************** How to burn image into nand ***********************************************
******************************************************************************************************************/

Some note you must know:
1. Lauterbach and JTAG have been used to load elf file into OCRAM. You can use a script to do that.
2. Mx6 family chips can be supported.

Steps to burn image:
1. Connect lauterbach and board using JTAG, attach and load elf using the script.
2. Press F7 to run, and input command "nand burn 0". Then FCB parameters are displayed, and FCB, DBBT, firmware can be 
   burn into nand.

Some problems that may occur:
1. As your chip may run failed and can't attach, you can first attach without nand, and then put it into nand socket,
   then burn images
2. If an image already contains the 1k data before IVT, you must enter "nand burn 1", else just enter "nand burn 0".
   Almost all the uboot image doesn't contain the 1k data, so you can just enter "nand burn 0"
   For some ROM team internal test image, this 1k data may exsit.
3. As DDR can be only initialize once (seemed our IC bug), if DDR intialized in DCD, then initialized by Lauterbach script,
   bus will hang. You can also first attach without nand, and then put it into nand socket, then burn images.

/******************************************************************************************************************
********************************* How to use ROM utility for fuse *******************************************
******************************************************************************************************************/
1. commands as below
	"         ocotp sense index"
	"         ocotp blow index value"
	"         ocotp blowbank bank word0 word1 word2 word3"
	"         For fuse below 0x800 in fuse map, the index = (fuse_addr - 0x400)/0x10"
	"         For fuse above 0x800, the index = (fuse_addr - 0x400)/0x10 - 0x10"

/******************************************************************************************************************
********************************* How to use ROM utility for log parser *******************************************
******************************************************************************************************************/
1. commands as below
	"        log_parse" 

/******************************************************************************************************************
********************************* How to use USB print *******************************************
******************************************************************************************************************/
1. we have developed usb print function to make it more easy for us to use, especially for customers who don't have a Lauterbach.
2. you must connect usb cable and open the host side tool in the PC.
2. to enable usb print, you must add "usb_print=1" when compiling ROM utility. 
   for example, "make clean && make TARGET=mx7x usb_print=1"

If any other problems occur, please contact Terry Xie <B47714@freescale.com>
