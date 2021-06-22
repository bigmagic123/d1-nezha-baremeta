#include <stdio.h>
#include <stdlib.h>
#include <riscv64.h>
#include <clk.h>
#include <printf.h>
#include <uart.h>
#include <gpio.h>
#include <common.h>

//typedef unsigned int virtual_addr_t;
//typedef unsigned int u32_t;
void sys_uart_putc(char c);

void _putchar(char character)
{
    sys_uart_putc(character);
    // send char to console etc.
}

#define UART0_MODE_TX   (6)
#define UART0_MODE_RX   (6)

void sys_uart_init(void)
{
    virtual_addr_t addr;
    u32_t val;

    d1_set_gpio_mode(GPIO_PORT_B, GPIO_PIN_8, UART0_MODE_TX);
    d1_set_gpio_mode(GPIO_PORT_B, GPIO_PIN_9, UART0_MODE_RX);

    clk_enable_module_uart(D1_CCU_BASE + CCU_UART_BGR_REG, 0);

    /* Config uart0 to 115200-8-1-0 */
    addr = 0x02500000;
    write32(addr + 0x04, 0x0);
    write32(addr + 0x08, 0xf7);
    write32(addr + 0x10, 0x0);
    val = read32(addr + 0x0c);
    val |= (1 << 7);
    write32(addr + 0x0c, val);
    write32(addr + 0x00, 0xd & 0xff);
    write32(addr + 0x04, (0xd >> 8) & 0xff);
    val = read32(addr + 0x0c);
    val &= ~(1 << 7);
    write32(addr + 0x0c, val);
    val = read32(addr + 0x0c);
    val &= ~0x1f;
    val |= (0x3 << 0) | (0 << 2) | (0x0 << 3);
    write32(addr + 0x0c, val);
}

void sys_uart_putc(char c)
{
    virtual_addr_t addr = 0x02500000;

    while((read32(addr + 0x7c) & (0x1 << 1)) == 0);
    write32(addr + 0x00, c);
}

int main(void)
{
    sys_clock_init();
    sys_uart_init();
    printf("hello world\n\r");
    return 0;
}
