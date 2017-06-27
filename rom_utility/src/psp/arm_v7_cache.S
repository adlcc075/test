	.equ	I_C_DISABLE_MASK,         0xFFFFEFFB
 	.equ	I_DISABLE_MASK,           0xFFFFEFFF
 	.equ	ENABLE_CACHE,             0x01
 	.equ	ARM_CTRL_DCACHE,          (1 << 2)
 	.equ	ARM_CTRL_FLOW,            (1 << 11)
 	.equ	ARM_CTRL_ICACHE,          (1 << 12)
 	.equ	ARM_CTRL_VECTORS,         (1 << 13)
	.equ	ARM_CTRL_MMU,             (1 << 0x0)
	.equ	ARM_AUXCR_L2EN,           (1 << 1)
	.equ	GPIO_INTERRUPT_MASK,      (0x1 << 12)
	.equ	SRC_SBMR2,                0x1C
	.equ	SRC_PERSISTENT_STATUS_OFFSET,  (0x24)
	.equ	SRC_PERSISTENT_STATUS_MASK,    (1 << 24)
	.equ	L2CC_AUX_CTRL_WAYSIZE_CLR_MASK,    ~(7 << 17)
	.equ  	L2CC_AUX_CTRL_WAYSIZE_MASK,        (3 << 17)

	.equ	pl310_base_address,  		0xa02000
	.equ	scu_base_address,    		0xa00000

	.text
	.code 32

	.global pu_irom_boot_l1icache_disable
	.func pu_irom_boot_l1icache_disable
pu_irom_boot_l1icache_disable:
    MRC     p15, 0, r0, c1, c0, 0       /*; read CP15 register 1 into r0*/
    LDR     r2, =I_DISABLE_MASK       /*; disable I Cache(Bit 12)*/
    AND     r0,r0,r2
    MCR     p15, 0, r0, c1, c0, 0       /*; write CP15 register 18 */

    BLX              lr
	.endfunc

	.global	hapi_cache_clean_invalidate
	.func hapi_cache_clean_invalidate
hapi_cache_clean_invalidate:   
    stmfd   sp!, {r0-r12, lr}
    MOV     r0,#0
    MCR     p15,2,r0,c0,c0,0  //; cache size selection register, select dcache
    ISB
    MRC     p15,1,r0,c0,c0,0  //; cache size ID register
    MOV     r0,r0,ASR #13
    LDR     r3,=0x7fff
    AND     r0,r0,r3
    CMP     r0,#0x7f
    MOVEQ   r6,#0x1000
    BEQ     clean_size_done
    CMP     r0,#0xff
    MOVEQ   r6,#0x2000
    MOVNE   r6,#0x4000

clean_size_done:
    MOV     r2,#0
    MOV     r3,#0x40000000
    MOV     r4,#0x80000000
    MOV     r5,#0xc0000000

clean_d_inv_loop:
    MCR     p15,0,r2,c7,c14,2  //; invalidate dcache by set / way
    MCR     p15,0,r3,c7,c14,2  //; invalidate dcache by set / way
    MCR     p15,0,r4,c7,c14,2  //; invalidate dcache by set / way
    MCR     p15,0,r5,c7,c14,2  //; invalidate dcache by set / way
    ADD     r2,r2,#0x20
    ADD     r3,r3,#0x20
    ADD     r4,r4,#0x20
    ADD     r5,r5,#0x20

    CMP     r2,r6
    BNE     clean_d_inv_loop

//; skip L2 cache clean invalidate if used as OCRAM or not enabled
    LDR r1,=pl310_base_address
    LDR r4, [r1, #0x100]
    TST r4, #0x1 //; Z=1 if bit 0 is not set
    BEQ l2cache_clinv_skip

//    ;  Clean Invalidate L2 cache
    LDR r4,=0xFFFF      //; works for 8 and 16 ways associativity
    STR r4,[r1,#0x7FC]
clean_l2_inv_loop:
    LDR r2,[r1,#0x7FC]
    TST r2,r4
    BNE clean_l2_inv_loop

l2cache_clinv_skip:
    ldmfd   sp!, {r0-r12, lr}
    bx      lr
	.endfunc

	.global hapi_mmu_disable
	.func hapi_mmu_disable
hapi_mmu_disable:   
//;==================================
    stmfd   sp!, {r0-r12,lr}        //;push register onto stack

    MRC     p15, 0, r0, c1, c0, 0   //; read CP15 register 1 into r0
    ANDS    r0, r0, #0x1            //; check if MMU is enabled
    BEQ     mmu_disable_done        //; exit if MMU is already disabled

//; Invalidate I cache
    MOV     r0,#0
    MCR     p15,0,r0,c7,c5,0

//; invalidate Inst TLB & Data TLB
    MOV     r0,#0
    MCR     p15,0,r0,c8,c7,0

//; Invalidate data caches
    bl hapi_cache_clean_invalidate

//; Disable caches, MMU
    MRC     p15, 0, r0, c1, c0, 0           //; read CP15 register 1 into r0
    BIC     r0, r0, #(ENABLE_CACHE << 2)    //; disable D Cache
    BIC     r0, r0, #0x1                    //; clear bit 0 ; MMU off

    BIC     r0, r0, #(0x1 << 11)            //; disable Z, branch prediction
    BIC     r0, r0, #(0x1 << 1)             //; disable A, Strict alignment check enabled.

    MCR     p15, 0, r0, c1, c0, 0           //; write CP15 register 1
    MOV     r0, r0
    MOV     r0, r0
    MOV     r0, r0
    MOV     r0, r0

    LDR r1,=pl310_base_address
    LDR r0, [r1, #0x100]
    TST r0, #0x1 ; 
    BEQ l2cache_disable_skip

//; disable L2
    MOV r0,#0
    STR r0,[r1,#0x100]

//; disable SCU
l2cache_disable_skip:
    LDR     r1,=scu_base_address
    LDR     r0,[r1]
    AND     r0,r0,#(~0x1)
    STR     r0,[r1]

//; invalidate SCU tag RAMs
    LDR     r0,=0xffff
    STR     r0,[r1, #0xc]

mmu_disable_done:
    ldmfd   sp!, {r0-r12,lr}
    bx      lr
	.endfunc

    .end
