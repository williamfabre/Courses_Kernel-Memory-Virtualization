#include <string.h>
#include <syscall.h>


extern char __task_start;
extern char __task_end;
extern char __bss_end;


char zero_variable = 0;
char zero_zone[8000];

void entry(void)
{
	syscall_print("  ==> mylittlefork Task\n");

	if (syscall_fork())
	{
		syscall_print("mylittlefork Task FATHER\n");
	}else{
		syscall_print("mylittlefork Task SON\n");
	}

	syscall_yield();
	syscall_print("  --> mylittlefork result: success\n");
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
