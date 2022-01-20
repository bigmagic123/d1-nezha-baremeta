#include <gpio.h>
#include <common.h>

void d1_set_gpio_mode(uint32_t gpio_port, uint32_t gpio_pin, uint16_t mode)
{
    uint32_t pin_level = 0;
    uint32_t gpio_base_addr = 0;
    uint32_t val = 0;
    pin_level = gpio_pin / 8;
    gpio_base_addr = gpio_port + pin_level * 0x04;
    val = read32(gpio_base_addr);

    val &= ~(0xf << ((gpio_pin & 0x7) << 2));
    val |= ((mode & 0xf) << ((gpio_pin & 0x7) << 2));

    write32(gpio_base_addr, val);
}

void d1_set_gpio_val(uint32_t gpio_port, uint32_t gpio_pin, uint32_t val)
{
    uint32_t gpio_base_addr = 0;
    uint32_t cur_val = 0;
    gpio_base_addr = gpio_port + 0x10;
    cur_val = read32(gpio_base_addr);

    if(val)
    {
        cur_val |= (1 << gpio_pin);
    }
    else
    {
        cur_val &= ~(1 << gpio_pin);
    }
    
    write32(gpio_base_addr, cur_val);
}

uint8_t d1_get_gpio_val(uint32_t gpio_port, uint32_t gpio_pin)
{
    uint32_t gpio_base_addr = 0;
    uint32_t cur_val = 0;
    gpio_base_addr = gpio_port + 0x10;
    cur_val = read32(gpio_base_addr);
    uint8_t ret = 0; 

    if(cur_val & (1 << gpio_pin))
    {
        ret = 1;
    }
    else
    {
        ret = 0;
    }

    return ret;
}

void d1_set_gpio_irq_enable(uint32_t gpio_port, uint32_t gpio_pin, uint32_t cfg, uint8_t enable)
{
    uint32_t pin_level = 0;
    uint32_t gpio_base_addr = 0;
    uint32_t val = 0;
    pin_level = gpio_pin / 8;
    gpio_base_addr = D1_GPIO_BASE + D1_GPIO_PB_EINT_CFG0 + pin_level * 0x04;
    val = read32(gpio_base_addr);

    val &= ~(0xf << ((gpio_pin & 0x7) << 2));
    val |= ((cfg & 0x0f) << ((gpio_pin & 0x7) << 2));

    write32(gpio_base_addr, val);

    //set gpio irq enable
    gpio_base_addr = D1_GPIO_BASE + D1_GPIO_PB_EINT_CTL;
    val = read32(gpio_base_addr);
    if(enable)
    {
        val |= (1 << gpio_pin);
    }
    else
    {
        val &= ~(1 << gpio_pin);
    }
    write32(gpio_base_addr, val);
}

uint32_t d1_get_pb_irq_status(void)
{
    uint32_t val = 0;
    uint32_t gpio_base_addr = 0;
    gpio_base_addr = D1_GPIO_BASE + D1_GPIO_PB_EINT_STATUS;
    val = read32(gpio_base_addr);

    write32(gpio_base_addr, val);//write 1 clear
    return val;
}
