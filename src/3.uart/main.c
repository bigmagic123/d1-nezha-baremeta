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
    sys_clock_init();
    sys_uart0_init();
    printf("hello world\n\r");
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
