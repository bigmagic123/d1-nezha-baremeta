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

int main(void)
{
    char ch = -1;
    int cnt = 0;
    sys_clock_init();
    sys_uart0_init();
    table_val_set();
    printf("hello world\n\r");
    all_interrupt_enable();
    printf("enter ok!\n");
    clint_timer_init();
    while(1)
    {
        ch = sys_uart_getc(0);
        if(ch != 0xff)
        {
            printf("%c\n\r", ch);
        }
        sdelay(1000);
    }
    return 0;
}
