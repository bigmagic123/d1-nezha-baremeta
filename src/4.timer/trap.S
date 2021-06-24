	
/*
Register    ABI Name            Description                             Saver
x0           zero               Hard-wired zero                         --
x1           ra                 Return address                          Caller
x2           sp                 Stack pointer                           Caller
x3           gp                 Global pointer                          --
x4           tp                 Thread pointer                          --
x5-7         t0-2               Temporaries                             Caller
x8           s0/fp              Saved register/frame pointer            Caller
x9           s1                 Save register                           Caller
x10-11       a0-1               Function arguments/return values        Caller
x12-17       a2-7               Function arguments                      Caller
x18-27       s2-11              Saved registers                         Caller
x28-31       t3-6               Temporaries                             Caller
-------------------------------------------------------------------------------
f0-7         ft0-7              FP temporaries                          Caller
f8-9         fs0-1              FP save registers                       Caller
f10-11       fa0-1              FP arguments/return values              Caller
f12-17       fa2-7              FP arguments                            Caller
f18-27       fs2-11             FP saved registers                      Caller
f28-31       ft8-11             FP temporaries                          Caller
*/


#if __riscv_xlen == 64
#define STORE    sd
#define LOAD     ld
#define REGBYTES 8
#else
#define STORE    sw
#define LOAD     lw
#define REGBYTES 4
#endif

/* Save caller registers */
.macro RESTORE_CONTEXT
    LOAD x1, 0*REGBYTES(sp)
    LOAD x4, 1*REGBYTES(sp)
    LOAD x5, 2*REGBYTES(sp)
    LOAD x6, 3*REGBYTES(sp)
    LOAD x7, 4*REGBYTES(sp)
    LOAD x10, 5*REGBYTES(sp)
    LOAD x11, 6*REGBYTES(sp)
    LOAD x12, 7*REGBYTES(sp)
    LOAD x13, 8*REGBYTES(sp)
    LOAD x14, 9*REGBYTES(sp)
    LOAD x15, 10*REGBYTES(sp)
    LOAD x16, 14*REGBYTES(sp)
    LOAD x17, 15*REGBYTES(sp)
    LOAD x28, 16*REGBYTES(sp)
    LOAD x29, 17*REGBYTES(sp)
    LOAD x30, 18*REGBYTES(sp)
    LOAD x31, 19*REGBYTES(sp)
.endm


.macro SAVE_CONTEXT
    /* Allocate stack space for context saving */
    addi sp, sp, -20*REGBYTES

    STORE x1, 0*REGBYTES(sp)
    STORE x4, 1*REGBYTES(sp)
    STORE x5, 2*REGBYTES(sp)
    STORE x6, 3*REGBYTES(sp)
    STORE x7, 4*REGBYTES(sp)
    STORE x10, 5*REGBYTES(sp)
    STORE x11, 6*REGBYTES(sp)
    STORE x12, 7*REGBYTES(sp)
    STORE x13, 8*REGBYTES(sp)
    STORE x14, 9*REGBYTES(sp)
    STORE x15, 10*REGBYTES(sp)
    STORE x16, 14*REGBYTES(sp)
    STORE x17, 15*REGBYTES(sp)
    STORE x28, 16*REGBYTES(sp)
    STORE x29, 17*REGBYTES(sp)
    STORE x30, 18*REGBYTES(sp)
    STORE x31, 19*REGBYTES(sp)
.endm

    
    .section .entry, "ax", %progbits
	.align 3
	.globl _trap_handler
_trap_handler:
    /* Save the caller saving registers (context) */
    SAVE_CONTEXT
    csrr a0,mcause
    csrr a1,mepc
    call irq_handle_trap
    RESTORE_CONTEXT
    mret

//write mtvec val 
  .global table_val_set
table_val_set:
    la t0, _trap_handler
    csrw mtvec, t0
    jr ra

  .global all_interrupt_disable
all_interrupt_disable:
    csrrci a0, mstatus, 8
    ret

  .global all_interrupt_enable
all_interrupt_enable:
    csrrwi a0, mstatus, 8
    ret
