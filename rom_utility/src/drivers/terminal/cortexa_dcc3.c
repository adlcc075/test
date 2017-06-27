/*
 * Copyright (C) 2011, Freescale Semiconductor, Inc. All Rights Reserved
 * THIS SOURCE CODE IS CONFIDENTIAL AND PROPRIETARY AND MAY NOT
 * BE USED OR DISTRIBUTED WITHOUT THE WRITTEN PERMISSION OF
 * Freescale Semiconductor, Inc.
*/

unsigned int dcc3_get_term_status(void)
{
	unsigned int status = 0;
	
	__asm__ volatile("mrc 14, 0, %0, c0, c1" :"=r"(status));

	return status;
}

void dcc3_put_block(const unsigned char * data, int size)
{
    int             len;
    unsigned long   ch, k;

    while (size > 0) {
	len = size;
	if (len > 3)
	    len = 3;
	ch = data[0] | (data[1] << 8) | (data[2]) << 16 | ((len - 1) << 24);

	do{	/* Wait for Terminal Ready */
		__asm__ volatile("mrc 14, 0, %0, c0, c1" :"=r"(k));
	}while(k & 0x20000000);

	__asm__ volatile("mcr 14, 0, %0, c0, c5" : :"r"(ch));

	data += len;
	size -= len;
    }
}

void dcc3_get_block(unsigned char * data, int size)
{
    static int      bufsize = 0;
    static int      bufindex;
    static unsigned char buf[4];
    int             i, j;
    unsigned long   ch;

    while (size > 0) {
	if (!bufsize) {
	    do{		/*Wait for input byte*/
		__asm__ volatile("mrc 14, 0, %0, c0, c1" :"=r"(ch));
	    }while(!(ch & 0x40000000));

	    __asm__ volatile("mrc 14, 0, %0, c0, c5" :"=r"(ch));
	    buf[0] = ch & 0xff;
	    buf[1] = (ch >> 8) & 0xff;
	    buf[2] = (ch >> 16) & 0xff;
	    bufsize = (ch >> 24) + 1;
	    bufindex = 0;
	}
	i = bufsize;
	if (i > size)
	    i = size;

	for (j = 0; j < i; j++)
	    data[j] = buf[bufindex + j];

	bufindex += i;
	bufsize -= i;
	data += i;
	size -= i;
    }
}
