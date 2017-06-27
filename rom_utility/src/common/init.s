/*
 * Copyright (C) 2012, Freescale Semiconductor, Inc. All Rights Reserved
 * THIS SOURCE CODE IS CONFIDENTIAL AND PROPRIETARY AND MAY NOT
 * BE USED OR DISTRIBUTED WITHOUT THE WRITTEN PERMISSION OF
 * Freescale Semiconductor, Inc.
*/

@//*********************************************************
@//** WARNING: This code comes with no guarantees or warranties
@//**
@//** This is an internal Freescale test tool under debug and 
@//** development 
@//**
@//** It was tested on internal Freescale development boards but may 
@//** not  work on other customer or third party boards
@//**
@//** This code comes as is with no support.
@//**
@//** USE AT YOUR OWN RISK! 
@//**
@//** Freescale is absolved or any legal or support obligations for those 
@//** who ignore this warning
@//*********************************************************


/* --- Standard definitions of mode bits and interrupt (I & F) flags in PSRs */

    .equ    Mode_USR,     0x10
    .equ    Mode_FIQ,     0x11
    .equ    Mode_IRQ,     0x12
    .equ    Mode_SVC,     0x13
    .equ    Mode_ABT,     0x17
    .equ    Mode_UND,     0x1B
    .equ    Mode_SYS,     0x1F /* available on ARM Arch 4 and later */

    .equ    I_Bit,      0x80 /* when I bit is set, IRQ is disabled */
    .equ    F_Bit,      0x40 /* when F bit is set, FIQ is disabled */

/* --- Amount of memory (in bytes) allocated for stacks */
    .equ    Len_FIQ_Stack,  1024
    .equ    Len_IRQ_Stack,  1024
    .equ    Len_ABT_Stack,  1024
    .equ    Len_UND_Stack,  1024
    .equ    Len_SVC_Stack,  1024
    .equ    Len_USR_Stack,  1024 

/*  Add lengths >0 for FIQ_Stack, ABT_Stack, UND_Stack if you need them.
 * Offsets will be loaded as immediate values.
 * Offsets must be 8 byte aligned.
*/
    .equ    Offset_FIQ_Stack,   0
    .equ    Offset_IRQ_Stack,   Offset_FIQ_Stack + Len_FIQ_Stack
    .equ    Offset_ABT_Stack,   Offset_IRQ_Stack + Len_IRQ_Stack
    .equ    Offset_UND_Stack,   Offset_ABT_Stack + Len_ABT_Stack
    .equ    Offset_SVC_Stack,   Offset_UND_Stack + Len_UND_Stack
    .equ    Offset_USR_Stack,   Offset_SVC_Stack + Len_SVC_Stack


    .text
    .code 32

    .global Reset_Handler
    .func Reset_Handler

Reset_Handler:
/* TODO: Disable L1/L2 cache and MMU to easy the USB transfer */


/* --- Initialize stack pointer registers */

/* Enter each mode in turn and set up the stack pointer */

    LDR     r0, =top_of_stacks

    MSR     CPSR_c, #Mode_FIQ | I_Bit | F_Bit /* No interrupts*/
    SUB     sp, r0, #Offset_FIQ_Stack

    MSR     CPSR_c, #Mode_IRQ | I_Bit | F_Bit /* No interrupts */
    SUB     sp, r0, #Offset_IRQ_Stack

    MSR     CPSR_c, #Mode_ABT | I_Bit | F_Bit /* No interrupts */
    SUB     sp, r0, #Offset_ABT_Stack

    MSR     CPSR_c, #Mode_SVC | I_Bit | F_Bit /* No interrupts */
    SUB     sp, r0, #Offset_SVC_Stack

 /* Init .bss */
     /* Clear the .bss section (zero init) */
    LDR     r1,=__bss_start__
    LDR     r2,=__bss_end__
    MOV     r3,#0
1:
    CMP     r1,r2
    STMLTIA r1!,{r3}
    BLT     1b


/* Branch to C Library entry point */

    LDR     r12,=main               /* save this in register for possible long jump */

    BX      r12                         /* branch to __main */

    .endfunc

    .end                       /* mark the end of this file */

