/*
 * Copyright (C) 2011, Freescale Semiconductor, Inc. All Rights Reserved
 * THIS SOURCE CODE IS CONFIDENTIAL AND PROPRIETARY AND MAY NOT
 * BE USED OR DISTRIBUTED WITHOUT THE WRITTEN PERMISSION OF
 * Freescale Semiconductor, Inc.
*/

/*********************************************************
** WARNING: This code comes with no guarantees or warranties
**
** This is an internal Freescale test tool under debug and 
** development 
**
** It was tested on internal Freescale development boards but may 
** not  work on other customer or third party boards
**
** This code comes as is with no support.
**
** USE AT YOUR OWN RISK! 
**
** Freescale is absolved or any legal or support obligations for those 
** who ignore this warning
*********************************************************/


OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(Reset_Handler)
/* SEARCH_DIR(.) */
/* GROUP(-lgcc -lc -lcs3 -lcs3arm) */
GROUP(-lgcc -lc)

MEMORY {
    IRAM2 (rwx) : ORIGIN = 0x00910000, LENGTH = 0x00940000-0x00910000
    IRAM1 (rwx) : ORIGIN = 0x00904000, LENGTH = 0x00910000-0x00904000
}

PROVIDE(__cs3_heap_start = _end);

SECTIONS
{
    .vectors :
    {
        __vectors_start = .;
		__image_start__ = .;
        *o(.vectors)
        __vectors_end = .;
        . = ALIGN (4);
    }>IRAM2

    .text : {
        *(.text)
        . = ALIGN (4);
    }>IRAM2

    .shell_cmd :
    {
        . = ALIGN (4);
        __shell_cmds_beginning = .;
        KEEP(*(.shell_cmd))
	__shell_cmds_end = .;
    } >IRAM2

    .data : { 
	*(*.*data*.*) 
    } >IRAM2

    .bss : {
        . = ALIGN (4);
        __bss_start__ = .;
        *(.shbss)
        *(.bss .bss.* .gnu.linkonce.b.*)
        *(COMMON)
        __bss_end__ = .;
    } >IRAM2

    . = ALIGN (4);
    _end = .;
    __end = _end;
    __image_end__ = _end;
    __image_size__ = __image_end__ - __image_start__;
    PROVIDE(end = .);

    . = ALIGN (4);
    . += 8;
    free_memory_start = .;

    .stack : {
        . += 8K;
    	. = ALIGN (4);
    	top_of_stacks = .;
    } >IRAM1

   .usb_buffer :{
	. = ALIGN (4K);
	__usb_buffer_base__ = .;
	. += 2K;
    } > IRAM1
}

