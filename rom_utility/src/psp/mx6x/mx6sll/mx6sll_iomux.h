
#ifndef __MX6SLL_IOMUX_H__
#define __MX6SLL_IOMUX_H__

#include "iomux_def.h"
#include "iomux_setting.h"

#define USDHC_PAD_CTRL	(PAD_CTL_PKE | PAD_CTL_PUE |	\
		                 PAD_CTL_PUS_47K_UP | PAD_CTL_SPEED_HIGH | \
		                 PAD_CTL_DSE_40ohm | PAD_CTL_SRE_SLOW | PAD_CTL_HYS)


#define MX6SLL_PAD_SD1_CLK   IOMUX_PAD_CFG(0x0534, USDHC_PAD_CTRL, 0x022C, 0, 0x0000, 0)
#define MX6SLL_PAD_SD1_CMD   IOMUX_PAD_CFG(0x0538, USDHC_PAD_CTRL, 0x0230, 0 | IOMUX_CONFIG_SION, 0x0000, 0)
#define MX6SLL_PAD_SD1_DAT0  IOMUX_PAD_CFG(0x053C, USDHC_PAD_CTRL, 0x0234, 0, 0x0000, 0)
#define MX6SLL_PAD_SD1_DAT1  IOMUX_PAD_CFG(0x0540, USDHC_PAD_CTRL, 0x0238, 0, 0x0000, 0)
#define MX6SLL_PAD_SD1_DAT2  IOMUX_PAD_CFG(0x0544, USDHC_PAD_CTRL, 0x023C, 0, 0x0000, 0)
#define MX6SLL_PAD_SD1_DAT3  IOMUX_PAD_CFG(0x0548, USDHC_PAD_CTRL, 0x0240, 0, 0x0000, 0)
#define MX6SLL_PAD_SD1_DAT4  IOMUX_PAD_CFG(0x054C, USDHC_PAD_CTRL, 0x0244, 0, 0x0000, 0)
#define MX6SLL_PAD_SD1_DAT5  IOMUX_PAD_CFG(0x0550, USDHC_PAD_CTRL, 0x0248, 0, 0x0000, 0)
#define MX6SLL_PAD_SD1_DAT6  IOMUX_PAD_CFG(0x0554, USDHC_PAD_CTRL, 0x024C, 0, 0x0000, 0)
#define MX6SLL_PAD_SD1_DAT7  IOMUX_PAD_CFG(0x0558, USDHC_PAD_CTRL, 0x0250, 0, 0x0000, 0)


#define MX6SLL_PAD_SD2_CLK   IOMUX_PAD_CFG(0x055C, USDHC_PAD_CTRL, 0x0254, 0, 0x0000, 0)
#define MX6SLL_PAD_SD2_CMD   IOMUX_PAD_CFG(0x0560, USDHC_PAD_CTRL, 0x0258, 0 | IOMUX_CONFIG_SION, 0x0000, 0)
#define MX6SLL_PAD_SD2_DAT0  IOMUX_PAD_CFG(0x0564, USDHC_PAD_CTRL, 0x025C, 0, 0x0000, 0)
#define MX6SLL_PAD_SD2_DAT2  IOMUX_PAD_CFG(0x0568, USDHC_PAD_CTRL, 0x0260, 0, 0x0000, 0)
#define MX6SLL_PAD_SD2_DAT1  IOMUX_PAD_CFG(0x056C, USDHC_PAD_CTRL, 0x0264, 0, 0x0000, 0)
#define MX6SLL_PAD_SD2_DAT3  IOMUX_PAD_CFG(0x0570, USDHC_PAD_CTRL, 0x0268, 0, 0x0000, 0)
#define MX6SLL_PAD_SD2_DAT4  IOMUX_PAD_CFG(0x0574, USDHC_PAD_CTRL, 0x026C, 0, 0x0000, 0)
#define MX6SLL_PAD_SD2_DAT5  IOMUX_PAD_CFG(0x0578, USDHC_PAD_CTRL, 0x0270, 0, 0x0000, 0)
#define MX6SLL_PAD_SD2_DAT6  IOMUX_PAD_CFG(0x057C, USDHC_PAD_CTRL, 0x0274, 0, 0x0000, 0)
#define MX6SLL_PAD_SD2_DAT7  IOMUX_PAD_CFG(0x0580, USDHC_PAD_CTRL, 0x0278, 0, 0x0000, 0)


#define MX6SLL_PAD_SD3_CLK   IOMUX_PAD_CFG(0x0588, USDHC_PAD_CTRL, 0x0280, 0, 0x0000, 0)
#define MX6SLL_PAD_SD3_CMD   IOMUX_PAD_CFG(0x058C, USDHC_PAD_CTRL, 0x0280, 0 | IOMUX_CONFIG_SION, 0x0000, 0)
#define MX6SLL_PAD_SD3_DAT0  IOMUX_PAD_CFG(0x0590, USDHC_PAD_CTRL, 0x0288, 0, 0x0000, 0)
#define MX6SLL_PAD_SD3_DAT1  IOMUX_PAD_CFG(0x0594, USDHC_PAD_CTRL, 0x028C, 0, 0x0000, 0)
#define MX6SLL_PAD_SD3_DAT2  IOMUX_PAD_CFG(0x0598, USDHC_PAD_CTRL, 0x0290, 0, 0x0000, 0)
#define MX6SLL_PAD_SD3_DAT3  IOMUX_PAD_CFG(0x059C, USDHC_PAD_CTRL, 0x0294, 0, 0x0000, 0)


#endif
