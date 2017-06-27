#include "platform.h"
#include "debug.h"
#include "shell/shell.h"

#define LED_PIN				0x00002000

#define CPU_FREQ			1000000
#define DELAY_DIV			10

extern chip_e chip;

char mbench_help[] =
	"Usage: mbench [<base_addr>] [<mask>]\rn"
	"   Random access the memory to test its performance\r\n"
	"	e.g. For QSPI use base address 0x70000000"
	"	Default will perform an OCRAM and QSPI speed test";

uint32_t get_cyclecount(void)
{
	uint32_t reg;

	// read CCNT register
	asm volatile ("MRC p15, 0, %0, c9, c13, 0\n\t": "=r"(reg));

	return reg;
}

void init_perfcounters(void)
{
	uint32_t reg = 0;

	/* disable counter overflow interrupts (just in case)*/
	asm volatile ("MCR p15, 0, %0, C9, C14, 2\n\t" :: "r"(0x8000000f));

	// in general enable all counters (including cycle counter)
	reg = 1;

	// perform reset:  
	reg |= 2;     // reset all counters to zero.
	reg |= 4;     // reset cycle counter to zero.
	reg |= 8;     // enable "by 64" divider for CCNT.
	reg |= 16;

	// program the performance-counter control-register:
	asm volatile ("MCR p15, 0, %0, c9, c12, 0\n\t" :: "r"(reg));

	// enable all counters:
	asm volatile ("MCR p15, 0, %0, c9, c12, 1\n\t" :: "r"(0x8000000f)); 

	// clear overflows:
	asm volatile ("MCR p15, 0, %0, c9, c12, 3\n\t" :: "r"(0x8000000f));
}

// this function provides a simple 3 instruction cycle loop a given of times:
void short_delay(uint32_t count)
{
	asm volatile ("SUB r0, #1\n\t" :: "r"(count));
// 		"BNE short_delay\n\t"
// 		"BX lr\n\t");
}

void enable_neon(void)
{
	asm volatile (
		"LDR r0, =(0xF << 20)\n\t"
		"MCR p15, 0, r0, c1, c0, 2\n\t"
		"MOV r3, #0x40000000\n\t"
		".long 0xeee83a10\n\t"
		//"VMSR FPEXC, r3\n\t"
		"BX  lr\n\t");
}
 
int do_mbench(unsigned int argc, char *argvs[])
{
	// for pele only:
	volatile uint32_t *gpio_dir_reg = (volatile uint32_t *)0x020A0004;
	volatile uint32_t *gpio_data_reg = (volatile uint32_t *)0x020A0000;

	uint32_t r_dir;
	uint32_t r_reg;

	uint32_t seed;
	volatile uint32_t var;
	uint8_t *iram_base, *qspi_base;
	uint32_t iram_size, qspi_size;

	unsigned char i_bench;

	uint32_t t_overhead, t_s, t_e, t_d;

// 	if (argc < 2) {
// 		printf(mbench_help);
// 		return -1;
// 	}

#define RANDOM_MEM_READ(base, size) {	\
		seed ^= (seed << 13);			\
		seed ^= (seed >> 17);			\
		seed ^= (seed << 5);			\
		var = base[seed & size];		\
	}	// xorshift algorithm

	init_perfcounters();

	printf("enable neon..\r\n");
	enable_neon();
	printf("neon enabled\r\n");

	t_overhead = get_cyclecount();
	t_overhead = get_cyclecount() - t_overhead;

	// configure gpio direction:
	r_dir = *gpio_dir_reg;
	r_dir |= LED_PIN;
	*gpio_dir_reg = r_dir;

	iram_base = (uint8_t *)OCRAM_ARB_BASE_ADDR_MX6SX;
	qspi_base = (uint8_t *)QSPI1_ARB_BASE_ADDR_MX6SX;
	iram_size = OCRAM_ARB_END_ADDR - OCRAM_ARB_BASE_ADDR_MX6SX;
	qspi_size = QSPI1_ARB_END_ADDR - QSPI1_ARB_BASE_ADDR_MX6SX;

// 	(void)(iram_base);
// 	(void)(qspi_base);
// 	(void)(iram_size);
// 	(void)(qspi_size);

	printf("\r\nbenchmark start\r\n");

	for (i_bench = 0; i_bench < 0xFF; i_bench ++)
	{
		// LED ON:
		/* read */
		r_reg = *gpio_data_reg;
		/* or */
		r_reg |= LED_PIN;
		/* write */
		*gpio_data_reg = r_reg;

		t_s = get_cyclecount();
		RANDOM_MEM_READ(iram_base, iram_size);
		RANDOM_MEM_READ(iram_base, iram_size);
		RANDOM_MEM_READ(iram_base, iram_size);
		RANDOM_MEM_READ(iram_base, iram_size);
		RANDOM_MEM_READ(iram_base, iram_size);
		RANDOM_MEM_READ(iram_base, iram_size);
		RANDOM_MEM_READ(iram_base, iram_size);
		RANDOM_MEM_READ(iram_base, iram_size);
		RANDOM_MEM_READ(iram_base, iram_size);
		RANDOM_MEM_READ(iram_base, iram_size);
		RANDOM_MEM_READ(iram_base, iram_size);
		RANDOM_MEM_READ(iram_base, iram_size);
		RANDOM_MEM_READ(iram_base, iram_size);
		RANDOM_MEM_READ(iram_base, iram_size);
		RANDOM_MEM_READ(iram_base, iram_size);
		RANDOM_MEM_READ(iram_base, iram_size);
		t_e = get_cyclecount();
		t_d = t_e - t_s - t_overhead;

		printf("MEM: %d\r\n", t_d);

		t_s = get_cyclecount();
		RANDOM_MEM_READ(qspi_base, qspi_size);
		RANDOM_MEM_READ(qspi_base, qspi_size);
		RANDOM_MEM_READ(qspi_base, qspi_size);
		RANDOM_MEM_READ(qspi_base, qspi_size);
		RANDOM_MEM_READ(qspi_base, qspi_size);
		RANDOM_MEM_READ(qspi_base, qspi_size);
		RANDOM_MEM_READ(qspi_base, qspi_size);
		RANDOM_MEM_READ(qspi_base, qspi_size);
		RANDOM_MEM_READ(qspi_base, qspi_size);
		RANDOM_MEM_READ(qspi_base, qspi_size);
		RANDOM_MEM_READ(qspi_base, qspi_size);
		RANDOM_MEM_READ(qspi_base, qspi_size);
		RANDOM_MEM_READ(qspi_base, qspi_size);
		RANDOM_MEM_READ(qspi_base, qspi_size);
		RANDOM_MEM_READ(qspi_base, qspi_size);
		RANDOM_MEM_READ(qspi_base, qspi_size);
		t_e = get_cyclecount();
		t_d = t_e - t_s - t_overhead;

		printf("QSPI: %d\r\n", t_d);

		short_delay(CPU_FREQ/DELAY_DIV);

		// LED OFF:
		/* read */
		r_reg = *gpio_data_reg;
		/* or */
		r_reg &= ~LED_PIN;
		/* write */
		*gpio_data_reg = r_reg;

		short_delay(CPU_FREQ);
	}

	(void)var;

	return 0;
}

SHELL_CMD(mbench, do_mbench, "mbench\t\tmemory benchmark\tUsage: mbench [<base_addr>] [<mask>]")
