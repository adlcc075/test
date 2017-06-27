#ifndef _IMX_SPI_NOR_H_
#define _IMX_SPI_NOR_H_

#include "io.h"

#define JEDEC_ID    0x9F        // read JEDEC ID. tx: 1 byte cmd; rx: 3 byte ID


typedef struct chip_id {
    char id0;
    char id1;
    char id2;
} chip_id_t, *chip_id_p;

typedef enum {
    ATMEL_AT45DB321D,
    MICRON_M25P32,
    MICRON_N25Q32,
    ATMEL_AT45DB021E,
} spinor_dev_type_en;

typedef struct {
    chip_id_t id;
    char *name;
    spinor_dev_type_en type;
    int (*erase) (u32, u32);
    int (*read) (void *, void *, int);
    int (*write) (u32, u8 *, u32);
} spinor_info_t, *spinor_info_p;

int spi_nor_get_id(u32 *data);
int spi_nor_init(u32 base, u32 ss);
int spi_nor_write(u32 addr, u8 * data, u32 length);
int spi_nor_read(void *src, void *dest, int len);
int spi_nor_erase(u32 addr, u32 length);

#endif
