
#ifndef __IOMUX_DEF_H__
#define __IOMUX_DEF_H__

/*
 * Use to set PAD control
 */
#define PAD_CTL_LVE_OFFSET	(22)
#define PAD_CTL_LVE		(1 << 18)
#define NO_PAD_CTRL		(1 << 17)
#define PAD_CTL_HYS		(1 << 16)

#define NO_PAD_I		0
#define NO_MUX_I		0
#define IOMUX_CONFIG_SION	(0x1 << 4)

#define PAD_CTL_PUS_100K_DOWN	(0 << 14)
#define PAD_CTL_PUS_47K_UP	(1 << 14)
#define PAD_CTL_PUS_100K_UP	(2 << 14)
#define PAD_CTL_PUS_22K_UP	(3 << 14)

#define PAD_CTL_PUE		(1 << 13)
#define PAD_CTL_PKE		(1 << 12)
#define PAD_CTL_ODE		(1 << 11)

#define PAD_CTL_SPEED_LOW	(1 << 6)
#define PAD_CTL_SPEED_MED	(2 << 6)
#define PAD_CTL_SPEED_HIGH	(3 << 6)

#define PAD_CTL_DSE_DISABLE	(0 << 3)
#define PAD_CTL_DSE_240ohm	(1 << 3)
#define PAD_CTL_DSE_120ohm	(2 << 3)
#define PAD_CTL_DSE_80ohm	(3 << 3)
#define PAD_CTL_DSE_60ohm	(4 << 3)
#define PAD_CTL_DSE_48ohm	(5 << 3)
#define PAD_CTL_DSE_40ohm	(6 << 3)
#define PAD_CTL_DSE_34ohm	(7 << 3)

#define PAD_CTL_SRE_FAST	(1 << 0)
#define PAD_CTL_SRE_SLOW	(0 << 0)

#define GPIO_PIN_MASK 0x1f

#define GPIO_PORT_SHIFT 5
#define GPIO_PORT_MASK (0x7 << GPIO_PORT_SHIFT)

#define GPIO_PORTA	(0 << GPIO_PORT_SHIFT)
#define GPIO_PORTB	(1 << GPIO_PORT_SHIFT)
#define GPIO_PORTC	(2 << GPIO_PORT_SHIFT)
#define GPIO_PORTD	(3 << GPIO_PORT_SHIFT)
#define GPIO_PORTE	(4 << GPIO_PORT_SHIFT)
#define GPIO_PORTF	(5 << GPIO_PORT_SHIFT)

#endif

