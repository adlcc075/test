/*
 * Copyright (C) 2011, Freescale Semiconductor, Inc. All Rights Reserved
 * THIS SOURCE CODE IS CONFIDENTIAL AND PROPRIETARY AND MAY NOT
 * BE USED OR DISTRIBUTED WITHOUT THE WRITTEN PERMISSION OF
 * Freescale Semiconductor, Inc.
*/

extern void dcc3_put_block(const unsigned char * data, int size);
void dcc3_get_block(unsigned char * data, int size);

unsigned char term_getchar(void)
{
	unsigned char imsg[4];

	dcc3_get_block(imsg,1);

	return imsg[0];
}

void term_putchar( unsigned char *ch)
{
	dcc3_put_block((unsigned char *)ch,1);
}

void term_puts(const char * buffer)
{
	int i;

	for (i = 0; buffer[i] ; i++);

	dcc3_put_block((unsigned char *) buffer, i);
}
