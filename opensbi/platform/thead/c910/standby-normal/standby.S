// SPDX-License-Identifier: BSD-2-Clause

.globl	standby_bin_start
.align	8
standby_bin_start:
.incbin	"build/platform/thead/c910/standby-normal/standby.bin"
.globl	standby_bin_end
standby_bin_end:
.align	8
