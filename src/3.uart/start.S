 
#include "riscv64.h"
#include "linkage.h"
    .global _start
_start:
    j reset
/*
 * The actual reset code
 */
reset:
    /* Mask all interrupts */
    csrw mideleg, zero /*machine exception delegation register*/
    csrw medeleg, zero /*machine interrupt delegation register*/
    csrw mie, zero	   /* mie set 0*/
    csrw mip, zero     /* mip set 0*/

    /* Enable FPU and accelerator if present */
    li t0, MSTATUS_FS | MSTATUS_XS | (0x01800000)
    csrs mstatus, t0

    /* Initialize global pointer */
.option push
.option norelax
    la t0, _global_pointer$
    ld gp, (t0)
.option pop

    /* Initialize stacks */
    la t1, _stack_start
	add  sp, t1, zero

    j main

_stack_start:
	RVPTR __stack_start
_global_pointer$:
	RVPTR __global_pointer$