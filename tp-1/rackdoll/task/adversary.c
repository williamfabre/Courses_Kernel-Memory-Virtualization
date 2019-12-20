#include <string.h>
#include <syscall.h>


extern char __task_start;
extern char __task_end;
extern char __bss_end;


char zero_variable = 0;
char zero_zone[8000];

void entry(void)
{
	size_t i;
	//			0x2000000000 fin de stack
	char *addr = (char *)	0x1fffff3000;
	//			0x40000000 debut de stack

	syscall_print("  ==> Adversary Task\n");

	// 0 -> 4096
	// (0x0 -> 0x100) = (0 -> 255) (10 fois)
	for (i = 0; i < 0x1000; i++)
	{
		// 0 -> 127
		// 18446744073709551488
		// FFFFFFFFFFFFFF80

		addr[i] = i & 0xff;

		/*syscall_print("adversary addr = ");*/
		/*syscall_printnum(addr[i]);*/
		/*syscall_printnum(i & 0xff);*/
		/*syscall_print("\n");*/
	}

	syscall_munmap((vaddr_t) addr);

	// 0 -> 4096
	for (i = 0; i < 0x1000; i++)
		if (addr[i] != 0) {
			syscall_print("  --> Adversary result: failure\n");
			syscall_exit();
		}

	syscall_yield();
	syscall_print("  --> Adversary result: success\n");
	syscall_exit();
}


struct task_header header __attribute__((section(".header"))) = {
	.magic = TASK_HEADER_MAGIC,
	.load_addr = (vaddr_t) &__task_start,
	.load_end_addr = (vaddr_t) &__task_end,
	.bss_end_addr = (vaddr_t) &__bss_end,
	.header_addr = (vaddr_t) &header,
	.entry_addr = (vaddr_t) &entry
};
