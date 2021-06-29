#include <stdio.h>
#include <stdlib.h>
#include <riscv64.h>
#include <printf.h>
#include <uart.h>
#include <common.h>
#include <clk.h>

void _putchar(char character)
{
    sys_uart_putc(0, character);
    // send char to console etc.
}

// #define CSR_MCOR         0x7c2
// #define CSR_MHCR         0x7c1
// #define CSR_MCCR2        0x7c3
// #define CSR_MHINT        0x7c5
// #define CSR_MXSTATUS     0x7c0
// #define CSR_PLIC_BASE    0xfc1
// #define CSR_MRMR         0x7c6
// #define CSR_MRVBR        0x7c7

int main(void)
{
    char ch = -1;
    sys_clock_init();
    sys_uart0_init();
    table_val_set();
    printf("hello world\n\r");
    //all_interrupt_enable();
    //timer_init();
    printf("enter ok!\n");
    //csr_read(CSR_PLIC_BASE);
    while(1)
    {
        ch = sys_uart_getc(0);
        if(ch != 0xff)
        {
            printf("%c\n\r", ch);
        }
    }
    return 0;
}
