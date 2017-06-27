/*
 * Copyright (c) 2013, Freescale Semiconductor, Inc.
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

/*
** This file contains re-implementations of functions whose
** C library implementations rely on semihosting.
** This includes I/O and clocking functionality.
**
** Defining USE_SERIAL_PORT targets the I/O
** to the Integrator AP serial port A.  Otherwise, I/O is targeted
** to the debugger console.
**
** Defining USE_TARGET_CLOCK targets the clocking mechanism to the
** Integrator core module reference clock.  Otherwise, clocking is
** timed off the host system clock.
**
** In an image built to run on a standalone target USE_SERIAL_PORT
** USE_HOST_CLOCK must be defined.
**
*/


/*!
 * @file retarget.c
 * @brief The retargeted functions for the diagnostics environment.
 *
 * @ingroup common
 */

#include "io.h"
#include "../drivers/usb/usb.h"
#include "../drivers/usb/usb_hid_dev.h"

extern void GPT_reset_delay(UINT32 delay);

extern volatile unsigned int usb_attached;
extern usb_module_t usbPort;
report_out_t report_out = { HID_REPORT_OUT_ID, {0} };
report_in_t report_in = { HID_REPORT_IN_ID, {0} };
unsigned int payload_index = 0;

unsigned char usb_getchar(void)
{
    unsigned char ch = 0xFF;
#ifdef USB_PRINT
#	ifdef USB_HID_SET_ERPORT_ON_EP1OUT
    usbdEndpointInfo_t endpoint1Info;
    endpoint1Info.endpointNumber = 1;
    endpoint1Info.endpointDirection = OUT;

    if (1 == usb_attached) {
        usbd_receive_packet(&usbPort, &endpoint1Info, (unsigned char *)&report_out,
                            sizeof(report_out_t));
        if (HID_REPORT_OUT_ID == report_out.ID) {
            ch = report_out.payload[0];
        } else {
            ch = 0xFF;
        }
        return ch;
    } else
#	else
    usbdSetupPacket_t setupPacket;

    if (1 == usb_attached) {
        if (0 == usbd_get_setup_packet(&usbPort, &setupPacket)) {
            if (SET_REPORT == hid_usb_req_handler(&usbPort, &setupPacket)) {
                if (HID_REPORT_OUT_ID == report_out.ID) {
                    ch = report_out.payload[0];
                }
            }
        }
        return ch;
    } else
#	endif
#endif
        return -1;
}

unsigned char usb_putchar(unsigned char *ch)
{
    usbdEndpointInfo_t endpoint1Info;
    endpoint1Info.endpointNumber = 1;
    endpoint1Info.endpointDirection = IN;

    if (1 == usb_attached) {
        report_in.payload[payload_index++] = *ch;
        if (payload_index == (sizeof(report_in_t) - 1)) {
            usbd_send_packet(&usbPort, &endpoint1Info, (unsigned char *)&report_in,
                             sizeof(report_in_t));
            payload_index = 0;
            /* Delay for a while to avoid the host losing usb messages. Just a workaround. */
			GPT_reset_delay(4 << 5);		// 32kHz timer
        }
    }

    return *ch;
}

int usb_flush_std_dev_buf(void)
{
    int i;
    usbdEndpointInfo_t endpoint1Info;
    endpoint1Info.endpointNumber = 1;
    endpoint1Info.endpointDirection = IN;

    if (1 == usb_attached) {
        if (0 != payload_index) {
            for (i = payload_index; i < (sizeof(report_in_t) - 1); i++) {
                report_in.payload[i++] = 0;
            }
            usbd_send_packet(&usbPort, &endpoint1Info, (unsigned char *)&report_in,
                             sizeof(report_in_t));
            payload_index = 0;
        }
    }

    return 0;
}

