#ifndef __D1_CLINT_H__
#define __D1_CLINT_H__

//#define D1_PLIC         0x10000000
#define D1_MSIP0        0x14000000
#define D1_MTIMECMPL0   0x14004000
#define D1_MTIMECMPH0   0x14004004
#define D1_MTIME        0x1400BFF8

#define D1_SSIP0        0x1400C000
#define D1_STIMECMPL0   0x1400D000
#define D1_STIMECMPH0   0x1400D004

void d1_clint_timer_cmp_set_val(int val);
void d1_clint_soft_irq_clear(void);

#endif