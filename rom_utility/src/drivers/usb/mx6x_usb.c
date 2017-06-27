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
 * @file mx6x_usb.c
 * @brief i.MX6x specific USB routines.
 *
 * The functions in this file are implementation specific routines, like USB transceiver, PLL, clocks etc.\n
 * Each supported i.MX device has a dedicated set of routines.\n
 * The routines are referenced in the usb_module structure.
 * 
 * @ingroup diag_usb
 */

#include "usb.h"
#include "regsusb.h"
#include "platform.h"

/*!
 * This function enables the clocks needed for USB operation.
 * @param port
 * @return
 */
int usbEnableClocks(usb_module_t * port)
{
#ifdef MX6X
    multiAccess_t *usbPllControl;

    *(volatile uint32_t *)(CCM_CCGR6) |= 0x00000003;

	/* port1 only for host with HSIC PHY, while port2 & port3 for otg with UTMI PHY */
    switch (port->controllerID) {
    case OTG:                  // OTG, Host2 and Host3 use the same PLL
    case Host2:
    case Host3:
        usbPllControl = (multiAccess_t *) (HW_CCM_ANALOG_USBPHY0_PLL_480_CTRL_ADDR);
        break;
    case Host1:
        usbPllControl = (multiAccess_t *) (HW_CCM_ANALOG_USBPHY1_PLL_480_CTRL_ADDR);
        break;
    default:
        return (-1);
    }

    usbPllControl->SET = USBPLL_CTRL_POWER; //! - Turn PLL power on.
    while (!(usbPllControl->RW & USBPLL_CTRL_LOCK)) ;   //! - Wait for PLL to lock
    usbPllControl->CLEAR = USBPLL_CTRL_BYPASS;  //! - Clear bypass
    usbPllControl->SET = USBPLL_CTRL_ENABLE;    //! - Enable PLL clock output for the PHY
#endif
    return (0);
}

/*!
 * Enable USB transceiver\n
 * This function enables the USB transceiver for the selected USB port.
 *
 * @param port      USB module to initialize
 */
int usbEnableTransceiver(usb_module_t * port)
{
#ifdef MX6X
    usbPhy_t *usbPhy;

    switch (port->controllerID) {
    case OTG:
        usbPhy = (usbPhy_t *) (IP2APB_USBPHY1_BASE_ADDR);   // Point register structure to USBPHY1
        break;
    case Host1:
        usbPhy = (usbPhy_t *) (IP2APB_USBPHY1_BASE_ADDR);   // Point register structure to USBPHY2
        break;
    default:
        break;
    }
    //! NOTE !! CLKGATE must be cleared before clearing power down
    usbPhy->USBPHY_CTRL.CLEAR = (USBPHY_CTRL_SFTRST);   //! - clear SFTRST
    usbPhy->USBPHY_CTRL.CLEAR = (USBPHY_CTRL_CLKGATE);  //! - clear CLKGATE
    usbPhy->USBPHY_PWD.RW = 0;  //! - clear all power down bits
    usbPhy->USBPHY_CTRL.SET = (3 << 14);
    //usbPhy->USBPHY_CTRL.SET = (1 << 1); //! - Enable Host Disconnect

    //! disable the charger detector. This must be off during normal operation
    {
        // this register is not documented. Will be updated in the next release
        uint32_t *ChargerDetectControl;
        ChargerDetectControl = (uint32_t *) 0x020c81b0;
        *ChargerDetectControl |= 1 << 20;   // disable detector
    }

    //! Check if all power down bits are clear
    if (usbPhy->USBPHY_PWD.RW != 0)
        return -1;              // Phy still in power-down mode. Check if all clocks are running.
    else
#endif
        return 0;
}

int usbEnableConnect(usb_module_t * port, bool enable)
{
#ifdef MX6X
    usbPhy_t *usbPhy;

    switch (port->controllerID) {
    case OTG:
        usbPhy = (usbPhy_t *) (IP2APB_USBPHY1_BASE_ADDR);   // Point register structure to USBPHY1
        break;
    case Host1:
        usbPhy = (usbPhy_t *) (IP2APB_USBPHY2_BASE_ADDR);   // Point register structure to USBPHY2
        break;
    default:
        break;
    }
    if (enable)
        usbPhy->USBPHY_CTRL.SET = USBPHY_CTRL_ENDEVPLUGINDETECT;
    else
        usbPhy->USBPHY_CTRL.CLEAR = USBPHY_CTRL_ENDEVPLUGINDETECT;
#endif
    return 0;
}
