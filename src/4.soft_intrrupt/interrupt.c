#include <interrupt.h>
#include <common.h>
#include <clint.h>

void irq_handle_trap(uint32_t mcause, uint32_t epc)
{
    all_interrupt_disable();
    printf("mcause:%llx,epc:%llx\n\r", mcause, epc);
    clint_soft_irq_clear();
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
