/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _PLAT_STARTUP_H_
#define _PLAT_STARTUP_H_


/***********************************/
#define CPU_2_BE_32(l) 			\
       ((((l) & 0x000000FF) << 24) | 	\
	(((l) & 0x0000FF00) << 8)  | 	\
	(((l) & 0x00FF0000) >> 8)  | 	\
	(((l) & 0xFF000000) >> 24))          
                                       
#define DCDGEN(i, addr, val)   	\
	dcd_node_##i:			\
        .word CPU_2_BE_32(addr) ;     	\
        .word CPU_2_BE_32(val)  ;     	

#define DCD_HDR_LEN_FLD(l)	((((((l)*8 + 4 + 4) & 0x00FF) << 8) |	\
		 ((((l)*8 + 4 + 4) & 0xFF00) >> 8))	<< 8 )

#define DCD_CMD_LEN_FLD(l)	((((((l)*8 + 4) & 0x00FF) << 8) |	\
		 ((((l)*8 + 4) & 0xFF00) >> 8))	<< 8 )
/************************************/

#define IMAGE_START_ADDR	0x907000
#define USR_DCD_NUM		16

#define PLATFORM_INIT           plat_dcd_startup
.macro plat_dcd_startup
#ifdef PLAT_CONFIG_BOOT_FROM_SD
    b startup
    .org 0x400
#endif

ivt_header:             .long 0x402000D1    //Tag=0xD1, Len=0x0020, Ver=0x40
app_code_jump_v:        .long startup       // Entry point for the bootloader
reserv1:                .long 0x0
#ifdef PLAT_CONFIG_BOOT_FROM_SD
	dcd_ptr:                .long 0x0090742c
	boot_data_ptr:          .long 0x00907420
	self_ptr:               .long 0x00907400
#else
	dcd_ptr:               	.long 0x0
	boot_data_ptr:         	.long boot_data
	self_ptr:              	.long ivt_header
#endif
app_code_csf:           .long 0x0 			// reserve 4K for csf
reserv2:                .long 0x0

boot_data:              .long  __image_start__
image_len:              .long __image_size__
plugin:                 .long 0x0


	.global usr_dcd_base
	.global usr_dcd_end
usr_dcd_base:
	.rept USR_DCD_NUM
		.long 0x0
		.long 0x0
	.endr
usr_dcd_end:

#ifdef PLAT_CONFIG_BOOT_FROM_SD
dcd_hdr:		.long ((0x40 << 24) | DCD_HDR_LEN_FLD(98) | 0xD2)	/* Tag=0xD2, Len=128*8 + 4 + 4 = 0x0408, Ver=0x40 */
dcd_cmd:		.long ((0x04 << 24) | DCD_CMD_LEN_FLD(98) | 0xCC) 	/* Tag=0xCC, Len=128*8 + 4 = 0x0404 , Param=4 */

	/* Add DCD here if the SD boot was wanted */

#endif
.endm

#endif
