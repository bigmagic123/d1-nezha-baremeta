#include <clint.h>
#include <common.h>

//soft intterupt
//set MSIPR/SSIPR bit0 1 soft interrupt
//clear MSIPR/SSIPR bit0 0 soft interrupt

//timer
//riscv need 64 bit mtime


void d1_clint_timer_init()
{

}

void d1_clint_soft_irq_init()
{

}

void d1_clint_timer_cmp_set_val(int val)
{
    // int time_cnt = read32(D1_MTIME);
    // printf("time_cnt is %d\n", time_cnt);
    // sdelay(1000);
    // time_cnt = read32(D1_MTIME);

    //write32(D1_MSIP0, 1);
    write32(CLINT, 1);
    
    //printf("time_cnt is %d\n", soft);
    //int time_cnt = read32(CLINT);
    //printf("time_cnt is %d\n", time_cnt);

    //while(1);
}


void d1_clint_soft_irq_clear(void)
{
    //write32(D1_MSIP0, 0);
    write32(CLINT, 0);
    // int time_cnt = read32(CLINT);
    // printf("time_cnt is %d\n", time_cnt);
}