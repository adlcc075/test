/*
 * Copyright (C) 2011-2014, Freescale Semiconductor, Inc. All Rights Reserved
 * THIS SOURCE CODE IS CONFIDENTIAL AND PROPRIETARY AND MAY NOT
 * BE USED OR DISTRIBUTED WITHOUT THE WRITTEN PERMISSION OF
 * Freescale Semiconductor, Inc.
*/

/*!
 * @file imx_i2c.c
 * @brief Driver for the i.MX I2C controller.
 *
 * @ingroup diag_i2c
 */

#include "io.h"
#include "imx_i2c.h"
#include "timer/timer.h"

#define WAIT_RXAK_LOOPS     1000000

static inline int is_bus_free(unsigned int base)
{
    int i = WAIT_RXAK_LOOPS;

    while ((readw(base + I2C_I2SR) & I2C_I2SR_IBB) && (--i > 0)) ;

    if (i <= 0) {
        printf("Error: I2C Bus not free!\n");
        return -1;
    }

    return 0;
}

static int wait_till_busy(u32 base)
{
    int i = 100000;

    while (((readw(base + I2C_I2SR) & I2C_I2SR_IBB) == 0) && (--i > 0)) {
        if (readw(base + I2C_I2SR) & I2C_I2SR_IAL) {
            printf("Error: arbitration lost!\n");
            return -1;
        }
    }

    if (i <= 0) {
        printf("Error: timeout in %s; %d\n");
        return -1;
    }

    return 0;
}

#define ASSERT_NO_ARBITRATION_LOST(stat)  \
{ \
    if (stat & I2C_I2SR_IAL) { \
        printf("Error %d: Arbitration lost\n", __LINE__); \
        return -1; \
    } \
}

static inline void imx_send_stop(unsigned int base)
{
    writew((readw(base + I2C_I2CR) & ~(I2C_I2CR_MSTA)), base + I2C_I2CR);
}

static int wait_op_done(u32 base, int is_tx)
{
    volatile unsigned short v;
    int i = WAIT_RXAK_LOOPS;

    while ((((v = readw(base + I2C_I2SR)) & I2C_I2SR_IIF) == 0) && (--i > 0)) ;

    if (i <= 0) {
        I2CDBG("I2C Error: timeout unexpected\n");
        return -1;
    }

    writew(0x0, base + I2C_I2SR);

    if (v & I2C_I2SR_IAL) {
        I2CDBG("Error %d: Arbitration lost\n", __LINE__);
        return ERR_ARB_LOST;
    }

    if (is_tx) {
        if (v & I2C_I2SR_RXAK) {
            I2CDBG("Error %d: no ack received\n", __LINE__);
            /* Generate a STOP signal */
            imx_send_stop(base);
            return ERR_NO_ACK;
        }
    }

    return 0;
}

int wait_ack_done(u32 base, int timeout)
{
    int time_out = timeout;

    while ((!(readw(base + I2C_I2SR) & 0x02)) && (time_out--)) ;

    if (time_out <= 0)
        return -1;

    writew(readw(base + I2C_I2SR) & (~0x02), base + I2C_I2SR);  //must clear IIF bit

    time_out = timeout;

    while ((readw(base + I2C_I2SR) & 0x01) && (time_out--)) ;

    if (time_out <= 0) {
        /* Generate a STOP signal */
        writew((readw(base + I2C_I2CR) & ~(I2C_I2CR_MSTA)), base + I2C_I2CR);
        return -1;
    } else
        return 0;
}

// For master TX, always expect a RXAK signal to be set!
static int tx_byte(u8 * data, u32 base)
{
    int ret = 0;

    I2CDBG("%s(data=0x%02x, base=0x%x)\n", __FUNCTION__, *data, base);

    // clear both IAL and IIF bits
    writew(0, base + I2C_I2SR);

    writew(*data, base + I2C_I2DR);

    if ((ret = wait_op_done(base, 1)) != 0)
        return ret;

    return ret;
}

// For master RX
static int rx_bytes(u8 * data, u32 base, int sz)
{
    unsigned short i2cr;
    int i, ret = 0;

    for (i = 0; sz > 0; sz--, i++) {
        if (wait_op_done(base, 0) != 0)
            return -1;

        // the next two if-statements setup for the next read control register value
        if (sz == 1) {
            // last byte --> generate STOP
            imx_send_stop(base);
        }

        if (sz == 2) {
            // 2nd last byte --> set TXAK bit to NOT generate ACK
            i2cr = readw(base + I2C_I2CR);
            writew(i2cr | I2C_I2CR_TXAK, base + I2C_I2CR);
        }
        // read the true data
        data[i] = readw(base + I2C_I2DR);
        I2CDBG("OK 0x%02x\n", data[i]);
    }

    return ret;
}

/*!
 * This is a rather simple function that can be used for most i2c devices.
 * Common steps for both READ and WRITE:
 *      step 1: issue start signal
 *      step 2: put i2c device addr on the bus (always 1 byte write. the dir always I2C_WRITE)
 *      step 3: offset of the i2c device write (offset within the device. can be 1-4 bytes)
 * For READ:
 *      step 4: do repeat-start
 *      step 5: send slave address again, but indicate a READ operation by setting LSB bit
 *      Step 6: change to receive mode
 *      Step 7: dummy read
 *      Step 8: reading
 * For WRITE:
 *      Step 4: do data write
 *      Step 5: generate STOP by clearing MSTA bit
 *
 * @param base      i.MX i2c module base
 * @param rq        pointer to struct imx_i2c_request
 * @param dir       I2C_READ/I2C_WRITE
 *
 * @return          0 on success; non-zero otherwise
 */
int i2c_xfer(u32 base, struct imx_i2c_request *rq, int dir)
{
    unsigned int reg;
    unsigned char i, data;
    unsigned short i2cr;

    int ret = 0;

    if (rq->buffer_sz == 0 || rq->buffer == NULL) {
        printf("Invalid register address size=%x, buffer size=%x, buffer=%x\n",
               rq->reg_addr_sz, rq->buffer_sz, (unsigned int)rq->buffer);
        return -1;
    }

    /* Check if bus is free, if not return error */
    if (is_bus_free(base) != 0) {
        return -1;
    }
    // reset and enable I2C
    writew(0, base + I2C_I2CR);
    writew(I2C_I2CR_IEN, base + I2C_I2CR);
    /* Need wait at least 2 cycles of per_clk */
    hal_delay_us(5000);

    // 1.2 clear both IAL and IIF bits
    writew(0, base + I2C_I2SR);

    // 1.3 assert START signal and also indicate TX mode
    i2cr = I2C_I2CR_IEN;
    writew(i2cr, base + I2C_I2CR);
    i2cr = I2C_I2CR_IEN | I2C_I2CR_MTX;
    writew(i2cr, base + I2C_I2CR);
    i2cr = I2C_I2CR_IEN | I2C_I2CR_MSTA | I2C_I2CR_MTX;
    writew(i2cr, base + I2C_I2CR);

    // 1.4 make sure bus is busy after the START signal
    if (wait_till_busy(base) != 0) {
        I2CDBG("1\n");
        return -1;
    }
    // Step 2: send slave address + read/write at the LSB
    data = (rq->dev_addr << 1) | I2C_WRITE;
    I2CDBG("sending slave address (8-bit): 0x%X\n", data);

    if ((ret = tx_byte(&data, base)) != 0) {
        I2CDBG("START TX ERR %d\n", ret);

        if (ret == ERR_NO_ACK) {
        	I2CDBG("Error %d: no ack received\n", __LINE__);
            return ERR_NO_ACK_ON_START;
        } else {
            return ret;
        }
    }
    // Step 3: send I2C device register address
    if (rq->reg_addr_sz > 4) {
        printf("Warning register address size %d should less than 4\n", rq->reg_addr_sz);
        rq->reg_addr_sz = 4;
    }

    reg = rq->reg_addr;

    for (i = 0; i < rq->reg_addr_sz; i++, reg >>= 8) {
        data = reg & 0xFF;
        I2CDBG("sending device register: 0x%x, byte %d\n", base, data, i);
        I2CDBG("sending device register (%d of %d): 0x%X\n", i+1, rq->reg_addr_sz, data);

        if (tx_byte(&data, base) != 0) {
            return -1;
        }
    }

    // Step 4: read/write data
    if (dir == I2C_READ) {
        // do repeat-start
        i2cr = readw(base + I2C_I2CR);
        writew(i2cr | I2C_I2CR_RSTA, base + I2C_I2CR);

        // make sure bus is busy after the REPEATED START signal
        if (wait_till_busy(base) != 0) {
            return ERR_TX;
        }
        // send slave address again, but indicate read operation
        data = (rq->dev_addr << 1) | I2C_READ;

        I2CDBG("sending slave address (8-bit) again: 0x%X\n", data);
        if (tx_byte(&data, base) != 0) {
            return -1;
        }
        // change to receive mode
        i2cr = readw(base + I2C_I2CR);

        if (rq->buffer_sz == 1) {
            // if only one byte to read, make sure don't send ack
            i2cr |= I2C_I2CR_TXAK;
        }

        writew(i2cr & ~I2C_I2CR_MTX, base + I2C_I2CR);
        // dummy read
        readw(base + I2C_I2DR);

        // now reading ...
        if (rx_bytes(rq->buffer, base, rq->buffer_sz) != 0) {
            return -1;
        }
    } else {
        // I2C_WRITE
        for (i = 0; i < rq->buffer_sz; i++) {
            // send device register value
            data = rq->buffer[i];

            I2CDBG("sending data (%d of %d): 0x%X\n", i+1, rq->buffer_sz, data);
            if ((ret = tx_byte(&data, base)) != 0) {
                break;
            }
        }

        // generate STOP by clearing MSTA bit
        imx_send_stop(base);
    }

    /* Check if bus is free, if not return error */
    if (is_bus_free(base) != 0) {
        printf("WARNING: bus is not free\n");
    }
    writew(0, base + I2C_I2CR);

    return ret;
}

/*!
 * Initialize and enable a i2c module -- mainly enable the I2C clock, module
 * itself and the I2C clock prescaler.
 *
 * @param   base        base address of i2c module (also assigned for I2Cx_CLK)
 * @param   baud        the desired data rate
 *
 * @return              0 if successful; non-zero otherwise
 */
#if 0
int i2c_init(u32 base, u32 baud)
{
    io_cfg_i2c(base);

    // reset I2C
    writew(0, base + I2C_I2CR);

    // since we find a valid divider, it is time to configure the register
    writew(0x14, base + I2C_IFDR);
    writew(I2C_I2CR_IEN, base + I2C_I2CR);

    return 0;
}
#endif
