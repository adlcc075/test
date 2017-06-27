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

#ifndef _USB_H
#define _USB_H

#include "usb_defines.h"
#include "usb_registers.h"

#define DEBUG_ENABLE	0
#if DEBUG_ENABLE
#define USB_TRACE(fmt, args...)	printf(fmt,##args)
#else
#define USB_TRACE(fmt, args...)
#endif

enum usbController {
    OTG = 0,
    Host1,
    Host2,
    Host3
};

enum usbPhyType {
    Utmi,
    Ulpi,
    Serial,
    Hsic
};

typedef enum usbPortSpeed {
    usbSpeedUnknown,
    usbSpeedLow,
    usbSpeedFull,
    usbSpeedHigh
} usbPortSpeed_t;

typedef enum usbPeriodicListType {
    listTypeIsochronous = 0,
    listTypeQueueHead,
    listTypeSplitTransaction,
    listTypeFrameSpan
} usbPeriodicListType_t;

typedef struct usb_module {
    int8_t *moduleName;
    usbRegisters_t *moduleBaseAddress;
    enum usbController controllerID;
    enum usbPhyType phyType;    // UTMI, ULPI, Serial, HSIC
} usb_module_t;

/*!
 *  usb host data structure definitions
 */

/* USB host queue head structure */
typedef struct usbhQueueHead {
    uint32_t queueHeadLinkPointer;
    uint32_t endpointCharacteristics;
    uint32_t endpointCapabilities;
    uint32_t currentQtd;
    uint32_t nextQtd;
    uint32_t alternateNextQtd;
    uint32_t qtdToken;
    uint32_t qtdBuffer[5];
    uint32_t mallocPointer;     /* Used to keep track of the
                                   memory block allocated for the data structure */
} usbhQueueHead_t;

/*!
 *  USB host queue element transfer descriptor
 */
typedef struct usbhTransferDescriptor {
    uint32_t nextQtd;
    uint32_t alternateNextQtd;
    uint32_t qtdToken;
    uint32_t qtdBuffer[5];
    uint32_t mallocPointer;     /* Used to keep track of the unaligned
                                   memory allocated for the data structure */

} usbhTransferDescriptor_t;

/*!
 * Protocol descriptor definitions
 */

typedef struct usbDeviceDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t iManufacturer;
    uint8_t iProduct;
    uint8_t iSerialNumber;
    uint8_t bNumConfigurations;
} __attribute__ ((__packed__)) usbDeviceDescriptor_t;

typedef struct usbConfigurationDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumInterfaces;
    uint8_t bConfigurationValue;
    uint8_t iConfiguration;
    uint8_t bmAttributes;
    uint8_t MaxPower;
} __attribute__ ((__packed__)) usbConfigurationDescriptor_t;

typedef struct usbInterfaceDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
} __attribute__ ((__packed__)) usbInterfaceDescriptor_t;

typedef struct usbEndpointDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
} __attribute__ ((__packed__)) usbEndpointDescriptor_t;

typedef struct usbHidDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdHID;
    uint8_t bCountryCode;
    uint8_t bNumDescriptors;
    uint8_t bReportDescriptorType;
    uint8_t wDescriptorLength[2];   // !!!! Not aligned on 16-bit boundary  !!!
} __attribute__ ((__packed__)) usbHidDescriptor_t;

/*!
 * Device driver definitions
 */

/*!
 * USB device endpoint list
 */

typedef uint32_t endpointList_t[8];

/*
 * USB device setup packet
 */

typedef struct usbdSetupPacket {
    uint8_t bRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} __attribute__ ((__packed__)) usbdSetupPacket_t;

/*!
 *  USB device endpoint queue head structure
 */

typedef struct usbdDeviceEndpointQueueHead {
    uint32_t endpointCharacteristics;
    uint32_t currentDtd;
    uint32_t nextDtd;
    uint32_t dtdToken;
    uint32_t dtdBuffer[5];
    uint32_t reserved;
    usbdSetupPacket_t setupBuffer;
    uint32_t reserved1[4];
} usbdDeviceEndpointQueueHead_t;

typedef struct usbdEndpointPair {
    usbdDeviceEndpointQueueHead_t out;
    usbdDeviceEndpointQueueHead_t in;
} usbdEndpointPair_t;

/*
 *  USB device endpoint transfer descriptor
 */
typedef struct usbdEndpointDtd {
    uint32_t nextDtd;
    uint32_t dtdToken;
    uint32_t dtdBuffer[5];
    uint32_t mallocPointer;     /* Used to keep track of the memory allocated for the data structure */
} usbdEndpointDtd_t;

/*!
 * Endpoint characteristics
 */
typedef enum usbdEndpointDirection {
    IN,
    OUT
} usbdEndpointDirection_e;

typedef enum {
    Ep0,
    Ep1,
    Ep2,
    Ep3,
    Ep4,
    Ep5,
    Ep6,
    Ep7,
} usbEndpointPair_e;

/* EP info: speed, number, direction, mult, max packet len, interrupt */
typedef struct usbdEndpointInfo {
    usbPortSpeed_t Speed;
    uint32_t endpointNumber;
    usbdEndpointDirection_e endpointDirection;
    uint32_t mult;
    uint32_t maxPacketLength;
    bool interruptOnSetup;
} usbdEndpointInfo_t;

/*!
 * Definitions for Setup packet
 */

/* Defines for commands in setup packets */
#define GET_DESCRIPTOR				0x8006
#define SET_CONFIGURATION			0x0009
#define SET_IDLE					0x210A
#define GET_HID_CLASS_DESCRIPTOR	0x8106
#define SET_FEATURE					0x0003
#define CLEAR_FEATURE				0x0201
#define SET_ADDRESS					0x0005
#define SET_REPORT					0x2109
#define GET_REPORT					0xA101

#define USB_HID_CLASS_REQUEST_GET_REPORT    (0x01)
#define USB_HID_CLASS_REQUEST_GET_IDLE      (0x02)
#define USB_HID_CLASS_REQUEST_GET_PROTOCOL  (0x03)
#define USB_HID_CLASS_REQUEST_SET_REPORT    (0x09)
#define USB_HID_CLASS_REQUEST_SET_IDLE      (0x0A)
#define USB_HID_CLASS_REQUEST_SET_PROTOCOL  (0x0B)

#include "usbd_config.h"

typedef struct {
    usbdEndpointPair_t EndpointPairs[EndpointPair_Num];		// in out EP queue head

    usbdEndpointDtd_t EndpointDtds[EndpointPair_Num][2];	// in out EP dtd(device transaction descriptor)

    unsigned char Ep0In_Buffer[Ep0In_Buffer_Size];			// EP0 in buffer, control trans
    unsigned char Ep0Out_Buffer[Ep0Out_Buffer_Size];		// EP0 out buffer, control trans

#if Ep1In_Buffer_Size
    unsigned char Ep1In_Buffer[Ep1In_Buffer_Size];			// EP1 in buffer, 0x200, bulk trans
#endif
#if Ep1Out_Buffer_Size
    unsigned char Ep1Out_Buffer[Ep1Out_Buffer_Size];
#endif

#if Ep2In_Buffer_Size
    unsigned char Ep2In_Buffer[Ep2In_Buffer_Size];
#endif
#if Ep2Out_Buffer_Size
    unsigned char Ep2Out_Buffer[Ep2Out_Buffer_Size];
#endif

    /* Add buffers here is more endpoint used.  */

} usbMemZone_t, *usbMemZone_p;

typedef enum {
    USB_DEV_NULL,
    USB_DEV_RESETED,
    USB_DEV_ADDRESSED,
    USB_DEV_ENUMMURATED,
} usb_dev_status_e;

//! device driver function prototypes
uint32_t usbd_device_init(usb_module_t * port);
void usbd_endpoint_qh_init(usbdEndpointInfo_t * usbdEndpoint, uint32_t nextDtd);

usbPortSpeed_t usbd_bus_reset(usb_module_t * port);

int usbd_get_setup_packet(usb_module_t * port, usbdSetupPacket_t * setupPacket);
void usbd_device_send_control_packet(usb_module_t * port, uint8_t * buffer, uint32_t size);
void usbd_device_send_zero_len_packet(usb_module_t * port, uint32_t endpoint);
int usbd_send_packet(usb_module_t * port, usbdEndpointInfo_t * endpoint, uint8_t * buffer,
                     uint32_t size);

//! common prototypes

usbPortSpeed_t usb_get_port_speed(usb_module_t * port);
int usbEnableClocks(usb_module_t * port);
int usbEnableTransceiver(usb_module_t * port);

#define DTD_TERMINATE 0xDEAD001
#define QTD_TERMINATE 0xDEAD001
#define OUT_PID  0
#define IN_PID  1
#define SETUP_PID  2

#define EPS_FULL 0
#define EPS_LOW	1
#define EPS_HIGH 2

#define MAX_USB_BUFFER_SIZE 0x100
#define MAX_USB_DESC_SIZE 	0x40

#define MAX_QTD_TRANS_SIZE	0x5000
#define MAX_DTD_TRANS_SIZE	0x5000
#define MAX_QH_PACKET_SIZE	0x400

#endif /* _USB_H */
