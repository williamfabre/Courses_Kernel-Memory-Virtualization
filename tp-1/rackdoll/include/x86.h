#ifndef _INCLUDE_X86_H_
#define _INCLUDE_X86_H_


/*
 * The zero, code and data selectors are set in the gdt64 declared in the
 * entry.S file.
 * Kernel code and data segment descriptors *must* be consecutive in the table
 * so we can use syscall and sysret instructions.
 * Kernel data and code segment descriptors *must* be consecutive in the table
 * so we can use syscall and sysret instructions (note that this time,  the
 * data segment must be placed before the code segment).
 */

#define ZERO_SELECTOR          0x00
#define KERNEL_CODE_SELECTOR   0x08
#define KERNEL_DATA_SELECTOR   0x10
#define USER_DATA_SELECTOR     0x18
#define USER_CODE_SELECTOR     0x20
#define TSS_SELECTOR           0x28


/*
 * Model Sepcific Registers defintions.
 * These ones are common to all families of modern Intel and AMD processors.
 */

#define MSR_EFER               0xc0000080
#define MSR_EFER_SCE           (1ul <<  0) //SCE (System Call Extensions)
#define MSR_EFER_LME           (1ul <<  8) //LME (Long Mode Enable)
#define MSR_EFER_LMA           (1ul << 10) //LMA (Long Mode Active)
#define MSR_EFER_NXE           (1ul << 11) //NXE (No-Execute Enable)
#define MSR_EFER_SVME          (1ul << 12) //SVME (Secure Virtual Machine Enable)
#define MSR_EFER_LMSLE         (1ul << 13) //LMSLE (Long Mode Segment Limit Enable)
#define MSR_EFER_FFXSR         (1ul << 14) //FFXSR (Fast FXSAVE/FXRSTOR)
#define MSR_EFER_TCE           (1ul << 15) //TCE (Translation Cache Extension)

/*
 * Rflags definitions.
 * Le registre RFLAGS - aussi dit registre de drapeaux - est le registre d'état
 * des processeurs de la famille x86-64 (64 bits)
 */

#define RFLAGS_CF              (1ul <<  0) //Carry flag
#define RFLAGS_PF              (1ul <<  2) //Parity flag
#define RFLAGS_AF              (1ul <<  4) //Auxiliary carryflag
#define RFLAGS_ZF              (1ul <<  6) //Zero flag
#define RFLAGS_SF              (1ul <<  7) //Sign flag
#define RFLAGS_TF              (1ul <<  8) //Trace flag
#define RFLAGS_IF              (1ul <<  9) //Interrupt flag
#define RFLAGS_DF              (1ul << 10) //Direction flag
#define RFLAGS_OF              (1ul << 11) //Overflow flag
#define RFLAGS_IOPL            (3ul << 12) // IOPL 12 et 13 Input / Output privilege level field
#define RFLAGS_NT              (1ul << 14) // Nested task Flag (Drapeau de tâche chaînée)
#define RFLAGS_RF              (1ul << 16) // Resume Flag (Drapeau de redémarrage)
#define RFLAGS_VM              (1ul << 17) // Virtual-8086 mode Flag(Drapeau de mode virtuel 8086)
#define RFLAGS_AC              (1ul << 18) // Alignment Check Flag
#define RFLAGS_VIF             (1ul << 19) // Virtual Interrupt Flag
#define RFLAGS_VIP             (1ul << 20) // Virtual Interrupt Pending
#define RFLAGS_ID              (1ul << 21) // Identification Flag



static inline void load_rsp(uint64_t rsp)
{
	asm volatile ("movq %0, %%rsp" : : "r" (rsp));
}

static inline uint64_t store_rsp(void)
{
	uint64_t rsp;
	asm volatile ("movq %%rsp, %0" : "=r" (rsp));
	return rsp;
}


static inline void load_cr2(uint64_t cr2)
{
	asm volatile ("movq %0, %%cr2" : : "a" (cr2));
}

static inline uint64_t store_cr2(void)
{
	uint64_t cr2;
	asm volatile ("movq %%cr2, %0" : "=a" (cr2));
	return cr2;
}


static inline void load_cr3(uint64_t cr3)
{
	asm volatile ("movq %0, %%cr3" : : "a" (cr3));
}

static inline uint64_t store_cr3(void)
{
	uint64_t cr3;
	asm volatile ("movq %%cr3, %0" : "=a" (cr3));
	return cr3;
}


// Load Task Register (ltr)
//
// The task register is loaded by LTR from the source
// register or memorylocation specified by the operand. The loaded task state
// segment is taggedbusy. A task switch does not occur.
static inline void load_tr(uint16_t tr)
{
	asm volatile ("ltr %0" : : "r" (tr));
}

//Store Task Register (str)
//
// The contents of the task register is stored by sldt as indicated by
// the effective address operand. STR is stored into the two-byte register or
// the memory location
// expl :
static inline uint16_t store_tr(void)
{
	uint16_t tr;
	asm volatile ("str %0" : "=r" (tr));
	return tr;
}


// Invalidate Page
//
// Invalidate a single entry in the translation lookaside buffer.
// expl : invlpg 5(%ebx)
static inline void invlpg(vaddr_t vaddr)
{
	asm volatile ("invlpg %0" : : "m" (vaddr));
}


typedef uint16_t  port_t;

//Input from Port (in,ins)
//
// in transfers a byte, word, or long from the immediate port into the
// byte,word, or long memory address pointed to by the AL, AX, or EAX
// register,respectively.
static inline uint8_t in8(port_t port)
{
	uint8_t ret;
	asm volatile ("inb %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

static inline uint16_t in16(port_t port)
{
	uint16_t ret;
	asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

static inline uint32_t in32(port_t port)
{
	uint32_t ret;
	asm volatile ("inl %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

//Output from Port (out,outs)
//
//Transfers a byte, word, or long from the memory address pointed to by
//thecontent of the AL, AX, or EAX register to the immediate 8-, 16-, or
//32-bitport address.
static inline void out8(port_t port, uint8_t val)
{
	asm volatile ("outb %0, %1" : : "a" (val), "dN" (port));
}

static inline void out16(port_t port, uint16_t val)
{
	asm volatile ("outw %0, %1" : : "a" (val), "dN" (port));
}

static inline void out32(port_t port, uint32_t val)
{
	asm volatile ("outl %0, %1" : : "a" (val), "dN" (port));
}

//WRMSR — Write to Model Specific Register
//
//https://www.felixcloutier.com/x86/wrmsr
static inline void wrmsr(uint32_t msr, uint64_t val)
{
	uint32_t eax, edx;
	edx = (val >> 32) & 0xffffffff;
	eax = (val >>  0) & 0xffffffff;
	asm volatile ("wrmsr" : : "a" (eax), "c" (msr), "d" (edx));
}

static inline uint64_t rdmsr(uint32_t msr)
{
	uint32_t eax, edx;
	asm volatile ("rdmsr" : "=a" (eax), "=d" (edx) : "c" (msr));
	return (((uint64_t) edx) << 32) | eax;
}


#endif
