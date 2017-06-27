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
 * @file usbd_drv.c
 * @brief USB device driver functions
 *
 * These functions demonstrate how to program the USB controller and\n
 * are not intended for use in an actual application.
 */
#include "usb.h"

extern usbMemZone_t *usbMemZone;

/**************** static functions *********************/

static unsigned int *usbdGetUsbBuffer(usbdEndpointInfo_t * usbdEndpoint)
{
    uint32_t endpointNumber = usbdEndpoint->endpointNumber;
    usbdEndpointDirection_e endpointDirection = usbdEndpoint->endpointDirection;

    if (0 == endpointNumber) {
        if (IN == endpointDirection) {
            return (unsigned int *)usbMemZone->Ep0In_Buffer;
        } else {
            return (unsigned int *)usbMemZone->Ep0Out_Buffer;
        }
    }

    if (1 == endpointNumber) {
        if (IN == endpointDirection) {
            return (unsigned int *)usbMemZone->Ep1In_Buffer;
        } else {
            return (unsigned int *)usbMemZone->Ep1Out_Buffer;
        }
    }

    return NULL;
}

//! Function to create a new transfer descriptor
/*!
 * This functions allocate memory for a device transfer descriptor (dTD) and
 * initializes the dTD. This function assumes the dTD is the last in the list so
 * the next dTD pointer is marked as invalid.
 *
 * 	@param transferSize			number of bytes to be transferred
 *	@param interruptOnComplete	interrupt on complete flag
 *	@param multOverride			Override the queue head multiplier setting (0 for default)
 *	@param bufferPointer		pointer to the data buffer
 *
 *	@return 					pointer to the transfer descriptor
 *
 */
 /* DTD:
  * 1st word: next dtd linker pointer
  * 2nd word: total_bytes | ioc | multo | status
              total_bytes: total bytes trans in by this descriptor
			  ioc: bool, interrupt on complete
			  multo: for trans ISO, to override the multiplier in the QH(queue head)
			  status: 7 active, 6 halted, 5 data buffer error, 3 trans error
  * 3rd word: buffer pointer (page 0) | current offset
  * 4th word: buffer pointer (page 1)
  * 5th word: buffer pointer (page 2)
  * 6th word: buffer pointer (page 3)
  * 7th word: buffer pointer (page 4)
  */
static usbdEndpointDtd_t *usbd_dtd_init(usbdEndpointInfo_t * usbdEndpoint, uint32_t transferSize,
                                        uint32_t interruptOnComplete, uint32_t multOverride,
                                        uint32_t * bufferPointer)
{
    uint32_t token;
    uint32_t *usbBuffer = (uint32_t *) usbdGetUsbBuffer(usbdEndpoint);

	/* get dtd poiniter by EP number and direction */
    usbdEndpointDtd_t *usbDtd =
        &(usbMemZone->EndpointDtds[usbdEndpoint->endpointNumber][usbdEndpoint->endpointDirection]);

	/* next dtd pointer */
    usbDtd->nextDtd = 0xDEAD0001;   //! invalidate nextDtd (set T-bit)

	/* token, including total_bytes, ioc, multo, status */
    if (interruptOnComplete)
        token = USB_DTD_TOKEN_IOC;
    else
        token = 0;

    usbDtd->dtdToken = (token | USB_DTD_TOKEN_TOTAL_BYTES(transferSize)
                        | USB_DTD_TOKEN_MULTO(multOverride)
                        | USB_DTD_TOKEN_STAT_ACTIVE);

	/* page 0 buffer pointer */ 
    if (IN == usbdEndpoint->endpointDirection) {
        if (usbBuffer != bufferPointer) {
            memcpy(usbBuffer, bufferPointer, transferSize);
        }
    }

    usbDtd->dtdBuffer[0] = (uint32_t) usbBuffer;
    usbDtd->dtdBuffer[1] = 0;
    usbDtd->dtdBuffer[2] = 0;
    usbDtd->dtdBuffer[3] = 0;
    usbDtd->dtdBuffer[4] = 0;

    return usbDtd;
}

/********************* APIs ****************************/

//! Function to initialize an endpoint queue head
/*!
 * Initialize an endpoint queue head. The space for the endpoint queue heads was
 * allocated when the endpoint list was created, so this function does not
 * call malloc.
 *
 * @param endpointList		location of the endpoint list
 * @param usbdEndpoint    Pointer to the endpoint characteristics
 * @param nextDtd		pointer to the first transfer descriptor for the queue head
 */
 /* queue head:
 	1st word: mult | zlt | max packet len | ios
			  mult: number of packets transed per dtd
			  zlt: bool, zero length termination
              max packet len: the max value is 0x400 (1KB)
			  ios: bool, interrupt on setup
	2nd word: current dtd pointer
	3rd~9th word: copy of 7 words dtd here
	10th word: reserved
	11th word: setup bytes 3 2 1 0
	12th word: setup bytes 7 6 5 4
	*/

void usbd_endpoint_qh_init(usbdEndpointInfo_t * usbdEndpoint, uint32_t nextDtd)
{
    usbdDeviceEndpointQueueHead_t *usbdQueueHead;
    uint32_t endpointCapabilities;

    usbdEndpointPair_t *endpointList = usbMemZone->EndpointPairs;

    if (usbdEndpoint->endpointDirection == OUT) {
        usbdQueueHead = &(endpointList[usbdEndpoint->endpointNumber].out);
    } else {
        usbdQueueHead = &(endpointList[usbdEndpoint->endpointNumber].in);
    }

    if (usbdEndpoint->interruptOnSetup == true)
        endpointCapabilities = USB_EP_QH_EP_CHAR_IOS;
    else
        endpointCapabilities = 0;

    usbdQueueHead->endpointCharacteristics = (endpointCapabilities
                                              | USB_EP_QH_EP_CHAR_MULT(usbdEndpoint->mult)
                                              | USB_EP_QH_EP_CHAR_ZLT   // Turn Zero Length Termination off
                                              | USB_EP_QH_EP_CHAR_MAX_PACKET(usbdEndpoint->
                                                                             maxPacketLength));

    usbdQueueHead->currentDtd = 0;  // Will be updated by the controller with nextDtd on startup
    usbdQueueHead->nextDtd = nextDtd;   // This will be the first dTD that the controller uses
    usbdQueueHead->dtdToken = 0;    // Will be updated with the token from nextdTD on startup

    int i;
    for (i = 0; i < 5; i++)     // Clear the buffer pointers (for ease of debug)
        usbdQueueHead->dtdBuffer[i] = 0;

#ifdef PLAT_CONFIG_USB_BUFFER_IN_IRAM
#pragma message("\n\nchache\n\n")
    l1_flush_cache_range((unsigned int)usbdQueueHead,
                         (unsigned int)usbdQueueHead + sizeof(usbdDeviceEndpointQueueHead_t));
    l2_flush_cache_range((unsigned int)usbdQueueHead,
                         (unsigned int)usbdQueueHead + sizeof(usbdDeviceEndpointQueueHead_t));
#endif
}

//! Function to initialize the USB controller for device operation.
/*! This initialization performs basic configuration to prepare the device for connection to a host.
 *
 * @param port           The USB module to use
 * @param endpointList   pointer to list with endpoint queue heads
 */

uint32_t usbd_device_init(usb_module_t * port)
{
    usbdEndpointPair_t *endpointList = usbMemZone->EndpointPairs;

    memset(usbMemZone, 0, sizeof(usbMemZone_t));

    if (usbEnableClocks(port) == -1) {
        return -1;
    }

    if (usbEnableTransceiver(port) == -1) {
        return -1;
    }

    if (port->phyType == Ulpi) {
        port->moduleBaseAddress->USB_PORTSC =
            (port->moduleBaseAddress->USB_PORTSC & (~(USB_PORTSC_PTS(3))))
            | (USB_PORTSC_PTS(2));
    } else {
        port->moduleBaseAddress->USB_PORTSC &= (~(USB_PORTSC_PTS(3)));  // UTMI PHY
    }

    // Reset controller after switching PHY's
    port->moduleBaseAddress->USB_USBCMD |= (USB_USBCMD_RST);

    // wait for reset to complete
    while (port->moduleBaseAddress->USB_USBCMD & (USB_USBCMD_RST)) ;

    //! - Set controller to device Mode
    port->moduleBaseAddress->USB_USBMODE = (USB_USBMODE_CM_DEVICE);

    //! - Set initial configuration for controller
    port->moduleBaseAddress->USB_USBCMD &= (~(USB_USBCMD_ITC(0xFF)));   // Set interrupt threshold control = 0

    port->moduleBaseAddress->USB_USBMODE |= (USB_USBMODE_SLOM); // Setup Lockouts Off

    //! - Set the device endpoint list address
    port->moduleBaseAddress->USB_ASYNCLISTADDR = (uint32_t) & endpointList[0];

    //! - Configure Endpoint 0.
    //  Only the required EP0 for control traffic is initialized at this time.
    port->moduleBaseAddress->USB_ENDPTCTRL[0] |= (USB_ENDPTCTRL_TXE | USB_ENDPTCTRL_RXE);   // Enable TX/RX of EP0

    //! - Initialize queue head for EP0 IN (device to host)
    usbdEndpointInfo_t endpoint0In, endpoint0Out, endpoint1Info;

    endpoint0In.endpointNumber = 0;
    endpoint0In.endpointDirection = IN;
    endpoint0In.maxPacketLength = 0x40;
    endpoint0In.mult = 0;
    endpoint0In.interruptOnSetup = true;

    usbd_endpoint_qh_init(&endpoint0In, DTD_TERMINATE);

    //! - Initialize queue head for EP0 OUT (host to device)
    endpoint0Out.endpointNumber = 0;
    endpoint0Out.endpointDirection = OUT;
    endpoint0Out.maxPacketLength = 0x40;
    endpoint0Out.mult = 0;
    endpoint0Out.interruptOnSetup = false;

    usbd_endpoint_qh_init(&endpoint0Out, DTD_TERMINATE);

    port->moduleBaseAddress->USB_ENDPTCTRL[1] |= USB_ENDPTCTRL_TXE | USB_ENDPTCTRL_TXT_BULK;	// bluk mode

    //! - Initialize IN endpoint queue head for endpoint 1
    endpoint1Info.endpointNumber = 1;
    endpoint1Info.endpointDirection = IN;
    endpoint1Info.interruptOnSetup = false;
    endpoint1Info.maxPacketLength = 0x200;
    endpoint1Info.mult = 0;
    usbd_endpoint_qh_init(&endpoint1Info, DTD_TERMINATE);

    /* Flush L1-D and L2 cache */
#ifdef PLAT_CONFIG_USB_BUFFER_IN_IRAM
    l1_flush_cache_range((unsigned int)usbMemZone, (unsigned int)usbMemZone + sizeof(usbMemZone_t));
    l2_flush_cache_range((unsigned int)usbMemZone, (unsigned int)usbMemZone + sizeof(usbMemZone_t));
#endif

    //! Start controller, D+ will be pullup, and host will reset bus
    port->moduleBaseAddress->USB_USBCMD |= (USB_USBCMD_RS);		// run

    return (0);
}

//! Function to initialize the controller after the USB bus reset
/*!
 * USB device response to a USB bus reset.
 *
 * @param port		USB controller to use
 * @return			returns the operating speed of the port
 */
usbPortSpeed_t usbd_bus_reset(usb_module_t * port)
{
    uint32_t temp;

    port->moduleBaseAddress->USB_USBSTS &= USB_USBSTS_URI;

    //! - Clear all setup token semaphores
    temp = port->moduleBaseAddress->USB_ENDPTSETUPSTAT;
    port->moduleBaseAddress->USB_ENDPTSETUPSTAT = temp;

    //! - Clear all complete status bits
    temp = port->moduleBaseAddress->USB_ENDPTCOMPLETE;
    port->moduleBaseAddress->USB_ENDPTCOMPLETE = temp;

    //! - Wait for all primed status to clear
    while (port->moduleBaseAddress->USB_ENDPTPRIME) ;

    //! - Flush all endpoints
    port->moduleBaseAddress->USB_ENDPTFLUSH = 0xFFFFFFFF;

    //! - Wait for host to stop signaling reset
    while (port->moduleBaseAddress->USB_PORTSC & (USB_PORTSC_PR)) ;		// this bit auto set to 0 when reset complete

    //! - Clear reset status bit
    port->moduleBaseAddress->USB_USBSTS |= (USB_USBSTS_URI | USB_USBSTS_UI);

    //! Return the connection speed (full/high speed)
    return (usb_get_port_speed(port));
}

/*!
 * USB device function to return the data from a setup packet.
 * NOTE: We assume only endpoint 0 is a control endpoint
 *
 * @param	endpointList	pointer to the device endpoint list address
 * @param	port			pointer to controller info structure
 * @param	setupPacket		Setup data of the setup packet
 */
int usbd_get_setup_packet(usb_module_t * port, usbdSetupPacket_t * setupPacket)
{
    usbdEndpointPair_t *endpointList;
    usbdDeviceEndpointQueueHead_t *enpointQueueHead;    // reference to the queue head

    endpointList = usbMemZone->EndpointPairs;
    enpointQueueHead = (usbdDeviceEndpointQueueHead_t *) & (endpointList[0].out);   // endpoint 0 out

    if (port->moduleBaseAddress->USB_ENDPTSETUPSTAT & USB_ENDPTSETUPSTAT_ENDPTSETUPSTAT(1)) {

#ifdef PLAT_CONFIG_USB_BUFFER_IN_IRAM
        l1_invalidate_cache_range(&enpointQueueHead->setupBuffer,
                                  &enpointQueueHead->setupBuffer + sizeof(usbdSetupPacket_t));
        l2_invalidate_cache_range(&enpointQueueHead->setupBuffer,
                                  &enpointQueueHead->setupBuffer + sizeof(usbdSetupPacket_t));
#endif
        /*! copy setup data from queue head to buffer and check semaphore\n
         * SUTW will be cleared when a new setup packet arrives.\n
         * To avoid data corruption, we check the SUTW flag after the copy\n
         * and when it is cleared, we repeat the copy to get the new setup data.
         */
        do {
            port->moduleBaseAddress->USB_USBCMD |= (USB_USBCMD_SUTW);   //! - Set setup tripwire
            memcpy(setupPacket, &enpointQueueHead->setupBuffer, sizeof(usbdSetupPacket_t)); //! - copy the setup data
        } while (!(port->moduleBaseAddress->USB_USBCMD & (USB_USBCMD_SUTW)));   //! - repeat if SUTW got cleared

        //! - Clear setup identification
        port->moduleBaseAddress->USB_ENDPTSETUPSTAT |= (USB_ENDPTSETUPSTAT_ENDPTSETUPSTAT(1));

        //! - Clear Setup tripwire bit
        port->moduleBaseAddress->USB_USBCMD &= (~USB_USBCMD_SUTW);

        //! - Flush Endpoint 0 IN and OUT in case some a previous transaction was left pending
        port->moduleBaseAddress->USB_ENDPTSTATUS &=
            (USB_ENDPTSTATUS_ERBR(0) | USB_ENDPTSTATUS_ETBR(0));

        //! - Wait for ENDPSETUPSTAT to clear.\n
        //! It must be clear before the status phase/data phase can be primed
        while (port->moduleBaseAddress->
               USB_ENDPTSETUPSTAT & (USB_ENDPTSETUPSTAT_ENDPTSETUPSTAT(1))) ;
    } else {
        return -1;
    }

    return 0;
}

/* 
 * It is assumed that the data size to be transferred is less than the maxiam pakage size of the endpoint.
 */
int usbd_send_packet(usb_module_t * port, usbdEndpointInfo_t * endpoint, uint8_t * buffer,
                     uint32_t size)
{
    uint32_t endpointNumber = endpoint->endpointNumber;
    usbdEndpointPair_t *endpointList = usbMemZone->EndpointPairs;
    usbdEndpointDtd_t *dtdIn;
#ifdef PLAT_CONFIG_USB_BUFFER_IN_IRAM
    uint32_t *usbBuffer = (uint32_t *) usbdGetUsbBuffer(endpoint);
#endif

    dtdIn = usbd_dtd_init(endpoint, size, 1, 0, (uint32_t *) buffer);
#ifdef PLAT_CONFIG_USB_BUFFER_IN_IRAM
    l1_flush_cache_range((unsigned int)dtdIn, (unsigned int)dtdIn + sizeof(usbdEndpointDtd_t));
    l2_flush_cache_range((unsigned int)dtdIn, (unsigned int)dtdIn + sizeof(usbdEndpointDtd_t));

    l1_flush_cache_range((unsigned int)usbBuffer, (unsigned int)usbBuffer + size);
    l2_flush_cache_range((unsigned int)usbBuffer, (unsigned int)usbBuffer + size);
#endif

    endpointList[endpointNumber].in.nextDtd = (uint32_t) dtdIn;
#ifdef PLAT_CONFIG_USB_BUFFER_IN_IRAM
    l1_flush_cache_range((unsigned int)&endpointList[endpointNumber].in,
                         (unsigned int)&endpointList[endpointNumber].in +
                         sizeof(usbdDeviceEndpointQueueHead_t));
    l2_flush_cache_range((unsigned int)&endpointList[endpointNumber].in,
                         (unsigned int)&endpointList[endpointNumber].in +
                         sizeof(usbdDeviceEndpointQueueHead_t));
#endif

    port->moduleBaseAddress->USB_ENDPTPRIME |= (USB_ENDPTPRIME_PETB(0x01 << endpointNumber));

    //! - Wait for prime to complete
    while (port->moduleBaseAddress->
           USB_ENDPTPRIME & (USB_ENDPTPRIME_PETB(0x01 << endpointNumber))) ;

    while (0 ==
           ((port->moduleBaseAddress->USB_ENDPTCOMPLETE) & (USB_ENDPTCOMPLETE_ETCE(0x01 << endpointNumber)))) ;
    port->moduleBaseAddress->USB_ENDPTCOMPLETE |= (USB_ENDPTCOMPLETE_ETCE(0x01 << endpointNumber));

    //! - Wait for IN to complete and clear flag
    while (!(port->moduleBaseAddress->USB_USBSTS & (USB_USBSTS_UI))) ;
    port->moduleBaseAddress->USB_USBSTS |= (USB_USBSTS_UI);

    //! - Check Active flag and wait for it to clear
    //while (dtdIn->dtdToken & USB_DTD_TOKEN_STAT_ACTIVE) ;

    return 0;
}

/* 
 * It is assumed that the data size to be transferred is less than the maxiam pakage size of the endpoint.
 */
int usbd_receive_packet(usb_module_t * port, usbdEndpointInfo_t * endpoint, uint8_t * buffer,
                        uint32_t size)
{
    uint32_t endpointNumber = endpoint->endpointNumber;
    usbdEndpointPair_t *endpointList = usbMemZone->EndpointPairs;
    unsigned int *usbBuffer = usbdGetUsbBuffer(endpoint);
    usbdEndpointDtd_t *dtdOut;

    dtdOut = usbd_dtd_init(endpoint, size, 1, 0, (uint32_t *) buffer);
#ifdef PLAT_CONFIG_USB_BUFFER_IN_IRAM
    l1_flush_cache_range((unsigned int)dtdOut, (unsigned int)dtdOut + sizeof(usbdEndpointDtd_t));
    l2_flush_cache_range((unsigned int)dtdOut, (unsigned int)dtdOut + sizeof(usbdEndpointDtd_t));
#endif

    endpointList[endpointNumber].out.nextDtd = (uint32_t) dtdOut;
#ifdef PLAT_CONFIG_USB_BUFFER_IN_IRAM
    l1_flush_cache_range((unsigned int)&endpointList[endpointNumber].out,
                         (unsigned int)&endpointList[endpointNumber].out +
                         sizeof(usbdDeviceEndpointQueueHead_t));
    l2_flush_cache_range((unsigned int)&endpointList[endpointNumber].out,
                         (unsigned int)&endpointList[endpointNumber].out +
                         sizeof(usbdDeviceEndpointQueueHead_t));
#endif

    port->moduleBaseAddress->USB_ENDPTPRIME |= (USB_ENDPTPRIME_PERB(0x01 << endpointNumber));

    //! - Wait for prime to complete
    while (port->moduleBaseAddress->
           USB_ENDPTPRIME & (USB_ENDPTPRIME_PERB(0x01 << endpointNumber))) ;

    //! - Wait for OUT to complete and clear flag
    while (!(port->moduleBaseAddress->USB_USBSTS & (USB_USBSTS_UI))) ;
    port->moduleBaseAddress->USB_USBSTS |= (USB_USBSTS_UI);

    //! - Check Active flag and wait for it to clear
    //while (dtdOut->dtdToken & USB_DTD_TOKEN_STAT_ACTIVE) ;

    size -= ((dtdOut->dtdToken >> 16) & 0x7FFF);

#ifdef PLAT_CONFIG_USB_BUFFER_IN_IRAM
    l1_invalidate_cache_range((unsigned int)usbBuffer,
                              (unsigned int)usbBuffer + ((size + 32) / 32) * 32);
    l2_invalidate_cache_range((unsigned int)usbBuffer,
                              (unsigned int)usbBuffer + ((size + 32) / 32) * 32);
#endif

    if ((unsigned int *)buffer != usbBuffer) {
        memcpy(buffer, usbBuffer, size);
    }

    return size;
}

//! Function to send an IN control packet to the host.
/*! NOTE: this function uses the default control endpoint (0).\n
 *       The endpoint number is hard-coded.
 *
 * @param	port			Controller to use
 * @param	endpointList	pointer to the device endpoint list
 * @param	buffer			Data to be sent to host
 * @param	size			Amount of data to be transferred in bytes
 */
void usbd_device_send_control_packet(usb_module_t * port, uint8_t * buffer, uint32_t size)
{
    //usbdEndpointDtd_t *dtdIn, *dtdOut;
    //usbdEndpointPair_t *endpointList = usbMemZone->EndpointPairs;

    //! - Initialize queue head for EP0 IN (device to host)
    usbdEndpointInfo_t endpoint0In, endpoint0Out;

    endpoint0In.endpointNumber = 0;
    endpoint0In.endpointDirection = IN;
    endpoint0In.maxPacketLength = 0x40;
    endpoint0In.mult = 0;
    endpoint0In.interruptOnSetup = true;

    usbd_send_packet(port, &endpoint0In, buffer, size);
    /*!
     * Initialize a transfer descriptor for receive.\n
     * The host won't actually send anything,\n
     * but a dTD needs to be setup to correctly deal with the 0 byte OUT\n
     * packet the host sends after receiving the IN data.\n
     * The OUT transaction must be queued together with the IN transaction\n
     * for error recovery purposes.\n
     */
    //! - Initialize queue head for EP0 OUT (host to device)
    endpoint0Out.endpointNumber = 0;
    endpoint0Out.endpointDirection = OUT;
    endpoint0Out.maxPacketLength = 0x40;
    endpoint0Out.mult = 0;
    endpoint0Out.interruptOnSetup = false;

    usbd_receive_packet(port, &endpoint0Out, usbMemZone->Ep0Out_Buffer, 0x40);
}

 //! Function to send an zero length IN packet to the host.
 /*!
  * Zero Length packets are used as completion handshake in control transfers.\n
  * They can also be used to signal the end of a variable length transfer.\n
  *
  * @param port             Usb controller to use
  * @param endpointList     pointer to the device endpoint list
  * @param endpointNumber   endpoint info data structure for the endpoint to use
  */
void usbd_device_send_zero_len_packet(usb_module_t * port, uint32_t endpointNumber)
{
    usbdEndpointDtd_t *usbDtd;
    usbdEndpointPair_t *endpointList = usbMemZone->EndpointPairs;

    usbdEndpointInfo_t endpoint0In;

    endpoint0In.endpointNumber = 0;
    endpoint0In.endpointDirection = IN;

    usbDtd = usbd_dtd_init(&endpoint0In, 0, 1, 0, NULL);
#ifdef PLAT_CONFIG_USB_BUFFER_IN_IRAM
    l1_flush_cache_range((unsigned int)usbDtd, (unsigned int)usbDtd + sizeof(usbdEndpointDtd_t));
    l2_flush_cache_range((unsigned int)usbDtd, (unsigned int)usbDtd + sizeof(usbdEndpointDtd_t));
#endif

    //! - Put dTD on endpoint queue head
    endpointList[0].in.nextDtd = (uint32_t) usbDtd;
#ifdef PLAT_CONFIG_USB_BUFFER_IN_IRAM
    l1_flush_cache_range((unsigned int)&endpointList[0].in,
                         (unsigned int)&endpointList[0].in + sizeof(usbdDeviceEndpointQueueHead_t));
    l2_flush_cache_range((unsigned int)&endpointList[0].in,
                         (unsigned int)&endpointList[0].in + sizeof(usbdDeviceEndpointQueueHead_t));
#endif

    //! - Prime Tx buffer for control endpoint
    port->moduleBaseAddress->USB_ENDPTPRIME |= (USB_ENDPTPRIME_PETB(1 << endpointNumber));

    //! - Wait for prime to complete
    while (port->moduleBaseAddress->USB_ENDPTPRIME & (USB_ENDPTPRIME_PETB(1 << endpointNumber))) ;

    //! - Wait for IN to complete and clear flag
    while (!(port->moduleBaseAddress->USB_USBSTS & (USB_USBSTS_UI))) ;
    port->moduleBaseAddress->USB_USBSTS |= (USB_USBSTS_UI);

    //! - Check Active flag and wait for it to clear
    //while (usbDtd->dtdToken & USB_DTD_TOKEN_STAT_ACTIVE) ;
}
