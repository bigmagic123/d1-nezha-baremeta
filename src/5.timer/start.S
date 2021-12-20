 
#include "riscv64.h"
#include "linkage.h"

.section ".start", "ax"

#define REGBYTES        4
/*mxstatus*/
#define    EN_THEADISAEE    (0x1 << 22)

.globl _start
_start:
    /*disable interrupt*/
    csrrci x0, mstatus, 0x8
    csrw mie, zero

#ifndef  RISCV64_QEMU
    /*enable theadisaee*/
    li t1, EN_THEADISAEE
    csrs mxstatus, t1

    /*invaild ICACHE/DCACHE/BTB/BHT*/
    li t2, 0x30013
    csrs mcor, t2
#endif
    la sp, __stack_end
    jal clear_bss
    jal  main
    j .

clear_bss:
    la    t0, __bss_start
    la    t1, __bss_end

clbss_1:
    sw zero, 0(t0)
    addi t0, t0, REGBYTES
    blt t0, t1, clbss_1
    ret
