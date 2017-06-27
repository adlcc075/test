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

MEMORY 
{
    IRAM (rwx) : ORIGIN = 0x00904000, LENGTH = 0x20000 - 0x4000
}

SECTIONS
{
    .vectors :
    {
        __vectors_start = .;
        *o(.vectors)
        __vectors_end = .;
    }>IRAM

    .text : {
        *(.text)
        . = ALIGN (4);
    }>IRAM

    .shell_cmd :
    {
        . = ALIGN (4);
        __shell_cmds_beginning = .;
        KEEP(*(.shell_cmd))
	__shell_cmds_end = .;
    } >IRAM

    .data : { 
	*(*.*data*.*) 
    }

    .bss : {
        . = ALIGN (4);
        __bss_start__ = .;
        *(.shbss)
        *(.bss .bss.* .gnu.linkonce.b.*)
        *(COMMON)
        __bss_end__ = .;
    } 

    . = ALIGN (4);

    .stack : {
        . += 8K;
    	. = ALIGN (4);
    	top_of_stacks = .;
    } 
    . = ALIGN (4);

    free_memory_start = .;

    _end = .;
}

