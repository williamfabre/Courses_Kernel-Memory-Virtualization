#include <idt.h>                            /* see there for interrupt names */
#include <memory.h>                               /* physical page allocator */
#include <printk.h>                      /* provides printk() and snprintk() */
#include <string.h>                                     /* provides memset() */
#include <syscall.h>                         /* setup system calls for tasks */
#include <task.h>                             /* load the task from mb2 info */
#include <types.h>              /* provides stdint and general purpose types */
#include <vga.h>                                         /* provides clear() */
#include <x86.h>                                    /* access to cr3 and cr2 */


void printpgt(uint64_t cr3);

__attribute__((noreturn))
void die(void)
{
	/* Stop fetching instructions and go low power mode */
	asm volatile ("hlt");

	/* This while loop is dead code, but it makes gcc happy */
	while (1)
		;
}

__attribute__((noreturn))
void main_multiboot2(void *mb2)
{
	uint64_t cr3;

	clear();                                     /* clear the VGA screen */
	printk("Rackdoll OS\n-----------\n\n");                 /* greetings */

	setup_interrupts();                           /* setup a 64-bits IDT */
	setup_tss();                                  /* setup a 64-bits TSS */
	interrupt_vector[INT_PF] = pgfault;      /* setup page fault handler */

	disable_pic();                         /* disable anoying legacy PIC */
	sti();                                          /* enable interrupts */

	// EXERCICE 2
	cr3 = store_cr3();
	printpgt(cr3);                                   /* print page table */

	load_tasks(mb2);                         /* load the tasks in memory */
	run_tasks();                                 /* run the loaded tasks */

	printk("\nGoodbye!\n");                                 /* fairewell */
	die();                        /* the work is done, we can die now... */
}


void printpgt(uint64_t cr3)
{
	/*uint64_t *p_val_64 = 0;*/
	/*uint64_t nb_0 = 0;*/

	int* pml4 = cr3;
	int* pml3 = cr3;
	int* pml2 = cr3;
	int* pml1 = cr3;

	uint64_t pml4_index = 0;
	uint64_t pml3_index = 0;
	uint64_t pml2_index = 0;
	uint64_t pml1_index = 0;

	uint64_t pml4_max = (1 << 8);
	uint64_t pml3_max = (1 << 8);
	uint64_t pml2_max = (1 << 8);
	uint64_t pml1_max = (1 << 8);

	printk("\n CR3 = %p\n", pml4);
	// TODO CEHCKER LA VALIDITE DE LA PAGE

	// PML4
	for (pml4_index = 0; pml4_index < pml4_max; pml4_index++)
	{
		if (*pml4 != 0)
		{
			pml3 = *pml4;
			printk("\n PML4 OK [%p]\n ", pml3);

		}
		pml4++;
	}

	// PML3
	pml3 =  (int)pml3 & 0xFFF000; // bit masking
	for (pml3_index = 0; pml3_index < pml3_max; pml3_index++)
	{
		if (*pml3 != 0)
		{
			pml2 = *pml3;
			printk("\n PML3 OK [%p]\n ", pml2);
		}
		pml3++;

	}

	// TODO CHECKER SI LA PAGE EST HUGE
	// PML2
	pml2 =  (int)pml2 & 0xFFF000; // bit masking
	for (pml2_index = 0; pml2_index < pml2_max; pml2_index++)
	{
		// TODO CHECKER SI LA PAGE EST HUGE
		if (*pml2 != 0 )
		{
			pml1 = *pml2;
			printk("\n pml2 OK [%p]\n ", pml1);

		}
		// TODO CHECKER SI LA PAGE EST HUGE
		// else ...
		pml2++;
	}

	// PML1
	pml1 =  (int)pml1 & 0xFFF000; // bit masking
	for (pml1_index = 0; pml1_index < pml1_max; pml1_index++)
	{
		if (*pml1 != 0)
		{
			printk("\n pml1 OK [%p]=%ld\n ", pml1, *pml1);
		}
		pml1++;

	}



}



// HAVING FUN WITH SOME FUNCTIONS
/*for (pml4_index = 0; pml4_index < pml4_max; pml4_index++)*/
/*{*/
/*load_rsp(cr3);	// place la valeur dans rsp*/
/*mov(val_64);			// lit la valeur en memoire*/
/*val_64 = store_rsp();		// place la valeur dans rsp*/

/*printk("\n test 1 PML4 pointer=%p value=%p\n", cr3, val_64);*/
/*mov(val_64);			// lit la valeur en memoire*/
/*val_64 = store_rsp();*/
/*printk("\n test 2 PML3 pointerbase=%p value=%p\n", val_64, val_64);*/
/*printk("\n test 2 PML4 pointer=%p value=%d\n", cr3, val_64);*/
/*}*/

/*for (pml3_index = 0; pml3_index < pml3_max; pml3_index++)*/
/*{*/
/*load_rsp(val_64);		// place la valeur dans rsp*/
/*mov(val_64);			// lit la valeur en memoire*/
/*val_64 = store_rsp();*/

/*printk("\n test 2 PML3 pointerbase=%p value=%p\n", val_64, val_64);*/
/*}*/


