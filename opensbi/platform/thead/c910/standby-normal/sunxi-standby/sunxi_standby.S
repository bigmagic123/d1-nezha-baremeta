// SPDX-License-Identifier: BSD-2-Clause

#include "sunxi_cpu.h"


	.align 3
	.global sbi_suspend_finish
sbi_suspend_finish:
	/* save gprs */
	sd	zero, SBI_REGS_OFFSET(zero)(a0)
	sd	ra, SBI_REGS_OFFSET(ra)(a0)
	sd	gp, SBI_REGS_OFFSET(gp)(a0)
	sd	tp, SBI_REGS_OFFSET(tp)(a0)
	sd	t0, SBI_REGS_OFFSET(t0)(a0)
	sd	t1, SBI_REGS_OFFSET(t1)(a0)
	sd	t2, SBI_REGS_OFFSET(t2)(a0)
	sd	s0, SBI_REGS_OFFSET(s0)(a0)
	sd	s1, SBI_REGS_OFFSET(s1)(a0)
	sd	a0, SBI_REGS_OFFSET(a0)(a0)
	sd	a1, SBI_REGS_OFFSET(a1)(a0)
	sd	a2, SBI_REGS_OFFSET(a2)(a0)
	sd	a3, SBI_REGS_OFFSET(a3)(a0)
	sd	a4, SBI_REGS_OFFSET(a4)(a0)
	sd	a5, SBI_REGS_OFFSET(a5)(a0)
	sd	a6, SBI_REGS_OFFSET(a6)(a0)
	sd	a7, SBI_REGS_OFFSET(a7)(a0)
	sd	s2, SBI_REGS_OFFSET(s2)(a0)
	sd	s3, SBI_REGS_OFFSET(s3)(a0)
	sd	s4, SBI_REGS_OFFSET(s4)(a0)
	sd	s5, SBI_REGS_OFFSET(s5)(a0)
	sd	s6, SBI_REGS_OFFSET(s6)(a0)
	sd	s7, SBI_REGS_OFFSET(s7)(a0)
	sd	s8, SBI_REGS_OFFSET(s8)(a0)
	sd	s9, SBI_REGS_OFFSET(s9)(a0)
	sd	s10, SBI_REGS_OFFSET(s10)(a0)
	sd	s11, SBI_REGS_OFFSET(s11)(a0)
	sd	t3, SBI_REGS_OFFSET(t3)(a0)
	sd	t4, SBI_REGS_OFFSET(t4)(a0)
	sd	t5, SBI_REGS_OFFSET(t5)(a0)
	sd	t6, SBI_REGS_OFFSET(t6)(a0)
	sd	sp, SBI_REGS_OFFSET(sp)(a0)
	fence
	/* flush dcache all */
	dcache.call
	/* enter wfi */
	wfi


	.align 3
	.global sbi_system_resume

sbi_system_resume:
	la	a0, SBI_GPRS
	ld	ra, SBI_REGS_OFFSET(ra)(a0)
	ld	gp, SBI_REGS_OFFSET(gp)(a0)
	ld	tp, SBI_REGS_OFFSET(tp)(a0)
	ld	t1, SBI_REGS_OFFSET(t1)(a0)
	ld	t2, SBI_REGS_OFFSET(t2)(a0)
	ld	s0, SBI_REGS_OFFSET(s0)(a0)
	ld	s1, SBI_REGS_OFFSET(s1)(a0)
	ld	a1, SBI_REGS_OFFSET(a1)(a0)
	ld	a2, SBI_REGS_OFFSET(a2)(a0)
	ld	a3, SBI_REGS_OFFSET(a3)(a0)
	ld	a4, SBI_REGS_OFFSET(a4)(a0)
	ld	a5, SBI_REGS_OFFSET(a5)(a0)
	ld	a6, SBI_REGS_OFFSET(a6)(a0)
	ld	a7, SBI_REGS_OFFSET(a7)(a0)
	ld	s2, SBI_REGS_OFFSET(s2)(a0)
	ld	s3, SBI_REGS_OFFSET(s3)(a0)
	ld	s4, SBI_REGS_OFFSET(s4)(a0)
	ld	s5, SBI_REGS_OFFSET(s5)(a0)
	ld	s6, SBI_REGS_OFFSET(s6)(a0)
	ld	s7, SBI_REGS_OFFSET(s7)(a0)
	ld	s8, SBI_REGS_OFFSET(s8)(a0)
	ld	s9, SBI_REGS_OFFSET(s9)(a0)
	ld	s10, SBI_REGS_OFFSET(s10)(a0)
	ld	s11, SBI_REGS_OFFSET(s11)(a0)
	ld	t3, SBI_REGS_OFFSET(t3)(a0)
	ld	t4, SBI_REGS_OFFSET(t4)(a0)
	ld	t5, SBI_REGS_OFFSET(t5)(a0)
	ld	t6, SBI_REGS_OFFSET(t6)(a0)
	li	t0, 0x400000
	csrw    mxstatus,t0
	ld	t0, SBI_REGS_OFFSET(t0)(a0)
	ld	sp, SBI_REGS_OFFSET(sp)(a0)
	ld	a0, SBI_REGS_OFFSET(a0)(a0)

	ret 
