


#include "plint.h"
#include "common.h"
#include "riscv64.h"

//priority 0 is invaild, 1 is lowset priority 
void c906_plic_priority_set(uint32_t irq_num, uint32_t priority)
{
    write32(C906_PLIC_PHY_ADDR + PLIC_PRIO_REG(irq_num), priority);
}

int c906_plic_priority_get(uint32_t irq_num)
{
    return read32(C906_PLIC_PHY_ADDR + PLIC_PRIO_REG(irq_num));
}

int c906_plic_pending_get(uint32_t irq_num)
{
    int pending_reg_cnt = irq_num / 32;
    int num = irq_num % 32;

    return (read32(C906_PLIC_PHY_ADDR + PLIC_IP_REG(pending_reg_cnt)) & (1 << num));
}

void c906_plic_pending_set(uint32_t irq_num)
{
    int pending_reg_cnt = irq_num / 32;
    int num = irq_num % 32;

    write32((C906_PLIC_PHY_ADDR + PLIC_IP_REG(pending_reg_cnt)), (1 << num));
}

void c906_plic_mmode_enable(uint32_t irq_num)
{
    int enable_reg_cnt = irq_num / 32;
    int num = irq_num % 32;

    c906_plic_priority_set(irq_num, 1);

    int temp = read32(C906_PLIC_PHY_ADDR + PLIC_MIE_REG(enable_reg_cnt));
    int write_val = temp | (1 << num);
    write32(C906_PLIC_PHY_ADDR + PLIC_MIE_REG(enable_reg_cnt), write_val);

    csr_set(mie, MIP_MEIP);
}

//disable
void c906_plic_mmode_disable(uint32_t irq_num)
{
    int enable_reg_cnt = irq_num / 32;
    int num = irq_num % 32;

    int temp = read32(C906_PLIC_PHY_ADDR + PLIC_MIE_REG(enable_reg_cnt));
    int write_val = temp & (~(1 << num));
    write32(C906_PLIC_PHY_ADDR + PLIC_MIE_REG(enable_reg_cnt), write_val);
}
// 0 only Machine  1 Machine and User
void c906_plic_ctrl(int ctrl)
{
    write32(C906_PLIC_PHY_ADDR + PLIC_CTRL_REG, ctrl);
}

//th >= 0
void c906_plic_mmode_threshold(int th)
{
    write32(C906_PLIC_PHY_ADDR + PLIC_MTH_REG, th);
}

int c906_plic_mmode_threshold_get(void)
{
    return read32(C906_PLIC_PHY_ADDR + PLIC_MTH_REG);
}

int c906_plic_mmode_mclaim_get(void)
{
    return read32(C906_PLIC_PHY_ADDR + PLIC_MCLAIM_REG);
}

void c906_plic_mmode_mclaim_complete(int id)
{
    write32(C906_PLIC_PHY_ADDR + PLIC_MCLAIM_REG, id);
}

void plic_complete(int irqno)
{
    c906_plic_mmode_mclaim_complete(irqno);
}


/*
 * Handling an interrupt is a two-step process: first you claim the interrupt
 * by reading the claim register, then you complete the interrupt by writing
 * that source ID back to the same claim register.  This automatically enables
 * and disables the interrupt, so there's nothing else to do.
 */
void plic_handle_irq(void)
{
    unsigned int irq;

    csr_clear(mie, MIP_MEIP);
    while ((irq = c906_plic_mmode_mclaim_get()))
    {
        printf("irq is %d\n",irq);
        plic_complete(irq);

        //uart
        if(irq == 18)
        {
            printf("recv:%c\n",sys_uart_getc(0));
        }

        //gpio pb
        if(irq == 85)
        {
            printf("pb %08x\n", d1_get_pb_irq_status());
        }

        //watchdog irq
        if(irq == 79)
        {
            watchdog_get_status();
        }
    }
    csr_set(mie, MIP_MEIP);
}
