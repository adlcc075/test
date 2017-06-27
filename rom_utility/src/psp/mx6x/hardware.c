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

#include "io.h"
#include "platform.h"

// ARM core is a special case. Assign 0 to it.
struct hw_module core = {
    "ARM",
    0,
};

struct hw_module *mx6x_module[] = {
    &core,
    NULL,
};

/*!
 * Retrieve the freq info based on the passed in module_base.
 * @param   module_base     the base address of the module
 * @return  frequency in hz
 */
u32 get_freq(u32 module_base)
{
    return 66500000;        //todo
}

/*!
 * Retrieve the clocks based on the hardware configuration and fill in the freq
 * info in each module's structure.
 */
void freq_populate(void)
{
    int i;
    struct hw_module *tmp;

    /* 
     * UART clock tree: PLL3 (480MHz) div-by-6: 80MHz
     * 80MHz uart_clk_podf (div-by-1) = 80MHz (UART module clock input)
     */

    /* Power up 480MHz PLL */
    reg32_write_mask(HW_ANADIG_USB1_PLL_480_CTRL_RW, 0x00001000, 0x00001000);

    /* Enable 480MHz PLL */
    reg32_write_mask(HW_ANADIG_USB1_PLL_480_CTRL_RW, 0x00002000, 0x00002000);

    /* UART clock is sourced from PLL3_80M 
     *   PLL3_80M is div-by-6 from PLL3 480MHz
     *   However, even though this is the default
     *   make sure div-by-6 is set 
     *   It appears that these dividers are not programmable
     */

    /*  UART clock is divided down from PLL3_80M
     * Default is div-by-1, but set to make sure
     *   CSCDR1: uart_clk_podf
     */
    reg32_write_mask(CCM_CSCDR1, 0x00000000, 0x0000003F);
    // try div-by-2 to get 40MHz; seeing some erroneous characters output, see if this resolves
    // note, this didn't help
//    reg32_write_mask(CCM_CSCDR1, 0x00000001, 0x0000003F);    

    /* Ensure UART clock is not gated, bits[27:26] and bits[25:24] of CCGR5 */
    reg32_write_mask(CCM_CCGR5, 0x0F000000, 0x0F000000);

    /* Populate module frequency settings (important for UART driver) */
    for (i = 0; (tmp = mx6x_module[i]) != NULL; i++) {
        tmp->freq = get_freq(tmp->base);
    }
}

