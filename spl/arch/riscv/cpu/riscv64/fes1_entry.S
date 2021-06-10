// * SPDX-License-Identifier:	GPL-2.0+


#include <config.h>

#define REGBYTES		4

.globl _start

_start:
	// mv	s1, ra
	addi    sp,sp,-32
	sd      s0,8(sp)
	sd      s1,16(sp)
	sd      ra,24(sp)
	mv      s0,a0

	jal clear_bss
	jal	main
	ld      ra,24(sp)
	ld      s0,8(sp)
	ld      s1,16(sp)
	addi    sp,sp,32
	ret

clear_bss:
	la	t0, __bss_start
	la	t1, __bss_end

clbss_1:
	sw zero, 0(t0)
	addi t0, t0, REGBYTES
	blt t0, t1, clbss_1
	ret
