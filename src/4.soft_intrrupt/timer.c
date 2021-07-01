#include <timer.h>
#include <riscv64.h>


void timer_init(void)
{
    //MIE
    // 17   16-12   11  10  9    8   7    6    5     4    3      2     1    0
    //MOIE         MEIE    SEIE     MTIE      STIE       MSIE         SSIE
    //now we can user MTIE
    csr_clear(mie, MIP_MTIP | MIP_MSIP);
    //*(uint64_t*)CLINT_MTIMECMP(r_mhartid()) = *(uint64_t*)CLINT_MTIME + interval;
    csr_set(mie,  MIP_MSIP);
}
