
#include "platform.h"
#include "io.h"
#include "iomux_def.h"
#include "iomux_setting.h"
#include "iomuxc.h"

NEED_MODULE(IOMUXC)

static int pad_configuration_setup(iomux_cfg_t cfg)
{
    unsigned int mux_ctl_off = (cfg & IOMUX_MUX_CTRL_OFFSET_MASK) >>
                                IOMUX_MUX_CTRL_OFFSET_SHIFT;
	unsigned int mux_mode = (cfg & IOMUX_MUX_MODE_MASK) >> IOMUX_MUX_MODE_SHIFT;
	unsigned int sel_input_offset = (cfg & IOMUX_SEL_INPUT_OFFSET_MASK) >>
                                     IOMUX_SEL_INPUT_OFFSET_SHIFT;
	unsigned int sel_input = (cfg & IOMUX_MUX_SEL_INPUT_MASK) >>
                                IOMUX_MUX_SEL_INPUT_SHIFT;
	unsigned int pad_ctl_off =(cfg & IOMUX_PAD_CTRL_OFFSET_MASK) >> IOMUX_PAD_CTRL_OFFSET_SHIFT;
	unsigned int pad_ctl = (cfg & IOMUX_PAD_CTRL_MASK) >> IOMUX_PAD_CTRL_SHIFT;

	if (mux_ctl_off)
        *(volatile unsigned int *)(regs_IOMUXC_base + mux_ctl_off) = mux_mode;

	if (sel_input_offset)
		*(volatile unsigned int *)(regs_IOMUXC_base + sel_input_offset) = sel_input;

	if (!(pad_ctl & NO_PAD_CTRL) && pad_ctl_off) {
		if (pad_ctl & PAD_CTL_LVE) {
			/* Set the bit for LVE */
			pad_ctl |= (1 << PAD_CTL_LVE_OFFSET);
			pad_ctl &= ~(PAD_CTL_LVE);
		}

        *(volatile unsigned int *)(regs_IOMUXC_base + pad_ctl_off) = pad_ctl;
	}

    return 0;
}

int module_iomux_configuration_setup(iomux_cfg_t *cfg_tbl)
{
    unsigned int i;

    for (i = 0; 0 != *(cfg_tbl + i); i++ )
        pad_configuration_setup(*(cfg_tbl + i));

    return 0;
}

