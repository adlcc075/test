
#ifndef __IOMUX_SETTING_H__
#define __IOMUX_SETTING_H__

#define IOMUX_MUX_CTRL_OFFSET_SHIFT     0
#define IOMUX_MUX_CTRL_OFFSET_MASK      ((iomux_cfg_t)0xfff << \
                                        IOMUX_MUX_CTRL_OFFSET_SHIFT)

#define IOMUX_PAD_CTRL_OFFSET_SHIFT     12
#define IOMUX_PAD_CTRL_OFFSET_MASK      ((iomux_cfg_t)0xfff << \
                                        IOMUX_PAD_CTRL_OFFSET_SHIFT)

#define IOMUX_SEL_INPUT_OFFSET_SHIFT    24
#define IOMUX_SEL_INPUT_OFFSET_MASK     ((iomux_cfg_t)0xfff << \
                                        IOMUX_SEL_INPUT_OFFSET_SHIFT)

#define IOMUX_MUX_MODE_SHIFT		36
#define IOMUX_MUX_MODE_MASK	        ((iomux_cfg_t)0x1f << IOMUX_MUX_MODE_SHIFT)

#define IOMUX_PAD_CTRL_SHIFT	41
#define IOMUX_PAD_CTRL_MASK	    ((iomux_cfg_t)0x7ffff << \
                                        IOMUX_PAD_CTRL_SHIFT)

#define IOMUX_MUX_SEL_INPUT_SHIFT	60
#define IOMUX_MUX_SEL_INPUT_MASK	((iomux_cfg_t)0xf << \
                                        IOMUX_MUX_SEL_INPUT_SHIFT)

#define IOMUX_PAD_CFG(_pad_ctrl_ofs, _pad_ctrl, \
                      _mux_ctrl_ofs, _mux_mode, \
                      _sel_input_ofs, _sel_input)                    \
    (((iomux_cfg_t)(_pad_ctrl_ofs) << IOMUX_PAD_CTRL_OFFSET_SHIFT) | \
        ((iomux_cfg_t)(_pad_ctrl) << IOMUX_PAD_CTRL_SHIFT) |    \
        ((iomux_cfg_t)(_mux_ctrl_ofs) << IOMUX_MUX_CTRL_OFFSET_SHIFT) |    \
        ((iomux_cfg_t)(_mux_mode) << IOMUX_MUX_MODE_SHIFT) |    \
        ((iomux_cfg_t)(_sel_input_ofs) << IOMUX_SEL_INPUT_OFFSET_SHIFT) | \
        ((iomux_cfg_t)(_sel_input) << IOMUX_MUX_SEL_INPUT_SHIFT))

typedef unsigned long long iomux_cfg_t;

#endif

