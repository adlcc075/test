#ifndef NAND_READ_RETRY_H
#define NAND_READ_RETRY_H

/*==================================================================================================
     Header Name: nand_read_retry.h
     General Description: Provide some defines for nand read retry operations.
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
===================================================================================
                                        INCLUDE FILES
==================================================================================*/
#include "io.h"

/*=================================================================================
                                 LOCAL FUNCTION PROTOTYPES
=================================================================================*/

/*================================================================================
                          Hash Defines And Macros
=================================================================================*/
/****************************************************/
/*              NAND read retry sequence            */
/****************************************************/
#define RR_SEQ_ITEM_HDR(cmd_exist, addr_exist, data_exist, update_cmd_in_tbl, addr_in_tbl, cmd_type) \
			((U8)((cmd_exist) \
			| ((addr_exist) << 1) \
			| ((data_exist) << 2) \
			| ((update_cmd_in_tbl) << 3) \
			| ((addr_in_tbl) << 4) \
			| ((cmd_type) << 5)))
#define RR_SEQ_ITEM_HDR_GET(seq)	((seq_item_hdr_t *)(seq))
#define RR_SEQ_CMD_TYPE_GET(seq) 	(((seq_item_hdr_t *)(seq))->cmd_type)
#define RR_TAG 						(0x82820001)

#define RR_ADDR_REG_NUM_MAX		(18)

typedef enum {
    RR_CMD,                     // Common CMD
    READ_PAGE,                  // Need issue a page read
    READ_PAGE_SUFFIX,           // Need run this sequence item after the page reading
    FINISH,
} rr_cmd_type_e;

/*
 * For a instance
 * 
 */
typedef struct {
    U8 cmd_exist:1;             // Command followed
    U8 addr_exist:1;
    U8 data_exist:1;
    U8 update_cmd_in_tbl:1;     // The CMD to update the RR level was saved in rr_seq_param_t.cmd_update
    U8 addr_in_tbl:1;
    U8 cmd_type:3;

} seq_item_hdr_t;

/*
 * Runtime informations, to easy the coding.
 * 
 */
typedef struct {
    const U8 *head;             // point to the head of read retry sequence table
    const U8 *body;             // point to the body of read retry sequence table
    const U8 *walker;           // Runtime pointer to walk through the RR sequence table
    U32 tag;
    U8 addr_reg_num;
    U8 addr_reg[RR_ADDR_REG_NUM_MAX];   // Buffer to hold the RR regeister address
    U8 update_cmd;
    U32 exit_offset;            // the offset of exit sequence, from beginning of the RR table.
    seq_item_hdr_t *status;
    /* For RR-level-cycling */
    struct {
        U8 level;               // Current level        
        U8 total_level;
        U8 cnt;                 // To hold the times RR had done for one certain page
        const U8 *prev;         // To hold the start of the sub sequence which the previous RR had returned with.
        const U8 *next;         // point to the next start seq, used in nand which needs retry level cycling ???
    } rr_cycle;
} rr_seq_param_t;

/*=================================================================================
                                 Global variables
=================================================================================*/
extern const U8 rr_seq_micron_20nm[];
extern U8 rr_seq_hynix_20nm[];
extern const U8 rr_seq_hynix_26nm[];
extern const U8 rr_seq_toshiba_a19nm[];
extern const U8 rr_seq_toshiba_19nm[];
extern const U8 rr_seq_sandisk_19nm[];
extern const U8 rr_seq_sandisk_1ynm[];

/*=================================================================================
                                 Global FUNCTION PROTOTYPES
=================================================================================*/
BOOL rr_seq_run(U32 cs, rr_cmd_type_e * rr_cmd_type);
BOOL rr_seq_run_page_read_suffix(U32 cs);
void rr_seq_goto_exit(U32 cs);
void rr_seq_reset_cycle_param(void);
void rr_seq_set_seq_entry(void);
BOOL rr_get_rrt_from_nand(U8 * ids, U8 * page_buf);
U32 rr_seq_get_len(U8 ids[]);

#endif /* NAND_READ_RETRY_H */
