/* SPDX-License-Identifier: Apache-2.0 */

#define STACK_AL    16

#if __riscv_xlen == 64
#  define REGSIZE     8
#  define REGOFF      3
#  define OP_LD       ld
#  define OP_ST       sd
#elif __riscv_xlen == 32
#  define REGSIZE     4
#  define REGOFF      2
#  define OP_LD       lw
#  define OP_ST       sw
#else
#  error __riscv_xlen must equal 32 or 64
#endif

.global k_crt_start
.type k_crt_start, @function

k_crt_start:
	/* Load gp */
.option push
.option norelax
	la    gp, __global_pointer$
.option pop
	/* argc */
	OP_LD a0, (sp)
	/* argv */
	addi  a1, sp, REGSIZE
	/* envp */
	slli  a2, a0, REGOFF
	add   a2, a1, a2
	addi  a2, a2, REGSIZE
	andi  sp, sp, ~(STACK_AL - 1)

	call  __libc_init
1:
	wfi
	j    1b
