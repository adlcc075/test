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
 * @file usb_common.c
 * @brief USB common routines.
 */
/*! This routine returns the current speed of the port\n
 * This must be called after the bus reset is completed.
 *
 * @param	port			The usb controller
 * @return	usbPortSpeed	The operating speed full/high
 */

#include "usb.h"
#include <stdio.h>

/* get the port speed, low, full or high speed
   USB_PORTSC: port status & control
   PSPD: port speed bit
 */
usbPortSpeed_t usb_get_port_speed(usb_module_t * port)
{
    usbPortSpeed_t speed;

    /* Determine the speed we are connected at. */
    switch (port->moduleBaseAddress->USB_PORTSC & USB_PORTSC_PSPD(0x3)) {
    case USB_PORTSC_PSPD_FULL:
        speed = usbSpeedFull;
        printf("Connected at full-speed\n");
        break;
    case USB_PORTSC_PSPD_LOW:
        speed = usbSpeedLow;
        printf("Connected at low-speed\n");
        break;
    case USB_PORTSC_PSPD_HIGH:
        speed = usbSpeedHigh;
        printf("Connected at high-speed\n");
        break;
    default:
        speed = usbSpeedUnknown;
        printf("Invalid port speed\n");
        break;
    }
    printf("PORTSC = 0x%08x\n", port->moduleBaseAddress->USB_PORTSC);

    return speed;
}
