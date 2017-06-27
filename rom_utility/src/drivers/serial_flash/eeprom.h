#ifndef _EEPROM_H_
#define _EEPROM_H_

#include "io.h"

int at24cx_read(u32 i2c_base_addr, u32 addr, u8 * buf, u32 sz);
int at24cx_write(u32 i2c_base_addr, u32 addr, u8 * buf, u32 sz);

#endif
