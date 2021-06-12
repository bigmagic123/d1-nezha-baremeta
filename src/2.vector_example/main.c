#include <stdio.h>
#include <stdlib.h>
#include <riscv64.h>
#include <clk.h>
#include <printf.h>

//typedef unsigned int virtual_addr_t;
//typedef unsigned int u32_t;
void sys_uart_putc(char c);

void _putchar(char character)
{
	sys_uart_putc(character);
  // send char to console etc.
}

void sys_jtag_init(void)
{
	virtual_addr_t addr;
	u32_t val;

	/* Config GPIOF0, GPIOF1, GPIOF3 and GPIOF5 to JTAG mode */
	addr = 0x020000f0 + 0x00;
	val = read32(addr);
	val &= ~(0xf << ((0 & 0x7) << 2));
	val |= ((0x4 & 0xf) << ((0 & 0x7) << 2));
	write32(addr, val);

	val = read32(addr);
	val &= ~(0xf << ((1 & 0x7) << 2));
	val |= ((0x4 & 0xf) << ((1 & 0x7) << 2));
	write32(addr, val);

	val = read32(addr);
	val &= ~(0xf << ((3 & 0x7) << 2));
	val |= ((0x4 & 0xf) << ((3 & 0x7) << 2));
	write32(addr, val);

	val = read32(addr);
	val &= ~(0xf << ((5 & 0x7) << 2));
	val |= ((0x4 & 0xf) << ((5 & 0x7) << 2));
	write32(addr, val);
}

void sys_uart_init(void)
{
	virtual_addr_t addr;
	u32_t val;

	/* Config GPIOB8 and GPIOB9 to txd0 and rxd0 */
	addr = 0x02000030 + 0x04;
	val = read32(addr);
	val &= ~(0xf << ((8 & 0x7) << 2));
	val |= ((0x6 & 0xf) << ((8 & 0x7) << 2));
	write32(addr, val);

	val = read32(addr);
	val &= ~(0xf << ((9 & 0x7) << 2));
	val |= ((0x6 & 0xf) << ((9 & 0x7) << 2));
	write32(addr, val);

	/* Open the clock gate for uart0 */
	addr = 0x0200190c;
	val = read32(addr);
	val |= 1 << 0;
	write32(addr, val);

	/* Deassert uart0 reset */
	addr = 0x0200190c;
	val = read32(addr);
	val |= 1 << 16;
	write32(addr, val);

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

void test_v(void)
{
  float a[]={1.0,2.0,3.0,4.0};
  float b[]={1.0,2.0,3.0,4.0};
  float c[]={0.0,0.0,0.0,0.0};
  int len=4;
  int i=0;

  //inline assembly for RVV 0.7.1
  //for(i=0; i<len; i++){c[i]=a[i]+b[i];}
  asm volatile(
               "mv         t4,   %[LEN]       \n\t"
               "mv         t1,   %[PA]        \n\t"
               "mv         t2,   %[PB]        \n\t"
               "mv         t3,   %[PC]        \n\t"
               "LOOP1:                        \n\t"
               "vsetvli    t0,   t4,   e32,m1 \n\t"
               "sub        t4,   t4,   t0     \n\t"
               "slli       t0,   t0,   2      \n\t" //Multiply number done by 4 bytes
               "vle.v      v0,   (t1)         \n\t"
               "add        t1,   t1,   t0     \n\t"
               "vle.v      v1,   (t2)         \n\t"
               "add        t2,   t2,   t0     \n\t"
               "vfadd.vv   v2,   v0,   v1     \n\t"
               "vse.v      v2,   (t3)         \n\t"
               "add        t3,   t3,   t0     \n\t"
               "bnez       t4,   LOOP1        \n\t"
               :
               :[LEN]"r"(len), [PA]"r"(a),[PB]"r"(b),[PC]"r"(c)
               :"cc","memory", "t0", "t1", "t2", "t3", "t4",
                "v0", "v1", "v2"
               );

  for(i=0; i<len; i++){
		printf("\n");
    		printf("%f\n",c[i]);
    		printf("\n");
  	}
}
int main(void)
{
	sys_clock_init();
	sys_jtag_init();
	sys_uart_init();
	test_v();
//	printf("hello world\n");
	while(1);
	return 0;
}
