
void start_pmccntr(void)
{
    /* Set PMCR.D bit to enable (64 cycles/count), write 1 to PMCR.C to cycle counter reset */
    asm("mov r1, #12");
    asm("mcr p15, 0, r1, c9, c12, 0");

    /* Set PMCNTENSET.C bit to enable cycle counter */
    asm("ldr r1,=0x80000000");
    asm("mcr p15, 0, r1, c9, c12, 1");

    /* Set PMCR.D and PMCR.E bit to enable cycle counter */
    asm("mov r1, #9");
    asm("mcr p15, 0, r1, c9, c12, 0");
}


unsigned int read_stop_pmccntr(void)
{
    unsigned int counter = 0;

    /* read PMCCNTR  */
    asm volatile ("mrc p15, 0, %0, c9, c13, 0" : "=r"(counter):);

    /*set C bit of PMCNTENCLR to disable the cycle counter */
    asm("ldr r1,=0x80000000");
    asm("mcr p15, 0, r1, c9, c12, 2 ");

    return counter*64;
}

