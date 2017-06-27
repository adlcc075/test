
#ifndef __IOMUX_DEF_H__
#define __IOMUX_DEF_H__

/*
 * Use to set PAD control
 */
#define PAD_CTL_LVE_OFFSET	(22)
#define PAD_CTL_LVE		(1 << 18)
#define NO_PAD_CTRL		(1 << 17)

#define NO_PAD_I		0
#define NO_MUX_I		0
#define IOMUX_CONFIG_SION	(0x1 << 4)

#define PAD_CTL_DSE_1P8V_140OHM		(0)
#define PAD_CTL_DSE_1P8V_35OHM		(1)
#define PAD_CTL_DSE_1P8V_70OHM		(2)
#define PAD_CTL_DSE_1P8V_23OHM		(3)
#define PAD_CTL_DSE_3P3V_196OHM		(0)
#define PAD_CTL_DSE_3P3V_49OHM		(1)
#define PAD_CTL_DSE_3P3V_98OHM		(2)
#define PAD_CTL_DSE_3P3V_32OHM		(3)

#define PAD_CTL_SRE_EN				(1 << 2)

#define PAD_CTL_HYS_EN				(1 << 3)

#define PAD_CTL_PUE_EN				(1 << 4)

#define PAD_CTL_PUS_PD100KOHM		(0 << 5)
#define PAD_CTL_PUS_PU5KOHM			(1 << 5)
#define PAD_CTL_PUS_PU47KOHM		(2 << 5)
#define PAD_CTL_PUS_PU100KOHM		(3 << 5)

//#define GPIO_PIN_MASK 0x1f
//#define GPIO_PORT_SHIFT 5
//#define GPIO_PORT_MASK (0x7 << GPIO_PORT_SHIFT)

//#define GPIO_PORTA	(0 << GPIO_PORT_SHIFT)
//#define GPIO_PORTB	(1 << GPIO_PORT_SHIFT)
//#define GPIO_PORTC	(2 << GPIO_PORT_SHIFT)
//#define GPIO_PORTD	(3 << GPIO_PORT_SHIFT)
//#define GPIO_PORTE	(4 << GPIO_PORT_SHIFT)
//#define GPIO_PORTF	(5 << GPIO_PORT_SHIFT)

#endif

