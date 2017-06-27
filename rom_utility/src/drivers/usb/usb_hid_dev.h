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

#ifndef __USB_HID_DEV_H__
#define __USB_HID_DEV_H__

#include "io.h"
#include "soc_info.h"

#define k_hid_usage_collection_application	\
	  	0xa1, 0x01
#define k_hid_usage_end_collection	\
	  	0xc0

#define _hid_usage_hidtc_data_out(__id, __count, __size)	\
	  	0x85, ((U8)(__id)),	/*   REPORT_ID (__id) */	\
	0x19, 0x01,			/*   USAGE_MINIMUM (1)*/	\
	0x29, ((U8)(__size)),			/*   USAGE_MAXIMUM (1)*/	\
	0x15, 0x00,			/*   LOGICAL_MINIMUM (0)*/	\
	0x26, 0xff, 0x00,	/*   LOGICAL_MAXIMUM (255)*/	\
	0x75, ((U8)(__size)),/*   REPORT_SIZE (n)*/	\
	0x95, ((U8)(__count)),/*   REPORT_COUNT (n)*/	\
	0x91, 0x02                  /*   OUTPUT (Data,Var,Abs) */

/* hidtc data buffer in report descriptor */
#define _hid_usage_hidtc_data_in(__id, __count, __size)		\
	  	0x85, ((U8)(__id)),	/*   REPORT_ID (__id) */	\
	0x19, 0x01,			/*   USAGE_MINIMUM (1)*/	\
	0x29, ((U8)(__size)),			/*   USAGE_MAXIMUM (1)*/	\
	0x15, 0x00,			/*   LOGICAL_MINIMUM (0)*/	\
	0x26, 0xff, 0x00,		/*   LOGICAL_MAXIMUM (255)*/	\
	0x75, ((U8)(__size)),	/*   REPORT_SIZE (n)*/	\
	0x95, ((U8)(__count)),	/*   REPORT_COUNT (n)*/	\
	0x81, 0x02                  /*   INPUT (Data,Var,Abs) */

#define HID_REPORT_IN_ID    1
#define HID_REPORT_OUT_ID   2
#define HID_REPORT_IN_LEN   128
#define HID_REPORT_OUT_LEN  1

#define HID_MX6X_HANDSHAKE	0xA5
#define USB_INIT_TIMEOUT_MS		1
 
typedef struct {
	cpu_type_e cpuid;
	uint16_t idProduct;
} usb_cfg_t; 

typedef struct {
    usbConfigurationDescriptor_t cfgDescriptorHeader;
    usbInterfaceDescriptor_t interfaceDescriptor;
    usbHidDescriptor_t HidDescriptor;
    usbEndpointDescriptor_t EndpointDescriptor1;
#ifdef USB_HID_SET_ERPORT_ON_EP1OUT
    usbEndpointDescriptor_t EndpointDescriptor2;
#endif
} __attribute__ ((__packed__)) usbMouseCfgDescriptor_t, *usbMouseCfgDescriptor_p;

typedef struct {
    unsigned char ID;
    unsigned char payload[HID_REPORT_OUT_LEN];
} __attribute__ ((__packed__)) report_out_t, *report_out_p;

typedef struct {
    unsigned char ID;
    unsigned char payload[HID_REPORT_IN_LEN];
} __attribute__ ((__packed__)) report_in_t, *report_in_p;

/* global functions */
void usb_hid_device_init(usb_module_t * port);
void hid_receive_report(usb_module_t * port, report_out_p report);
int hid_usb_req_handler(usb_module_t * port, usbdSetupPacket_t * setupPacket);
void hid_device_enum(usb_module_t * port);

#endif //__USB_HID_DEV_H__
