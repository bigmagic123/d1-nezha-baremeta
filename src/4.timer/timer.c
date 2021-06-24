#include <timer.h>
#include <riscv64.h>


void timer_init(void)
{
    csr_clear(mie, MIP_MTIP);
    //*(uint64_t*)CLINT_MTIMECMP(r_mhartid()) = *(uint64_t*)CLINT_MTIME + interval;
    csr_set(mie, MIP_MTIP);
}
