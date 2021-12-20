#include <clint.h>
#include <common.h>
#include <riscv64.h>

//soft intterupt
//set MSIPR/SSIPR bit0 1 soft interrupt
//clear MSIPR/SSIPR bit0 0 soft interrupt

void clint_soft_irq_init(void)
{
    csr_clear(mie, MIP_MTIP | MIP_MSIP);
    csr_set(mie,  MIP_MSIP); //set m-mode sip
}

void clint_soft_irq_start(void)
{
    write32(CLINT, 1);
}

void clint_soft_irq_clear(void)
{
    write32(CLINT, 0);
}

//timer
//riscv need 64 bit mtime
void clint_timer_init()
{
    uint64_t cur_cnt = counter();
    uint32_t tick_l = (cur_cnt & 0xffffffff);
    uint32_t tick_h = (cur_cnt >> 32) & 0xffffffff;

    csr_clear(mie, MIP_MTIP | MIP_MSIP);
    write32(CLINT + 0x4000, tick_l);
    write32(CLINT + 0x4004, tick_h);
    csr_set(mie,  MIP_MTIP);

    
}

#define TIMER_FRQ (24000000)
void clint_timer_cmp_set_val(int val)
{
    //MIE
    // 17   16-12   11  10  9    8   7    6    5     4    3      2     1    0
    //MOIE         MEIE    SEIE     MTIE      STIE       MSIE         SSIE
    //now we can user MTIE
    uint64_t cur_cnt = counter() + TIMER_FRQ; //1s
    uint32_t tick_l = (cur_cnt & 0xffffffff);
    uint32_t tick_h = (cur_cnt >> 32) & 0xffffffff;
    *(uint32_t*)CLINT_MTIMECMPL(0) = tick_l;
    *(uint32_t*)CLINT_MTIMECMPH(0) = tick_h;

}



