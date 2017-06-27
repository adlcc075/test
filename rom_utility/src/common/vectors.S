/*
 * Copyright (c) 2012-2013, Freescale Semiconductor, Inc.
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

/*!
 * @file vectors.s
 * @brief This file contains the exception vectors
 *
 * @ingroup common
 */

/*        PRESERVE8 */

#ifdef USB_PRINT
#include "platform_startup.inc"

#ifdef PLATFORM_INIT
		PLATFORM_INIT
#endif

#endif

    .text
    .code 32

/* A vector table and dummy exception handlers


; *****************
; Exception Vectors
; *****************

; Note: LDR PC instructions are used here, though branch (B) instructions
; could also be used, unless the ROM is at an address >32MB.
*/
/*	.global startup*/
	.func startup

startup:
        LDR     PC, Reset_Addr
        LDR     PC, Undefined_Addr
        LDR     PC, SWI_Addr
        LDR     PC, Prefetch_Addr
        LDR     PC, Abort_Addr
        NOP                             /* Reserved vector */
        LDR     PC, IRQ_Addr
        LDR     PC, FIQ_Addr

Reset_Addr:      .int     Reset_Handler
Undefined_Addr:  .int     Undefined_Handler
SWI_Addr:        .int     SWI_Handler
Prefetch_Addr:   .int     Prefetch_Handler
Abort_Addr:      .int     Abort_Handler
IRQ_Addr:        .int     IRQ_Handler
FIQ_Addr:        .int     FIQ_Handler


/************************
; Exception Handlers
; ************************

; The following dummy handlers do not do anything useful in this example.
; They are set up here for completeness. */

Undefined_Handler:
        B       Undefined_Handler
SWI_Handler:
        B       SWI_Handler
Prefetch_Handler:
        B       Prefetch_Handler
Abort_Handler:
        B       Abort_Handler
IRQ_Handler:
        B       IRQ_Handler
FIQ_Handler:
        B       FIQ_Handler

	.endfunc

    .end



