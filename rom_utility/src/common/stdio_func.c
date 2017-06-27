/*
 * Copyright (C) 2012, Freescale Semiconductor, Inc. All Rights Reserved
 * THIS SOURCE CODE IS CONFIDENTIAL AND PROPRIETARY AND MAY NOT
 * BE USED OR DISTRIBUTED WITHOUT THE WRITTEN PERMISSION OF
 * Freescale Semiconductor, Inc.
*/

/*Implement the printf so stdio will not be used to save some code size*/
/*Avoid using divide and multiply*/

#ifdef USB_PRINT
extern char usb_putchar(unsigned char *ch);
extern char usb_getchar(void);
extern int usb_flush_std_dev_buf(void);
#else
extern void term_putchar(char *ch);
extern unsigned char term_getchar(void);
#endif

char send_char(unsigned char *ch)
{
#ifdef USB_PRINT
	usb_putchar(ch);
#else
    term_putchar((char *)ch);
#endif

    return *ch;
}

int get_char(unsigned char *ch)
{
#ifdef USB_PRINT
    while(0xFF == (*ch = usb_getchar()));
#else
    *ch = term_getchar();
#endif

    return 0;
}

const char hex_asc_table[16] =
    { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
const char upper_hex_asc_table[16] =
    { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

int is_dec_asc(char ch)
{
    unsigned int i;
    for (i = 0; i < 10; i++) {
        if (ch == hex_asc_table[i])
            return 1;
    }

    return 0;
}

int is_asc(char ch)
{
    return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
}

int is_hex_asc(char ch)
{
    return (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f');
}

int printf_char(char ch)
{
    unsigned char c1 = (unsigned char)'\r';
    unsigned char c2 = 0;

    c2 = (unsigned char)ch;

    send_char(&c2);

    if (((unsigned char)'\n') == c2) {
        send_char(&c1);
    }

    return 0;
}

int printf_str(char *str)
{
    while (*str != (char)'\0') {
        printf_char(*str);
        str++;
    }

    return 0;
}

/*
 * printf("Hello") will built to printf()
 * printf("Hello\n") will built to puts(). Since '\n' will be omitted in the .rodata section,
 * '\n' should be added in the puts.
 */
#if 0
int puts(char *str)
{
    printf_str(str);

    printf_char('\n');

#ifdef USB_PRINT
    usb_flush_std_dev_buf();
#endif

    return 0;
}

int putchar(char c)
{
    printf_char(c);

#ifdef USB_PRINT
    usb_flush_std_dev_buf();
#endif

    return 0;
}
#endif

int printf_hex(unsigned int val, unsigned int width)
{
    int i = 0;
    char hex_val = 0, asc = 0;

    if ((width > 8) || (width == 0))
        width = 8;

    for (i = width - 1; i >= 0; i--) {
        hex_val = (val & (0x0F << (i << 2))) >> (i << 2);
        asc = hex_asc_table[(int)hex_val];
        printf_char(asc);
    }

    return 0;
}

int printf_hex_upper(unsigned int val, unsigned int width)
{
    int i = 0;
    char hex_val = 0, asc = 0;

    if ((width > 8) || (width == 0))
        width = 8;

    for (i = width - 1; i >= 0; i--) {
        hex_val = (val & (0x0F << (i << 2))) >> (i << 2);
        asc = upper_hex_asc_table[(int)hex_val];
        printf_char(asc);
    }

    return 0;
}

#define MULTIPLY_DIVIDE_CAN_BE_USED
#ifdef MULTIPLY_DIVIDE_CAN_BE_USED
int printf_dec(unsigned int val)
{
    unsigned char buf[16];
    char asc = 0;
    int i = 0;
    while (1) {
        buf[i] = val % 10;
        val = val / 10;
        i++;
        if (val == 0) {
            break;
        }
    }

    for (; i > 0; i--) {
        asc = hex_asc_table[buf[i - 1]];
        printf_char(asc);
    }

    return 0;
}
#else
unsigned int hex_weight_value_table[] =
    { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000 };
int printf_dec(unsigned int val)
{
    unsigned int i = 0;
    unsigned int tmp = 1, tmp_w = 1;
    char dec_val = 0, asc = 0;

    char buf[16];

    while (1) {
        tmp = (tmp << 3) + (tmp << 1);  // tmp *= 10;
        i++;
        if (tmp >= val) {
            break;
        }
    }

    if (i > 8) {
        return -1;
    }

    while (i > 0) {
        if (val >= ((hex_weight_value_table[i - 1] << 3) + hex_weight_value_table[i - 1])) {    //<=9xxx
            tmp = 9;
            tmp_w = (hex_weight_value_table[i - 1] << 3) + hex_weight_value_table[i - 1];
        } else if (val >= (hex_weight_value_table[i - 1] << 3)) {   //8xxx
            tmp = 8;
            tmp_w = hex_weight_value_table[i - 1] << 3;
        } else if (val >=
                   ((hex_weight_value_table[i - 1] << 2) + (hex_weight_value_table[i - 1] << 1) +
                    hex_weight_value_table[i - 1])) {
            tmp = 7;
            tmp_w =
                (hex_weight_value_table[i - 1] << 2) + (hex_weight_value_table[i - 1] << 1) +
                hex_weight_value_table[i - 1];
        } else if (val >=
                   ((hex_weight_value_table[i - 1] << 2) + (hex_weight_value_table[i - 1] << 1))) {
            tmp = 6;
            tmp_w = (hex_weight_value_table[i - 1] << 2) + (hex_weight_value_table[i - 1] << 1);
        } else if (val >= ((hex_weight_value_table[i - 1] << 2) + hex_weight_value_table[i - 1])) {
            tmp = 5;
            tmp_w = (hex_weight_value_table[i - 1] << 2) + hex_weight_value_table[i - 1];
        } else if (val >= (hex_weight_value_table[i - 1] << 2)) {
            tmp = 4;
            tmp_w = hex_weight_value_table[i - 1] << 2;
        } else if (val >= ((hex_weight_value_table[i - 1] << 1) + hex_weight_value_table[i - 1])) {
            tmp = 3;
            tmp_w = (hex_weight_value_table[i - 1] << 1) + hex_weight_value_table[i - 1];
        } else if (val >= (hex_weight_value_table[i - 1] << 1)) {
            tmp = 2;
            tmp_w = hex_weight_value_table[i - 1] << 1;
        } else if (val >= (hex_weight_value_table[i - 1])) {
            tmp = 1;
            tmp_w = hex_weight_value_table[i - 1];
        } else {
            tmp = 0;
            tmp_w = 0;
        }

        asc = hex_asc_table[tmp];
        printf_char(asc);
        i--;

        val -= tmp_w;
    }

    return 0;
}
#endif

int easy_printf(const char *fmt, ...)
{
    char *pArg = 0;
    char c;
    unsigned int width = 0;

    pArg = (char *)&fmt;        //Get the address of the fmt
    pArg += sizeof(fmt);        //Get the address of 1st value

    do {
        c = *fmt;
        if (c != '%') {
            printf_char(c);
        } else {
            while (1) {
                c = *++fmt;
                if ((c == 'd') || (c == 'x') || (c == 'X') || (c == 's') || (c == 'c')) {
                    if ((c == 'd') || (c == 'x') || (c == 'X')) {
                        width = *(fmt - 1) - '0';
                    }
                    break;
                }
            }

            switch (c) {
            case 'd':
                printf_dec(*((int *)pArg));
                break;
            case 'x':
                printf_hex(*((int *)pArg), width);
                break;
            case 'X':
                printf_hex_upper(*((int *)pArg), width);
                break;
            case 's':
                printf_str((char *)(*((int *)pArg)));
                break;
            case 'c':
                printf_char(*((int *)pArg));
                break;
            default:
                break;
            }
            pArg += sizeof(int);
        }
        ++fmt;
    }
    while (*fmt != '\0');

#ifdef USB_PRINT
    usb_flush_std_dev_buf();
#endif

    pArg = 0;

    return 0;
}

int easy_memset(void *mem, unsigned char val, unsigned int sz)
{
    unsigned char *p = (unsigned char *)mem;
    int i = 0;

    for (i = 0; i < sz; i++, *p++ = val) ;

    return 0;
}

int easy_memcpy(void *dst, const void *src, unsigned int sz)
{
    unsigned char *p_dst = (unsigned char *)dst;
    unsigned char *p_src = (unsigned char *)src;
    int i = 0;

    for (i = 0; i < sz; i++, *p_dst++ = *p_src++) ;

    return 0;
}

int easy_memcmp(void *mem1, void *mem2, unsigned int sz)
{
    unsigned char *p_mem1 = (unsigned char *)mem1;
    unsigned char *p_mem2 = (unsigned char *)mem2;
    int i = 0;

    for (i = 0; i < sz; i++, p_mem1++, p_mem2++) {
        if (*p_mem1 != *p_mem2) {
            break;
        }
    }

    if (i < sz) {
        return 1;
    } else {
        return 0;
    }
}

int easy_strcmp(char *str1, char *str2)
{
    char *p1 = str1, *p2 = str2;

    while ((*p1 != 0) && (*p2 != 0) && (*p1 == *p2)) {
        p1++;
        p2++;
    }

    return *p1 - *p2;
}

int easy_strncmp(char *str1, char *str2, unsigned int sz)
{
    char *p1 = str1, *p2 = str2;
    int i = 0;

    for (; i < sz; i++, p1++, p2++) {
        if (*p1 > *p2) {
            return 1;
        }
        if (*p1 < *p2) {
            return -1;
        }
    }

    return 0;
}

int easy_strtoul(char *str, unsigned int *val)
{
    char *p = str;
    unsigned int multiplier = 10;
    unsigned int result = 0, num = 0;

    if (*p == '0' && (*(p + 1) == 'x' || *(p + 1) == 'X')) {
        p += 2;
        multiplier = 0x10;
    }

    while (0 != *p) {
        if (multiplier == 0x10) {
            if (!is_hex_asc(*p)) {
                return -1;
            }
            if (*p >= '0' && *p <= '9') {
                num = *p - '0';
            } else if (*p >= 'a' && *p <= 'f') {
                num = *p - 'a' + 10;
            } else {
                num = *p - 'A' + 10;
            }
        } else {
            if (!is_dec_asc(*p)) {
                return -2;
            }

            num = *p - '0';
        }

        result = result * multiplier + num;

        p++;
    }

    *val = result;

    return 0;
}

int easy_strtol(char *str, int *val)
{
    char *p = str;
	int ret;
	
	if(*p == '-') {
		ret = easy_strtoul(++p, (unsigned int *)val);
		*val = -(*val);
		return ret;
	} else
		return easy_strtoul(p, (unsigned int *)val);

}

unsigned int easy_strlen(char *str)
{
    char *p = str;

    while (*p++ != '\0') ;

    return (unsigned int)(p - str);
}

int easy_strcpy(char *dst, char *src)
{
    char *p_dst = dst, *p_src = src;

    while (*p_src != '\0') {
        *p_dst = *p_src;
        p_dst++;
        p_src++;
    }
    *p_dst = '\0';

    return 0;
}
