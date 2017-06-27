#ifndef _QSPI_H_
#define _QSPI_H_

#include "io.h"
#include "platform.h"

extern int quadspi_test_init(uint32_t index);
extern int quadspi_test_program(uint32_t index, uint32_t src, uint32_t size, uint32_t seek, uint32_t skip);
extern int quadspi_test_erase_whole(uint32_t index);
extern int quadspi_test_erase_sector(uint32_t index, uint32_t addr);
extern int quadspi_test_data_check(uint32_t index, uint32_t src, uint32_t size, uint32_t init, uint32_t seek, uint32_t skip);

#endif
