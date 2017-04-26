/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>



###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
// Trap numbers
// These are processor defined:
#define T_DIVIDE     0		// divide error
#define T_DEBUG      1		// debug exception
#define T_NMI        2		// non-maskable interrupt
#define T_BRKPT      3		// breakpoint
#define T_OFLOW      4		// overflow
#define T_BOUND      5		// bounds check
#define T_ILLOP      6		// illegal opcode
#define T_DEVICE     7		// device not available
#define T_DBLFLT     8		// double fault
#define T_TSS       10		// invalid task switch segment
#define T_SEGNP     11		// segment not present
#define T_STACK     12		// stack exception
#define T_GPFLT     13		// general protection fault
#define T_PGFLT     14		// page fault
#define T_FPERR     16		// floating point error
#define T_ALIGN     17		// aligment check
#define T_MCHK      18		// machine check
#define T_SIMDERR   19		// SIMD floating point error*/


*/

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */

// No error code traps
TRAPHANDLER_NOEC(trap_divide, T_DIVIDE)
TRAPHANDLER_NOEC(trap_debug, T_DEBUG)
TRAPHANDLER_NOEC(trap_nmi, T_NMI)
TRAPHANDLER_NOEC(trap_breakpoint, T_BRKPT)
TRAPHANDLER_NOEC(trap_overflow, T_OFLOW)
TRAPHANDLER_NOEC(trap_bounds_check, T_BOUND)
TRAPHANDLER_NOEC(trap_illegal_opcode, T_ILLOP)
TRAPHANDLER_NOEC(trap_device, T_DEVICE)
TRAPHANDLER_NOEC(trap_floating_point, T_FPERR)
TRAPHANDLER(trap_machine_check, T_MCHK)
TRAPHANDLER(trap_simd_floating_point, T_SIMDERR)

// Error code producing traps
TRAPHANDLER(trap_double_flt, T_DBLFLT)
TRAPHANDLER(trap_invalid_tss, T_TSS)
TRAPHANDLER(trap_seg_notpresent, T_SEGNP)
TRAPHANDLER(trap_stack_exception, T_STACK)
TRAPHANDLER(trap_general_protection_flt, T_GPFLT)
TRAPHANDLER(trap_page_flt, T_PGFLT)
TRAPHANDLER(trap_alignment_check, T_ALIGN)


/*
 * Lab 3: Your code here for _alltraps
 */

_alltraps:
	push %ds
	push %es
	pushal // push edi, esi, ebp, esp, edx, ecx, ebx, eax

	movl $GD_KD, %eax
	movw %ax, %ds
	movw %ax, %es

	pushl %esp
	call trap
