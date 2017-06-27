/*
 * Copyright (c) 2012-2014, Freescale Semiconductor, Inc.
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
 * @file usb0_device_mouse_test.c
 * @brief Emulate a USB mouse
 */

#include "usb.h"
#include "usb_hid_dev.h"
#include "drivers/timer/timer.h"
#include "regsusb.h"

extern unsigned int regs_USB_base;
extern char usb_getchar(void);
extern void freq_populate(void);
extern void l1_flush_cache_range(unsigned int start, unsigned int end);
extern int usbd_receive_packet(usb_module_t * port, usbdEndpointInfo_t * endpoint, uint8_t * buffer,uint32_t size);

volatile unsigned int usb_attached = 0;
usb_module_t usbPort;
uint32_t usb_dev_status = USB_DEV_NULL;

extern unsigned int __usb_buffer_base__[];
usbMemZone_p usbMemZone = (usbMemZone_p) __usb_buffer_base__;

/* usb config */
usb_cfg_t usb_cfg[] = {
	{
		.cpuid = CPU_TYPE_MX6DQ,
    	.idProduct = 0x0054,        
	},
	{
		.cpuid = CPU_TYPE_MX6SDL,
    	.idProduct = 0x0061,        
	},
	{
		.cpuid = CPU_TYPE_MX6SL,
    	.idProduct = 0x0063,        
	},
	{
		.cpuid = CPU_TYPE_MX6SX,
    	.idProduct = 0x0071,        
	},
	{
		.cpuid = CPU_TYPE_MX7D,
    	.idProduct = 0x0076,        
	},
	{
		.cpuid = CPU_TYPE_MX6UL,
    	.idProduct = 0x007D,        
	},
	{
		.cpuid = CPU_TYPE_MX6ULL,
		.idProduct = 0x007D,        
	},
	{
		.cpuid = CPU_TYPE_MX6SLL,
		.idProduct = 0x0063,  //Not sure       
	},	
};

/* usb device descriptor */
usbDeviceDescriptor_t mouseDeviceDescriptor = {
    .bLength = 0x12,            // length of this descriptor
    .bDescriptorType = 0x01,    // Device descriptor
    .bcdUSB = 0x0200,           // USB version 2.0
    .bDeviceClass = 0x00,       // Device class (specified in interface descriptor)
    .bDeviceSubClass = 0x00,    // Device Subclass (specified in interface descriptor)
    .bDeviceProtocol = 0x00,    // Device Protocol (specified in interface descriptor)
    .bMaxPacketSize = 0x40,     // Max packet size for control endpoint
    .idVendor = 0x15a2,         // Freescale Vendor ID. -- DO NOT USE IN A PRODUCT
    .idProduct = 0x0,           // will be modified when usb init
    .bcdDevice = 0x0001,        // Device revsion
    .iManufacturer = 0x00,      // Index of  Manufacturer string descriptor
    .iProduct = 0x00,           // Index of Product string descriptor
    .iSerialNumber = 0x00,      // Index of serial number string descriptor
    .bNumConfigurations = 0x01, // Number of configurations
};

uint8_t hidReportDescriptor[] = {
    0x06, 0xff, 0xff,           /* USAGE_PAGE (Vendor Defined Page 1) */
    0x09, 0x01,                 /* USAGE (Vendor 1) */
    k_hid_usage_collection_application,
    _hid_usage_hidtc_data_in(HID_REPORT_IN_ID, HID_REPORT_IN_LEN, 8),
    _hid_usage_hidtc_data_out(HID_REPORT_OUT_ID, HID_REPORT_OUT_LEN, 8),
    k_hid_usage_end_collection,
};

usbMouseCfgDescriptor_t usbMouseCfgDescriptor = {
    .cfgDescriptorHeader = {
                            .bLength = 0x09,
                            .bDescriptorType = 0x02,    // Configuration descriptor
                            .wTotalLength = sizeof(usbMouseCfgDescriptor_t),    // Total length of data, includes interface, HID and endpoint
                            .bNumInterfaces = 0x01, // Number of interfaces
                            .bConfigurationValue = 0x01,    // Number to select for this configuration
                            .iConfiguration = 0x00, // No string descriptor
                            .bmAttributes = 0xC0,   // Self powered, No remote wakeup
                            .MaxPower = 10, // 20 mA Vbus power
                            }
    ,
    .interfaceDescriptor = {
                            .bLength = 0x09,
                            .bDescriptorType = 0x04,    // Interface descriptor
                            .bInterfaceNumber = 0x00,   // This interface = #0
                            .bAlternateSetting = 0x00,  // Alternate setting
                            .bNumEndpoints = 0x01,  // Number of endpoints for this interface
                            .bInterfaceClass = 0x03,    // HID class interface
                            .bInterfaceSubClass = 0x00, // bInterfaceSubClass : 1=BOOT, 0=no boot
                            .bInterfaceProtocol = 0x00,
                            .iInterface = 0,    // No string descriptor
                            }
    ,
    .HidDescriptor = {
                      .bLength = 0x09,  //
                      .bDescriptorType = 0x21,  // HID descriptor
                      .bcdHID = 0x0101, // HID Class spec 1.01
                      .bCountryCode = 0x00, //
                      .bNumDescriptors = 0x01,  // 1 HID class descriptor to follow (report)
                      .bReportDescriptorType = 0x22,    // Report descriptor follows
                      .wDescriptorLength[0] = sizeof(hidReportDescriptor),  // Length of report descriptor byte 1
                      .wDescriptorLength[1] = 0x00, // Length of report descriptor byte 2
                      }
    ,
    .EndpointDescriptor1 = {
                            .bLength = 0x07,
                            .bDescriptorType = 0x05,    // Endpoint descriptor
                            .bEndpointAddress = 0x81,   // Endpoint 1 IN
                            .bmAttributes = 0x3,    // 0 for control, 1 for iso, 2 for bulk, and 3 for interrupt endpoint
                            .wMaxPacketSize = 0x200,
                            .bInterval = 0x14,  //20ms      
                            }
    ,

#ifdef USB_HID_SET_ERPORT_ON_EP1OUT
    .EndpointDescriptor2 = {
                            .bLength = 0x07,
                            .bDescriptorType = 0x05,    // Endpoint descriptor
                            .bEndpointAddress = 0x01,   // Endpoint 1 OUT
                            .bmAttributes = 0x3,    // interrupt endpoint
                            .wMaxPacketSize = 0x0200,   // max 6 bytes (for high_speed)
                            .bInterval = 0x08,  // 10 ms interval
                            }
    ,
#endif
};

/*!
 * Main program loop.
 */
void usb_hid_device_init(usb_module_t * port)
{
    // Detach the device.
    port->moduleBaseAddress->USB_USBCMD &= (~USB_USBCMD_RS);	// stop
    GPT_reset_delay(100 << 5);

	/* ep0 in & out QH setting, related regs setting*/
    usbd_device_init(port);	

    printf("\nWaiting for reset to start enumeration.\n");
    while (!(port->moduleBaseAddress->USB_USBSTS & USB_USBSTS_URI)) ;

    /*
     *  Handle USB bus reset.
     *  If bus reset, EPx except EP0 will be disabled automatically.
     *  Host maybe reset bus if something bad happens.
     */
    usbd_bus_reset(port);

    hid_device_enum(port);

    port->moduleBaseAddress->USB_ENDPTCTRL[1] = 0 | USB_ENDPTCTRL_TXE | USB_ENDPTCTRL_TXT_BULK
#ifdef USB_HID_SET_ERPORT_ON_EP1OUT
        | USB_ENDPTCTRL_RXT_BULK | USB_ENDPTCTRL_RXE |
#endif
        ;

    //! - Initialize IN endpoint queue head for endpoint 1
    usbdEndpointInfo_t endpoint1Info;
    endpoint1Info.endpointNumber = 1;
    endpoint1Info.endpointDirection = IN;
    endpoint1Info.interruptOnSetup = false;
    endpoint1Info.maxPacketLength = usb_get_port_speed(port) == usbSpeedFull ? 64 : 512;
    endpoint1Info.mult = 0;
    usbd_endpoint_qh_init(&endpoint1Info, 0xDEAD0001);

#ifdef USB_HID_SET_ERPORT_ON_EP1OUT
	usbdEndpointInfo_t endpoint2Info;
    endpoint2Info.endpointNumber = 1;
    endpoint2Info.endpointDirection = OUT;
    endpoint2Info.interruptOnSetup = false;
    endpoint2Info.maxPacketLength = usb_get_port_speed(port) == usbSpeedFull ? 64 : 512;
    endpoint2Info.mult = 0;
    usbd_endpoint_qh_init(&endpoint2Info, 0xDEAD0001);
#endif

#ifdef PLAT_CONFIG_USB_BUFFER_IN_IRAM
    /* Flush L1-D and L2 cache */
    l1_flush_cache_range((unsigned int)usbMemZone, (unsigned int)usbMemZone + sizeof(usbMemZone_t));
    l2_flush_cache_range((unsigned int)usbMemZone, (unsigned int)usbMemZone + sizeof(usbMemZone_t));
#endif

#if 0
    report_in_t report_in = { HID_REPORT_IN_ID, 0 };
    while (1) {
        /*
           usbd_receive_packet(port, &endpoint2Info, (unsigned char *)&report_out,
           sizeof(report_out_t));
           if (report_out.ID == HID_REPORT_OUT_ID) {
           printf("Report received: 0x%02x", report_out.payload);
           } */
        report_in.payload[0] = 'a';
        report_in.payload[7] = 'b';
        usbd_send_packet(port, &endpoint1Info, (unsigned char *)&report_in, sizeof(report_in_t));
    }
#endif
}

void hid_receive_report(usb_module_t * port, report_out_p report)
{
    usbdEndpointInfo_t endpoint0Out;

    endpoint0Out.endpointNumber = 0;
    endpoint0Out.endpointDirection = OUT;
    endpoint0Out.maxPacketLength = 0x40;
    endpoint0Out.mult = 0;
    endpoint0Out.interruptOnSetup = false;

    usbd_receive_packet(port, &endpoint0Out, (uint8_t *) report, sizeof(report_out_t));

    usbd_device_send_zero_len_packet(port, 0);
}

extern report_out_t report_out;
extern report_in_t report_in;

/*
 * Note: Since the hid_usb_req_handler will be called in the usb_getchar(), 
 * do not enable the USB_TRACE. Otherwise, this function will be recursed.
 */

int hid_usb_req_handler(usb_module_t * port, usbdSetupPacket_t * setupPacket)
{
    uint32_t request;
    uint32_t transferSize;
    uint32_t deviceAddress;
    uint32_t testMode;

    request = setupPacket->bRequestType << 8 | setupPacket->bRequest;

    switch (request) {
    case GET_DESCRIPTOR:
        if (setupPacket->wValue == 0x100) { //! - send device descriptor.
            transferSize = setupPacket->wLength > 0x12 ? 0x12 : setupPacket->wLength;
            usbd_device_send_control_packet(port, (uint8_t *) (&mouseDeviceDescriptor),
                                            transferSize);

            USB_TRACE("Device descriptor sent %d bytes.\n", transferSize);
        } else if (setupPacket->wValue == 0x0200) { //! - send configuration descriptor.
            transferSize = setupPacket->wLength > sizeof(usbMouseCfgDescriptor) ?
                sizeof(usbMouseCfgDescriptor) : setupPacket->wLength;
            usbd_device_send_control_packet(port, (uint8_t *) (&usbMouseCfgDescriptor),
                                            transferSize);

            USB_TRACE("Configuration descriptor sent %d bytes.\n", transferSize);
        } else {
            // Stall the endpoint
            port->moduleBaseAddress->USB_ENDPTCTRL[0] |= USB_ENDPTCTRL_TXS;

            USB_TRACE("Unsupported descriptor request received\n");
            USB_TRACE("\t bRequestType: 0x%02x\n", setupPacket->bRequestType);
            USB_TRACE("\t bRequest: 0x%02x\n", setupPacket->bRequest);
            USB_TRACE("\t wValue: 0x%02x\n", setupPacket->wValue);
            USB_TRACE("\t wIndex: 0x%02x\n", setupPacket->wIndex);
        }
        break;

    case SET_CONFIGURATION:

        usbd_device_send_zero_len_packet(port, 0);

        USB_TRACE("SET_CONFIGURATION\n");
        break;

    case SET_IDLE:
        /*
         * HID class request
         */
        usbd_device_send_zero_len_packet(port, 0);

        USB_TRACE("SET_IDLE\n");
        break;

    case GET_HID_CLASS_DESCRIPTOR:
        if (setupPacket->wValue == 0x2200) {    //! - Send report descriptor.
            transferSize =
                setupPacket->wLength >
                sizeof(hidReportDescriptor) ? sizeof(hidReportDescriptor) : setupPacket->wLength;
            usbd_device_send_control_packet(port, (uint8_t *) hidReportDescriptor, transferSize);
            USB_TRACE("HID report descriptor %d bytes sent.\n", transferSize);

            /*
             * The report descriptor should always be the last thing sent, so
             * at this point we are done with the enumeration.
             * While on some PC, this descriptor will be requested twice. It seemed that
             * thie request times depends on the USB host cotroller and it's driver.
             * So we'd better reponse the setup packed all the time.
             */
            usb_dev_status = USB_DEV_ENUMMURATED;
        }
        break;
    case SET_FEATURE:
        USB_TRACE("SET_FEATURE:  wValue: %d, wIndex: 0x%x.\n", setupPacket->wValue,
                  setupPacket->wIndex);
        if (setupPacket->wValue == 0x0002) {    //! process set test mode
            // This supports the USBHSET tool to set the controller in test mode
            usbd_device_send_zero_len_packet(port, 0);

            // - Set the requested test mode in PORTSC and wait for power cycle
            testMode = setupPacket->wIndex & 0xFF00 << 8;
            port->moduleBaseAddress->USB_PORTSC |= testMode;

            USB_TRACE("Port set to test mode %d\n", setupPacket->wIndex >> 8);

            while (1) ;         // - Wait here for power cycle / board reset
        } else {
            port->moduleBaseAddress->USB_ENDPTCTRL[0] |= USB_ENDPTCTRL_TXS;
        }
        break;
    case CLEAR_FEATURE:
        usbd_device_send_zero_len_packet(port, 0);

        USB_TRACE("CLEAR_FEATURE, wValue: %d, wIndex: 0x%x.\n", setupPacket->wValue,
                  setupPacket->wIndex);
        break;
    case SET_ADDRESS:
        deviceAddress = (setupPacket->wValue) << 25;
        deviceAddress |= 1 << 24;   //! - Set ADRA bit to stage the new address
        port->moduleBaseAddress->USB_DEVICEADDR = deviceAddress;

        usbd_device_send_zero_len_packet(port, 0);

        USB_TRACE("USB device address was set to %d\n", setupPacket->wValue);

        break;

    case SET_REPORT:
        hid_receive_report(port, &report_out);
        USB_TRACE("SET_REPORT, ID: %d, size: %d\n", setupPacket->wValue & 0xFF,
                  setupPacket->wLength);
        USB_TRACE("Report received, ID:%d, payload: 0x%02x\n", report_out.ID,
                  report_out.payload[0]);
        break;

        /*
         * GET_REPORT never received even no EP descriptor included in 
         * the configuration descriptor.
         */

    default:
        // Stall the endpoint
        port->moduleBaseAddress->USB_ENDPTCTRL[0] |= USB_ENDPTCTRL_TXS;
        port->moduleBaseAddress->USB_ENDPTCTRL[0] |= USB_ENDPTCTRL_RXS;

        USB_TRACE("ERR!!! Unsupported command.\n\n");
        USB_TRACE("\t bRequestType: 0x%02x\n", setupPacket->bRequestType);
        USB_TRACE("\t bRequest: 0x%02x\n", setupPacket->bRequest);
        USB_TRACE("\t wValue: 0x%02x\n", setupPacket->wValue);
        USB_TRACE("\t wIndex: 0x%02x\n", setupPacket->wIndex);

        break;
    }                           /* end switch */

    return request;
}

/*!
 * Enumerate device
 *
 * @brief This function processes the setup transfers for the enumeration of the device
 *
 * @param	port				The USB controller to use
 */
void hid_device_enum(usb_module_t * port)
{
    usbdSetupPacket_t setupPacket;

    while (1) {
        if (0 == usbd_get_setup_packet(port, &setupPacket)) {
            hid_usb_req_handler(port, &setupPacket);
            if (usb_dev_status == USB_DEV_ENUMMURATED) {
				printf("0000000000USB_DEV_ENUMMURATED\n");
                break;
            }
        }
    }                           /* end while */
}

/** usb_init - init usb device
  * @cpu: the id of cpu
  *
  */
void usb_init(cpu_type_e cpu)
{
    U32 recvCh;
	U32 timeout_cycle = (USB_INIT_TIMEOUT_MS << 5);     // we use a 32kHz timer for timeout, cycle = t * fclk
	int i;

	/* usb freq populate */
#ifdef MX6X			// actually, we can depend on ROM to set freq
	freq_populate();
#endif

	/* usb port init */
    usbPort.moduleName = "OTG controller";
    usbPort.moduleBaseAddress = (usbRegisters_t *) USB_OTG_BASE_ADDR;
    usbPort.controllerID = OTG;
    usbPort.phyType = Utmi;

	/* hid device init */
	for (i = 0; i < ARRAY_SIZE(usb_cfg); i++) {
		if (cpu == usb_cfg[i].cpuid)
			mouseDeviceDescriptor.idProduct = usb_cfg[i].idProduct;
	}

    usb_hid_device_init(&usbPort);
    usb_attached = 1;

	/* host and device handshake */
    do {
        recvCh = usb_getchar();
    } while (recvCh != HID_MX6X_HANDSHAKE);

	GPT_reset_delay(timeout_cycle);
}
/********************************************************************/
