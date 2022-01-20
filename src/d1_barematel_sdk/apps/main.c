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

#define KEY_FIELD       (0x16AA)
#define WDOG_KEY_FIELD  (0xA57)

//init watchdog
#define WDOG_IRQ_EN_REG         (0x020500A0)
#define WDOG_IRQ_STA_REG        (0x020500A4)
#define WDOG_SOFT_RST_REG       (0x020500A8)
#define WDOG_CTRL_REG           (0x020500B0)
#define WDOG_CFG_REG            (0x020500B4)
#define WDOG_MODE_REG           (0x020500B8)
#define WDOG_OUTPUT_CFG_REG     (0x020500BC)

#define IRQ_WATCHDOG_NUM        (79)


void watchdog_irq_enable(void)
{
    write32(WDOG_IRQ_EN_REG, 0x01);
}

//read and clear
int watchdog_get_status(void)
{
    int val = read32(WDOG_IRQ_STA_REG);
    write32(WDOG_IRQ_STA_REG, val);
    return val;
}

//reset watchdog , watchdog first needs to be disabled
void watchdog_reset_system(void)
{
    write32(WDOG_SOFT_RST_REG, (KEY_FIELD << 16) | (0x01));//soft reset enable
}

//watchdog config
void watchdog_ctrl(void)
{
    write32(WDOG_CTRL_REG, (WDOG_KEY_FIELD << 16) | (0x01));//reset watchdog
}

void watchdog_cfg(int irq_enable, int clk)
{
    if(irq_enable)
    {
        write32(WDOG_CFG_REG, (KEY_FIELD << 16) | (clk << 8) | (0x10));
    }
    else
    {
        write32(WDOG_CFG_REG, (KEY_FIELD << 16) | (clk << 8) | (0x01));
    }
    
}

void watchdog_mode_set(int interval_val)
{
    write32(WDOG_MODE_REG, (KEY_FIELD << 16) | (interval_val << 4) | (0x01));//0:0.5s 1 1s
}

void watchdog_set_reset_time(int val)
{
    // 1/32ms * (N+1), default 1F (1ms)
    write32(WDOG_OUTPUT_CFG_REG, val);
}

#define WATCHDOG_IRQ

int main(void)
{
    char ch = -1;
    int cnt = 0;
    sys_clock_init();
    sys_uart0_init();
    table_val_set();
    printf("hello world\n\r");
    all_interrupt_enable();
    int tick = 0;
    while(1)
    {
        printf("cur tick is %d\r\n",tick++);
        sdelay(1000 * 1000);//1s
    }
    return 0;
}
