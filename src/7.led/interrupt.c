#include <interrupt.h>
#include <common.h>
#include <clint.h>
#include "riscv_encoding.h"
//mcause info
/*
Interrupt Exception Code  Description
1            0      User  software interrupt
1            1 Supervisor software interrupt
1            2 Hypervisor software interrupt
1            3 Machine software interrupt
1            4 User timer interrupt
1            5 Supervisor timer interrupt
1            6 Hypervisor timer interrupt
1            7 Machine timer interrupt
1            8 User external interrupt
1            9 Supervisor external interrupt
1            10 Hypervisor external interrupt
1            11 Machine external interrupt
1 >>12 Reserved
0            0 Instruction address misaligned
0            1 Instruction access fault
0            2 Illegal instruction
0            3 Breakpoint
0            4 Load address misaligned
0            5 Load access fault
0            6 Store/AMO address misaligned
0            7 Store/AMO access fault
0            8 Environment call from U-mode
0            9 Environment call from S-mode
0            10 Environment call from H-mode
0            11 Environment call from M-mode
0 >>12 Reserved

*/

#define M_MODE_EXT_IRQ  11
void irq_handle_trap(uint64_t mcause, uint64_t epc)
{
    all_interrupt_disable();
    printf("mcause:%llx,epc:%llx\n\r", mcause, epc);
    if(mcause & (1UL << 63))
    {
      int irq_cause = (mcause & 0xffffffff);
      switch(irq_cause)
      {
         //Machine software interrupt
         case 3:
            clint_soft_irq_clear();
            break;
         //Machine timer interrupt
         case 7:
            clint_timer_cmp_set_val(1000);
            break;
         case 11:
            plic_handle_irq();
            break;
         default:
            break;
      }
       //clint_soft_irq_clear();
    }
    else
    {
       printf("trap!!! is %p\n", read_csr(mstatus));
       //trap
    }
    
    all_interrupt_enable();
}

void irq_enable()
{

   // return 0;
}

void irq_disable()
{

   // return 0;
}
