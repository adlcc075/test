#ifndef __TIMER_H__
#define __TIMER_H__


#include "io.h"

#define EPIT_EPITCR_OFFSET      0x00    //  32bit timer 3 control reg
#define EPIT_EPITSR_OFFSET      0x04    //  32bit timer 3 prescaler reg
#define EPIT_EPITLR_OFFSET      0x08    //  32bit timer 3 compare reg
#define EPIT_EPITCMPR_OFFSET    0x0C    //  32bit timer 3 capture reg
#define EPIT_EPITCNR_OFFSET     0x10    //  32bit timer 3 counter reg

#define EPITCR      EPIT_EPITCR_OFFSET
#define EPITSR      EPIT_EPITSR_OFFSET
#define EPITLR      EPIT_EPITLR_OFFSET
#define EPITCMPR        EPIT_EPITCMPR_OFFSET
#define EPITCNR     EPIT_EPITCNR_OFFSET

void init_clock(u32 rate);
void hal_delay_us(unsigned int usecs);
void hal_set_timer_timeout(unsigned int usecs);
BOOL hal_is_timer_timeout(void);
void GPT_init(void);
void GPT_delay(UINT32 delay);
BOOL GPT_status(void);
void GPT_reset_delay(UINT32 gpt_reset_delay);
void GPT_disable(void);

#endif

