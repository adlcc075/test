/*
 * Copyright (C) 2012, Freescale Semiconductor, Inc. All Rights Reserved
 * THIS SOURCE CODE IS CONFIDENTIAL AND PROPRIETARY AND MAY NOT
 * BE USED OR DISTRIBUTED WITHOUT THE WRITTEN PERMISSION OF
 * Freescale Semiconductor, Inc.
*/

#ifndef __TYPES_H__
#define __TYPES_H__

#ifndef NULL
#define NULL				0
#endif

typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

typedef long long int64_t;
typedef int int32_t;
typedef short int16_t;
typedef char int8_t;

typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

typedef signed long long s64;
typedef signed int s32;
typedef signed short s16;
typedef signed char s8;

typedef unsigned long long U64;
typedef unsigned int U32;
typedef unsigned short U16;
typedef unsigned char U8;

typedef int bool;

typedef unsigned int UINT32;
typedef unsigned short UINT16;
typedef unsigned char UINT8;

typedef unsigned char BOOL;
typedef BOOL BOOLEAN;

typedef unsigned int size_t;

#define true                1
#define false               0

#define TRUE               1
#define FALSE               0

#define SUCCESS (0)
#define FAIL (1)

#define NONE_CHAR           0xFF

#define REG8(a)             (*(volatile unsigned char *)(a))
#define REG16(a)            (*(volatile unsigned short *)(a))
#define REG32(a)            (*(volatile unsigned int *)(a))
#define SET_REG8(v, a)      (*(volatile unsigned char *)(a) = (v))
#define SET_REG16(v, a)     (*(volatile unsigned short *)(a) = (v))
#define SET_REG32(v, a)     (*(volatile unsigned int *)(a) = (v))
#define readb(a)            REG8(a)
#define readw(a)            REG16(a)
#define readl(a)            REG32(a)
#define writeb(v, a)        SET_REG8(v, a)
#define writew(v, a)        SET_REG16(v, a)
#define writel(v, a)        SET_REG32(v, a)

#define reg8_read 	readb
#define reg8_write(a, v)	writeb(v, a)
#define reg16_read 	readw
#define reg16_write(a, v)	writew(v, a)
#define reg32_read 	readl
#define reg32_write(a, v)	writel(v, a)

#define  reg32clrbit(addr,bitpos) \
         reg32_write((addr),(reg32_read((addr)) & (0xFFFFFFFF ^ (1<<(bitpos)))))
#define  reg32setbit(addr,bitpos) \
         reg32_write((addr),(reg32_read((addr)) | (1<<(bitpos))))

#define  reg32_read_bit(addr,bit_offset) (reg32_read(addr) & (0x1<<(bit_offset)))

#define  reg32_write_mask(addr, data, msk)	\
		reg32_write(addr, (reg32_read(addr) & (~(msk))) | ((msk) & (data)))

struct hw_module {
    char *name;
    u32 base;
    u32 freq;
};

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#define STRUCT_OFFSET(structure, member)	((UINT32)(&(((structure *)0)->member)))

int easy_printf(const char *fmt, ...);
int easy_memset(void *mem, unsigned char val, unsigned int sz);
int easy_memcpy(void *dst, const void *src, unsigned int sz);
int easy_strcmp(char *str1, char *str2);
int easy_strncmp(char *str1, char *str2, unsigned int sz);
int easy_strtoul(char *str, unsigned int *val);
int easy_strtol(char *str, int *val);
unsigned int easy_strlen(char *str);
int easy_strcpy(char *dst, char *src);
int easy_memcmp(void *mem1, void *mem2, unsigned int sz);

#define printf easy_printf
#define memset easy_memset
#define memcpy easy_memcpy
#define memcmp easy_memcmp
#define strcmp easy_strcmp
#define strncmp easy_strncmp
#define strtoul easy_strtoul
#define strtol easy_strtol
#define strlen	easy_strlen
#define strcpy	easy_strcpy

int get_char(unsigned char *ch);

typedef void (*f_v_v)(void);
typedef u8 (*f_u8_v)(void);
typedef u8 (*f_u8_u32p_u32_u32)(u32 *, u32, u32);
typedef u32 (*f_u32_v)(void);
typedef void (*f_ecspi_init)(u32);
typedef void (*f_v_u32_u32)(u32, u32);
typedef void (*f_v_u32)(u32);
typedef BOOL (*f_b_v)(void);

#endif
