#ifndef NOR_H
#define NOR_H

#include  "platform.h"
/*===================================================================================
                                        MARCROS
==================================================================================*/
/* manufacture code */
#define NOR_MFR_NUMONYX		(0x20)
#define NOR_MFR_MICRON		(0x89)

/* block size and chip size */
#define BLOCK_SIZE_128KB	(0x20000)
#define BLOCK_SIZE_256KB	(0x40000)

#define CHIP_SIZE_32MB		(0x2000000)
#define CHIP_SIZE_128MB		(0x8000000)

/* WEIM mux */
#define WEIM_CFG_AD16_MUXED			(0x0000)
#define WEIM_CFG_ADH16_NONMUXED		(0x4000)
#define WEIM_CFG_ADL16_NONMUXED		(0x8000)
#define WEIM_CFG_AD32_MUXED			(0xc000)
#define WEIM_CFG_AD16_MUXED_7D			(0x5000)
#define WEIM_CFG_ADL16_NONMUXED_7D		(0x5400)
/*===================================================================================
                                        ENUM AND STRUCTS
==================================================================================*/
typedef struct {
	void (*reset)(void);
	void (*unlock)(U32 block_addr);
	void (*read_id)(U8 *ids);
	BOOL (*erase_block)(U32 block_addr);
	BOOL (*erase_chip)(void);
	BOOL (*read_words)(U32 addr, U32 len, U16 *buf);
	BOOL (*write_words)(U32 addr, U16 *buf, U32 len);

} nor_cmdset_t;

typedef struct {
	U8	name[20];
    U8 	id[4];
	U32 block_size;
	U32 chip_size;
	nor_cmdset_t *cmdset;

} nor_cfg_t;

/* cpu configuration related to NOR flash */
typedef struct nor_bootcfg {
	U32 cpuid;
	U32 firmware_load_addr;
    BOOL (*weim_init)(void);
	U32 *hapi_src_reg;
	U32 ivt_offset;

} nor_bootcfg_t;

/*===================================================================================
                                        INLINE FUNCTIONS
==================================================================================*/
/* weim base address */
extern U32 regs_WEIM_CS_base;

static inline void send_cmd_w(U16 data, U32 addr)
{
	writew(data, regs_WEIM_CS_base + (addr << 1));
}

static inline void send_cmd_b(U8 data, U32 addr)
{
	writeb(data, regs_WEIM_CS_base + addr);
}

static inline U16 get_data_w(U32 addr)
{
	return readw(regs_WEIM_CS_base + (addr << 1));
}

static inline U8 get_data_b(U32 addr)
{
	return readb(regs_WEIM_CS_base + addr);
}

/*===================================================================================
                                        GLOBAL FUNCTIONS
==================================================================================*/
BOOL nor_probe(void);
void nor_cfg_dump(void);
BOOL nor_boot_chip_cfg(chip_e chip);

#endif
