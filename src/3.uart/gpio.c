#include <gpio.h>

void d1_set_gpio_mode(uint32_t gpio_port, uint32_t gpio_pin, uint32_t mode)
{
    // uint32_t gpio_base_addr = 0;
    // uint32_t val = 0;
    // gpio_base_addr = gpio_port;

    // val = read32(gpio_base_addr + 0x40);
    // val &= ~(0xf << ((8 & 0x7) << 2));
    // val |= ((0x6 & 0xf) << ((8 & 0x7) << 2));
    // write32(addr, val);
}

void d1_set_gpio_val(uint32_t gpio_port, uint32_t gpio_pin, uint32_t val)
{
    
}

uint8_t d1_get_gpio_val(uint32_t gpio_port, uint32_t gpio_pin)
{
    return 0;
}