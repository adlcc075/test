/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
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

#ifndef REGS_USB_H
#define REGS_USB_H

#include "regs.h"
#include "io.h"

#ifndef USBOH3_USB_BASE_ADDR
#define USBOH3_USB_BASE_ADDR		((unsigned int)regs_USB_base)
#endif

#define USB_OTG_BASE_ADDR          (USBOH3_USB_BASE_ADDR+0x000)
#define USB_H1_BASE_ADDR           (USBOH3_USB_BASE_ADDR+0x200)
#define USB_H2_BASE_ADDR           (USBOH3_USB_BASE_ADDR+0x400)
#define USB_H3_BASE_ADDR           (USBOH3_USB_BASE_ADDR+0x600)
#define USB_OTG_CTRL_REG           (USBOH3_USB_BASE_ADDR+0x800)
#define USB_UH1_CTRL_REG           (USBOH3_USB_BASE_ADDR+0x804)
#define USB_UH2_CTRL_REG           (USBOH3_USB_BASE_ADDR+0x808)
#define USB_UH3_CTRL_REG           (USBOH3_USB_BASE_ADDR+0x80C)
#define USB_UH2_HSIC_REG           (USBOH3_USB_BASE_ADDR+0x810)
#define USB_UH3_HSIC_REG           (USBOH3_USB_BASE_ADDR+0x814)
#define USB_OTG_UTMIPHY_0          (USBOH3_USB_BASE_ADDR+0x818)
#define USB_UH1_UTMIPHY_0          (USBOH3_USB_BASE_ADDR+0x81C)
#define USB_UH2_HSIC_DLL_CFG1      (USBOH3_USB_BASE_ADDR+0x820)
#define USB_UH2_HSIC_DLL_CFG2      (USBOH3_USB_BASE_ADDR+0x824)
#define USB_UH2_HSIC_DLL_CFG3      (USBOH3_USB_BASE_ADDR+0x828)
#define USB_UH2_HSIC_DLL_STS       (USBOH3_USB_BASE_ADDR+0x82C)
#define USB_UH3_HSIC_DLL_CFG1      (USBOH3_USB_BASE_ADDR+0x830)
#define USB_UH3_HSIC_DLL_CFG2      (USBOH3_USB_BASE_ADDR+0x834)
#define USB_UH3_HSIC_DLL_CFG3      (USBOH3_USB_BASE_ADDR+0x838)
#define USB_UH3_HSIC_DLL_STS       (USBOH3_USB_BASE_ADDR+0x83C)

#define USB_H1_ID                  (USB_H1_BASE_ADDR+0x000) // Identification Register
#define USB_H1_HWGENERAL           (USB_H1_BASE_ADDR+0x004) // General Hardware Parameters
#define USB_H1_HWHOST              (USB_H1_BASE_ADDR+0x008) // Host Hardware Parameters
#define USB_H1_HWTXBUF             (USB_H1_BASE_ADDR+0x010) // TX Buffer Hardware Parameters
#define USB_H1_HWRXBUF             (USB_H1_BASE_ADDR+0x014) // RX Buffer Hardware Parameters
#define USB_H1_GPTIMER0LD          (USB_H1_BASE_ADDR+0x080)
#define USB_H1_GPTIMER0CTRL        (USB_H1_BASE_ADDR+0x084)
#define USB_H1_GPTIMER1LD          (USB_H1_BASE_ADDR+0x088)
#define USB_H1_GPTIMER1CTRL        (USB_H1_BASE_ADDR+0x08C)
#define USB_H1_SBUSCFG             (USB_H1_BASE_ADDR+0x090)
#define USB_H1_CAPLENGTH           (USB_H1_BASE_ADDR+0x100) // Capability Register Length
#define USB_H1_HCIVERSION          (USB_H1_BASE_ADDR+0x102) // Host Interface Version Number
#define USB_H1_HCSPARAMS           (USB_H1_BASE_ADDR+0x104) // Host Ctrl. Structural Parameters
#define USB_H1_HCCPARAMS           (USB_H1_BASE_ADDR+0x108) // Host Ctrl. Capability Parameters
#define USB_H1_USBCMD              (USB_H1_BASE_ADDR+0x140) // USB Command
#define USB_H1_USBSTS              (USB_H1_BASE_ADDR+0x144) // USB Status
#define USB_H1_USBINTR             (USB_H1_BASE_ADDR+0x148) // USB Interrupt Enable
#define USB_H1_FRINDEX             (USB_H1_BASE_ADDR+0x14C) // USB Frame Index
#define USB_H1_PERIODICLISTBASE    (USB_H1_BASE_ADDR+0x154) // Frame List Base Address
#define USB_H1_ASYNCLISTADDR       (USB_H1_BASE_ADDR+0x158) // Next Asynchronous List Address
#define USB_H1_BURSTSIZE           (USB_H1_BASE_ADDR+0x160) // Programmable Burst Size
#define USB_H1_TXFILLTUNING        (USB_H1_BASE_ADDR+0x164) // Host Transmit Pre-Buffer Packet Tuning
#define USB_H1_ICUSB               (USB_H1_BASE_ADDR+0x16C) // Host IC USB
#define USB_H1_CONFIGFLAG          (USB_H1_BASE_ADDR+0x180) // Configured Flag Register
#define USB_H1_PORTSC1             (USB_H1_BASE_ADDR+0x184) // Port Status/Control
#define USB_H1_USBMODE             (USB_H1_BASE_ADDR+0x1A8) // USB Device Mode

#define USB_H2_ID                  (USB_H2_BASE_ADDR+0x000) // Identification Register
#define USB_H2_HWGENERAL           (USB_H2_BASE_ADDR+0x004) // General Hardware Parameters
#define USB_H2_HWHOST              (USB_H2_BASE_ADDR+0x008) // Host Hardware Parameters
#define USB_H2_HWTXBUF             (USB_H2_BASE_ADDR+0x010) // TX Buffer Hardware Parameters
#define USB_H2_HWRXBUF             (USB_H2_BASE_ADDR+0x014) // RX Buffer Hardware Parameters
#define USB_H2_GPTIMER0LD          (USB_H2_BASE_ADDR+0x080)
#define USB_H2_GPTIMER0CTRL        (USB_H2_BASE_ADDR+0x084)
#define USB_H2_GPTIMER1LD          (USB_H2_BASE_ADDR+0x088)
#define USB_H2_GPTIMER1CTRL        (USB_H2_BASE_ADDR+0x08C)
#define USB_H2_SBUSCFG             (USB_H2_BASE_ADDR+0x090)
#define USB_H2_CAPLENGTH           (USB_H2_BASE_ADDR+0x100) // Capability Register Length
#define USB_H2_HCIVERSION          (USB_H2_BASE_ADDR+0x102) // Host Interface Version Number
#define USB_H2_HCSPARAMS           (USB_H2_BASE_ADDR+0x104) // Host Ctrl. Structural Parameters
#define USB_H2_HCCPARAMS           (USB_H2_BASE_ADDR+0x108) // Host Ctrl. Capability Parameters
#define USB_H2_USBCMD              (USB_H2_BASE_ADDR+0x140) // USB Command
#define USB_H2_USBSTS              (USB_H2_BASE_ADDR+0x144) // USB Status
#define USB_H2_USBINTR             (USB_H2_BASE_ADDR+0x148) // USB Interrupt Enable
#define USB_H2_FRINDEX             (USB_H2_BASE_ADDR+0x14C) // USB Frame Index
#define USB_H2_PERIODICLISTBASE    (USB_H2_BASE_ADDR+0x154) // Frame List Base Address
#define USB_H2_ASYNCLISTADDR       (USB_H2_BASE_ADDR+0x158) // Next Asynchronous List Address
#define USB_H2_BURSTSIZE           (USB_H2_BASE_ADDR+0x160) // Programmable Burst Size
#define USB_H2_TXFILLTUNING        (USB_H2_BASE_ADDR+0x164) // Host Transmit Pre-Buffer Packet Tuning
#define USB_H2_ICUSB               (USB_H2_BASE_ADDR+0x16C) // Host IC USB
#define USB_H2_CONFIGFLAG          (USB_H2_BASE_ADDR+0x180) // Configured Flag Register
#define USB_H2_PORTSC1             (USB_H2_BASE_ADDR+0x184) // Port Status/Control
#define USB_H2_USBMODE             (USB_H2_BASE_ADDR+0x1A8) // USB Device Mode

#define USB_H3_ID                  (USB_H3_BASE_ADDR+0x000) // Identification Register
#define USB_H3_HWGENERAL           (USB_H3_BASE_ADDR+0x004) // General Hardware Parameters
#define USB_H3_HWHOST              (USB_H3_BASE_ADDR+0x008) // Host Hardware Parameters
#define USB_H3_HWTXBUF             (USB_H3_BASE_ADDR+0x010) // TX Buffer Hardware Parameters
#define USB_H3_HWRXBUF             (USB_H3_BASE_ADDR+0x014) // RX Buffer Hardware Parameters
#define USB_H3_GPTIMER0LD          (USB_H3_BASE_ADDR+0x080)
#define USB_H3_GPTIMER0CTRL        (USB_H3_BASE_ADDR+0x084)
#define USB_H3_GPTIMER1LD          (USB_H3_BASE_ADDR+0x088)
#define USB_H3_GPTIMER1CTRL        (USB_H3_BASE_ADDR+0x08C)
#define USB_H3_SBUSCFG             (USB_H3_BASE_ADDR+0x090)
#define USB_H3_CAPLENGTH           (USB_H3_BASE_ADDR+0x100) // Capability Register Length
#define USB_H3_HCIVERSION          (USB_H3_BASE_ADDR+0x102) // Host Interface Version Number
#define USB_H3_HCSPARAMS           (USB_H3_BASE_ADDR+0x104) // Host Ctrl. Structural Parameters
#define USB_H3_HCCPARAMS           (USB_H3_BASE_ADDR+0x108) // Host Ctrl. Capability Parameters
#define USB_H3_USBCMD              (USB_H3_BASE_ADDR+0x140) // USB Command
#define USB_H3_USBSTS              (USB_H3_BASE_ADDR+0x144) // USB Status
#define USB_H3_USBINTR             (USB_H3_BASE_ADDR+0x148) // USB Interrupt Enable
#define USB_H3_FRINDEX             (USB_H3_BASE_ADDR+0x14C) // USB Frame Index
#define USB_H3_PERIODICLISTBASE    (USB_H3_BASE_ADDR+0x154) // Frame List Base Address
#define USB_H3_ASYNCLISTADDR       (USB_H3_BASE_ADDR+0x158) // Next Asynchronous List Address
#define USB_H3_BURSTSIZE           (USB_H3_BASE_ADDR+0x160) // Programmable Burst Size
#define USB_H3_TXFILLTUNING        (USB_H3_BASE_ADDR+0x164) // Host Transmit Pre-Buffer Packet Tuning
#define USB_H3_ICUSB               (USB_H3_BASE_ADDR+0x16C) // Host IC USB
#define USB_H3_CONFIGFLAG          (USB_H3_BASE_ADDR+0x180) // Configured Flag Register
#define USB_H3_PORTSC1             (USB_H3_BASE_ADDR+0x184) // Port Status/Control
#define USB_H3_USBMODE             (USB_H3_BASE_ADDR+0x1A8) // USB Device Mode

#define USB_OTG_ID                 (USB_OTG_BASE_ADDR+0x000)    // Identification Register
#define USB_OTG_HWGENERAL          (USB_OTG_BASE_ADDR+0x004)    // General Hardware Parameters
#define USB_OTG_HWHOST             (USB_OTG_BASE_ADDR+0x008)    // Host Hardware Parameters
#define USB_OTG_HWDEVICE           (USB_OTG_BASE_ADDR+0x00C)    // Device Hardware Parameters
#define USB_OTG_HWTXBUF            (USB_OTG_BASE_ADDR+0x010)    // TX Buffer Hardware Parameters
#define USB_OTG_HWRXBUF            (USB_OTG_BASE_ADDR+0x014)    // RX Buffer Hardware Parameters
#define USB_OTG_GPTIMER0LD         (USB_OTG_BASE_ADDR+0x080)
#define USB_OTG_GPTIMER0CTRL       (USB_OTG_BASE_ADDR+0x084)
#define USB_OTG_GPTIMER1LD         (USB_OTG_BASE_ADDR+0x088)
#define USB_OTG_GPTIMER1CTRL       (USB_OTG_BASE_ADDR+0x08C)
#define USB_OTG_SBUSCFG            (USB_OTG_BASE_ADDR+0x090)
#define USB_OTG_EPSEL              (USB_OTG_BASE_ADDR+0x094)
#define USB_OTG_CAPLENGTH          (USB_OTG_BASE_ADDR+0x100)    // Capability Register Length
#define USB_OTG_HCIVERSION         (USB_OTG_BASE_ADDR+0x102)    // Host Interface Version Number
#define USB_OTG_HCSPARAMS          (USB_OTG_BASE_ADDR+0x104)    // Host Ctrl. Structural Parameters
#define USB_OTG_HCCPARAMS          (USB_OTG_BASE_ADDR+0x108)    // Host Ctrl. Capability Parameters
#define USB_OTG_DCIVERSION         (USB_OTG_BASE_ADDR+0x120)    // Dev. Interface Version Number
#define USB_OTG_DCCPARAMS          (USB_OTG_BASE_ADDR+0x124)    // Device Ctrl. Capability Parameters
#define USB_OTG_USBCMD             (USB_OTG_BASE_ADDR+0x140)    // USB Command
#define USB_OTG_USBSTS             (USB_OTG_BASE_ADDR+0x144)    // USB Status
#define USB_OTG_USBINTR            (USB_OTG_BASE_ADDR+0x148)    // USB Interrupt Enable
#define USB_OTG_FRINDEX            (USB_OTG_BASE_ADDR+0x14C)    // USB Frame Index
#define USB_OTG_PERIODICLISTBASE   (USB_OTG_BASE_ADDR+0x154)    // Frame List Base Address
#define USB_OTG_ASYNCLISTADDR      (USB_OTG_BASE_ADDR+0x158)    // Next Asynchronous List Address
#define USB_OTG_BURSTSIZE          (USB_OTG_BASE_ADDR+0x160)    // Programmable Burst Size
#define USB_OTG_TXFILLTUNING       (USB_OTG_BASE_ADDR+0x164)    // Host Transmit Pre-Buffer Packet Tuning
#define USB_OTG_ICUSB              (USB_OTG_BASE_ADDR+0x16C)    // OTG IC USB
#define USB_OTG_CONFIGFLAG         (USB_OTG_BASE_ADDR+0x180)    // Configured Flag Register
#define USB_OTG_PORTSC1            (USB_OTG_BASE_ADDR+0x184)    // Port Status/Control
#define USB_OTG_OTGSC              (USB_OTG_BASE_ADDR+0x1A4)    // On-The-Go (OTG) Status and Control
#define USB_OTG_USBMODE            (USB_OTG_BASE_ADDR+0x1A8)    // USB Device Mode
#define USB_OTG_ENPDTSETUPSTAT     (USB_OTG_BASE_ADDR+0x1AC)    // Endpoint Setup Status
#define USB_OTG_ENDPTPRIME         (USB_OTG_BASE_ADDR+0x1B0)    // Endpoint Initialization
#define USB_OTG_ENDPTFLUSH         (USB_OTG_BASE_ADDR+0x1B4)    // Endpoint De-Initialize
#define USB_OTG_ENDPTSTATUS        (USB_OTG_BASE_ADDR+0x1B8)    // Endpoint Status
#define USB_OTG_ENDPTCOMPLETE      (USB_OTG_BASE_ADDR+0x1BC)    // Endpoint Complete
#define USB_OTG_ENDPTCTRL0         (USB_OTG_BASE_ADDR+0x1C0)    // Endpoint Control 0
#define USB_OTG_ENDPTCTRL1         (USB_OTG_BASE_ADDR+0x1C4)    // Endpoint Control 1
#define USB_OTG_ENDPTCTRL2         (USB_OTG_BASE_ADDR+0x1C8)    // Endpoint Control 2
#define USB_OTG_ENDPTCTRL3         (USB_OTG_BASE_ADDR+0x1CC)    // Endpoint Control 3
#define USB_OTG_ENDPTCTRL4         (USB_OTG_BASE_ADDR+0x1D0)    // Endpoint Control 4
#define USB_OTG_ENDPTCTRL5         (USB_OTG_BASE_ADDR+0x1D4)    // Endpoint Control 5
#define USB_OTG_ENDPTCTRL6         (USB_OTG_BASE_ADDR+0x1D8)    // Endpoint Control 6
#define USB_OTG_ENDPTCTRL7         (USB_OTG_BASE_ADDR+0x1DC)    // Endpoint Control 7

/*!
 * Non-core USB registers
 * These registers are implementation specific and may vary between i.MX parts
 */
typedef struct usbNonCore {
    uint32_t USBNC_USB_OTG_CTRL;
    uint32_t USBNC_USB_UH1_CTRL;
    uint32_t USBNC_USB_UH2_CTRL;
    uint32_t USBNC_USB_UH3_CTRL;
    uint32_t USBNC_USB_UH2_HSIC_CTRL;
    uint32_t USBNC_USB_UH3_HSIC_CTRL;
    uint32_t USBNC_USB_OTG_PHY_CTRL_0;
    uint32_t USBNC_USB_UH1_PHY_CTRL_0;
    uint32_t USBNC_USB_UH2_HSIC_DLL_CFG1;
    uint32_t USBNC_USB_UH2_HSIC_DLL_CFG2;
    uint32_t USBNC_USB_UH2_HSIC_DLL_CFG3;
    uint32_t USBNC_USB_UH3_HSIC_DLL_CFG1;
    uint32_t USBNC_USB_UH3_HSIC_DLL_CFG2;
    uint32_t USBNC_USB_UH3_HSIC_DLL_CFG3;
} usbNonCore_t;

/*!
 * Bit definitions for USB OTG, UH1, UH2 and UH3 control registers
 * Not all bits are defined in every control register !
 */
#define WIR                 (1 << 31)   // Wakeup interrupt enable
#define WKUP_VBUS_EN        (1 << 17)   // Wakeup on Vbus change (OTG and UH1 only)
#define WKUP_ID_EN          (1 << 16)   // Wakeup on ID change (OTG only)
#define WKUP_SW             (1 << 15)   // Software wakeup
#define WKUP_SW_EN          (1 << 14)   // Software wakeup enable
#define UTMI_ON_CLOCK       (1 << 13)   // Force PHY clock (OTG and UH1) on when in low-power suspend
#define _480MHZ_CLOCK_ON    (1 << 13)   // Force HSIC clock (UH2 and UH3) on when in low-power suspend
#define SUSPENDM            (1 << 12)   // Force PHY in low-power suspend
#define RESET_PHY           (1 << 11)   // RESET PHY
#define WIE                 (1 << 10)   // Wakeup interrupt enable
#define PM                  (1 << 9)    // Power/Overcurrent mask
#define OVER_CUR_POL        (1 << 8)    // Overcurrent polarity
#define OVER_CUR_DIS        (1 << 7)    // Disable overcurrent detection

/*!
 * Bit definitions for HSIC control registers (UH2 and UH3 only)
 */

#define CLK_VLD             (1 << 31)   // Clock valid status
#define HSIC_EN             (1 << 12)   // Enable HSIC interface
#define HSIC_CLK_ON         (1 << 11)   // Force 480 MHz clock on during suspend

/*!
 * Bit definitions for UTMI PHY control 0 registers (OTG and UH1 only)
 */

#define CLK_VLD             (1 << 31)   // Clock valid status

/*!
 * multiAccess:
 * Address register with RW, Set, Clear and Toggle addresses
 */

typedef struct multiAccess {
    uint32_t RW;
    uint32_t SET;
    uint32_t CLEAR;
    uint32_t TOGGLE;
} multiAccess_t;

/*!
 * USB PLL bit definitions
 */
#define USBPLL_CTRL_LOCK                       (1 << 31)    // PLL Locked status
#define USBPLL_CTRL_BYPASS                     (1 << 16)    // enable PLL bypass
#define USBPLL_CTRL_BYPASS_CLK_SRC(x)          ((x) << 14)  // Select bypass clock source
#define USBPLL_CTRL_ENABLE                     (1 << 13)    // Enable PLL clock output
#define USBPLL_CTRL_POWER                      (1 << 12)    // Power up PLL
#define USBPLL_CTRL_HOLD_RING_OFF              (1 << 11)    // Analog debug -- not for application use
#define USBPLL_CTRL_DOUBLE_CP                  (1 << 10)    //
#define USBPLL_CTRL_HALF_CP                    (1 << 9)
#define USBPLL_CTRL_DOUBLE_LF                  (1 << 8)
#define USBPLL_CTRL_HALF_LF                    (1 << 7)
#define USBPLL_CTRL_EN_USB_CLKS                (1 << 6)
#define USBPLL_CTRL_CONTROL0                   0    // Not for application use
#define USBPLL_CTRL_DIV_SELECT(x)              (x)  // Pll loop divider 0: Fout=Fref*20; 1: Fout=Fref*22.

/*!
 * Register access and bit definitions for USB PHY on i.MX6
 *
 */
typedef struct usbPhy {
    multiAccess_t USBPHY_PWD;   // PHY power down register
    multiAccess_t USBPHY_TX;    // Transmitter control
    multiAccess_t USBPHY_RX;    // Receiver control
    multiAccess_t USBPHY_CTRL;  // PHY general control
    multiAccess_t USBPHY_STATUS;    // PHY status register
    multiAccess_t USBPHY_DEBUG0_STATUS; // Debug register -- Not for application use
    multiAccess_t USBPHY_DEBUG1;    // Debug register == Not for application use
    multiAccess_t USBPHY_VERSION;   //
    multiAccess_t USBPHY_IP;    // Analog debug
} usbPhy_t;

/*
 * USB PHY bit definitions
 */

#define USBPHY_PWD_RXPWDRX       (1 << 20)  // power down receiver except FS differential RX
#define USBPHY_PWD_RXPWDDIFF     (1 << 19)  // Power down HS receiver
#define USBPHY_PWD_RXPWD1PT1     (1 << 18)  // Power down FS differential receiver
#define USBPHY_PWD_RXPWDENV      (1 << 17)  // power down receiver envelope detector
#define USBPHY_PWD_TXPWDV2I      (1 << 12)  // power down transmitter V/I converter
#define USBPHY_PWD_TXPWDIBIAS    (1 << 11)  // Power down current bias
#define USBPHY_PWD_TXPWDFS       (1 << 10)  // Power down full-speed drivers

#define USBPHY_CTRL_SFTRST                  (1 << 31)
#define USBPHY_CTRL_CLKGATE					(1 << 30)
#define USBPHY_CTRL_UTMI_SUSPENDM           (1 << 29)
#define USBPHY_CTRL_HOST_FORCE_LS_SE0       (1 << 28)
#define USBPHY_CTRL_OTG_ID_VALUE            (1 << 27)
#define USBPHY_CTRL_ENAUTOSET_USBCLKS       (1 << 26)
#define USBPHY_CTRL_ENAUTOCLR_USBCLKGATE    (1 << 25)
#define USBPHY_CTRL_FSDLL_RST_EN            (1 << 24)
#define USBPHY_CTRL_ENVBUSCHG_WKUP          (1 << 23)
#define USBPHY_CTRL_ENIDCHG_WKUP            (1 << 22)
#define USBPHY_CTRL_ENDPDMCHG_WKUP          (1 << 21)
#define USBPHY_CTRL_ENAUTOCLR_PHY_PWD       (1 << 20)
#define USBPHY_CTRL_ENAUTOCLR_CLKGATE       (1 << 19)
#define USBPHY_CTRL_ENAUTO_PWRON_PLL        (1 << 18)
#define USBPHY_CTRL_WAKEUP_IRQ              (1 << 17)
#define USBPHY_CTRL_ENIRQWAKEUP             (1 << 16)
#define USBPHY_CTRL_ENUTMILEVEL3            (1 << 15)
#define USBPHY_CTRL_ENUTMILEVEL2            (1 << 14)
#define USBPHY_CTRL_DATA_ON_LRADC           (1 << 13)
#define USBPHY_CTRL_DEVPLUGIN_IRQ           (1 << 12)
#define USBPHY_CTRL_ENIRQDEVPLUGIN          (1 << 11)
#define USBPHY_CTRL_RESUME_IRQ              (1 << 10)
#define USBPHY_CTRL_ENIRQRESUMEDETECT       (1 << 9)
#define USBPHY_CTRL_RESUMEIRQSTICKY         (1 << 8)
#define USBPHY_CTRL_ENOTGIDDETECT           (1 << 7)
#define USBPHY_CTRL_OTG_ID_CHG_IRQ          (1 << 6)
#define USBPHY_CTRL_DEVPLUGIN_POLARITY      (1 << 5)
#define USBPHY_CTRL_ENDEVPLUGINDETECT       (1 << 4)
#define USBPHY_CTRL_HOSTDISCONDETECT_IRQ    (1 << 3)
#define USBPHY_CTRL_ENIRQHOSTDISCON         (1 << 2)
#define USBPHY_CTRL_ENHOSTDISCONDETECT      (1 << 1)
#define USBPHY_CTRL_ENOTG_ID_CHG_IRQ        (1 << 0)

/*
 * USBPHY_STATUS bit definitions
 */
#define USBPHY_STATUS_RESUME_STATUS              (1 << 10)
#define USBPHY_STATUS_OTGID_STATUS               (1 << 8)
#define USBPHY_STATUS_DEVPLUGIN_STATUS           (1 << 6)
#define USBPHY_STATUS_HOSTDISCONDETECT_STATUS    (1 << 3)

#endif /* __USB_REGS__ */
