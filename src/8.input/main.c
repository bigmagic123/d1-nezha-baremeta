#include <stdio.h>
#include <stdlib.h>
#include <riscv64.h>
#include <printf.h>
#include <uart.h>
#include <common.h>
#include <clk.h>
#include "gpio.h"

void _putchar(char character)
{
    sys_uart_putc(0, character);
    // send char to console etc.
}

#define PLIC_UART0_NUM  (18)
#define PLIC_UART1_NUM  (19)
#define PLIC_UART2_NUM  (20)
#define PLIC_UART3_NUM  (21)
#define PLIC_UART4_NUM  (22)
#define PLIC_UART5_NUM  (23)

#define GPIO_INPUT      (0x00)
#define GPIO_OUTPUT     (0x01)

int main(void)
{
    char ch = -1;
    int cnt = 0;
    sys_clock_init();
    sys_uart0_init();
    table_val_set();
    printf("hello world\n\r");
    all_interrupt_enable();

    d1_set_gpio_mode(GPIO_PORT_B, GPIO_PIN_5, GPIO_INPUT);

    int read_val = 0;
    while(1)
    {
        sdelay(1000 * 1000);//1s
        read_val = d1_get_gpio_val(GPIO_PORT_B, GPIO_PIN_5);
        printf("read gpio is %d\n", read_val);
    }
    return 0;
}
