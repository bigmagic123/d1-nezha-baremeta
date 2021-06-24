#include <interrupt.h>
#include <common.h>

uint32_t irq_handle_trap(uint32_t mcause, uint32_t epc)
{
    printf("irq!\n");	
    return 0;
}

void irq_enable()
{

    return 0;
}

void irq_disable()
{

    return 0;
}
