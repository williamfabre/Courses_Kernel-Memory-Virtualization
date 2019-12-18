#include <memory.h>
#include <printk.h>
#include <string.h>
#include <x86.h>
#include <bit_manipulation.h>

#define PHYSICAL_POOL_PAGES  64
#define PHYSICAL_POOL_BYTES  (PHYSICAL_POOL_PAGES << 12)
#define BITSET_SIZE          (PHYSICAL_POOL_PAGES >> 6)

extern __attribute__((noreturn)) void die(void);

static uint64_t bitset[BITSET_SIZE];

static uint8_t pool[PHYSICAL_POOL_BYTES] __attribute__((aligned(0x1000)));

// alloue une nouvelle page déjà mappée par une identité.
// Le contenu de cette page est indéfini.
paddr_t alloc_page(void)
{
	size_t i, j;
	uint64_t v;

	for (i = 0; i < BITSET_SIZE; i++) {
		if (bitset[i] == 0xffffffffffffffff)
			continue;

		for (j = 0; j < 64; j++) {
			v = 1ul << j;
			if (bitset[i] & v)
				continue;

			bitset[i] |= v;
			return (((64 * i) + j) << 12) + ((paddr_t) &pool);
		}
	}

	printk("[error] Not enough identity free page\n");
	return 0;
}

void free_page(paddr_t addr)
{
	paddr_t tmp = addr;
	size_t i, j;
	uint64_t v;

	tmp = tmp - ((paddr_t) &pool);
	tmp = tmp >> 12;

	i = tmp / 64;
	j = tmp % 64;
	v = 1ul << j;

	if ((bitset[i] & v) == 0) {
		printk("[error] Invalid page free %p\n", addr);
		die();
	}

	bitset[i] &= ~v;
}


/*
 * Memory model for Rackdoll OS
 *
 * +----------------------+++0xffffffffffffffff
 * | Higher half          |
 * | (unused)             |
 * +----------------------+++0xffff800000000000	16Pico		= 18 446 603 336 221 196 288
 * | (impossible address) |
 * +----------------------+++0x00007fffffffffff	128To		=        140 737 488 355 327
 * | User            .:.  | // Between 128 GiB and 128 TiB is the heap addresses for user procs
 * | (text+data+heap) |   | // ERREUR		0x20000000030	=          2 199 023 255 600
 * +----------------------+++0x2000000000	128Go		=            137 438 953 472
 * | User      |          | // Between 1 GiB and 128 GiB is the stack
 * | (stack)   v          | // addresses for user processes growing down from 128 GiB.
 * +----------------------+++0x40000000		1Go=2^30	=              1 073 741 824
 * | Kernel               | // Between 2 MiB and 1 GiB, there are kernel
 * | (valloc)             | // addresses which are not mapped with an identity table.
 * +----------------------+++0x201000				=                  2 101 248
 * | Kernel               | // Between 2 MiB and 1 GiB, there are kernel
 * | (APIC)               | // addresses which are not mapped with an identity table.
 * +----------------------+++0x200000		2(Mo)=2*2^20	=                  2 097 152
 * | Kernel               | // The first 2 MiB are identity mapped and not cached.
 * | (text + data)        |
 * +----------------------+++0x100000		1(Mo)=2^20	=                  1 048 576
 * | Kernel               | // The first 2 MiB are identity mapped and not cached.
 * | (BIOS + VGA)         |
 * +----------------------+ 0x0
 *
 * This is the memory model for Rackdoll OS: the kernel is located in low
 * addresses. The first 2 MiB are identity mapped and not cached.
 * Between 2 MiB and 1 GiB, there are kernel addresses which are not mapped
 * with an identity table.
 * Between 1 GiB and 128 GiB is the stack addresses for user processes growing
 * down from 128 GiB.
 * The user processes expect these addresses are always available and that
 * there is no need to map them exmplicitely.
 * Between 128 GiB and 128 TiB is the heap addresses for user processes.
 * The user processes have to explicitely map them in order to use them.
 */

// THANKS TO SOME CODE THAT I CORRECTED FROM GITHUB
//mappe l’adresse virtuelle vaddr sur l’adresse physique paddr sur
// un espace d’une page pour la tâche ctx
// donne les droits 0x7
void map_page(struct task *ctx, vaddr_t vaddr, paddr_t paddr)
{
	vaddr_t *cadre = (vaddr_t *)ctx->pgt;
	int offset, i;

	/*printk("Mapping v_addr %p to p_addr %p...\n", vaddr ,paddr);*/

	for (i = 4; i > 0; --i)
	{
		offset = INDEX(vaddr, i);
		/*printk("%s offset lvl%d %p \n", __func__, i, offset);*/
		/*printk("%s cadre without offset lvl%d %p \n", __func__, i, cadre);*/
		cadre = cadre + offset;
		/*printk("%s cadre with offset lvl%d %p \n", __func__, i, cadre);*/
		if (!cadre || !check_1bit(*cadre, 1))// if empty or invalid
		{
			/*printk("alloc needed\n");*/
			*cadre = alloc_page();
			set_usr(cadre); // set right to value pointed by cadre
		}
		cadre = (uint64_t*) (*cadre & ADDR_MASK);
		/*printk("%s next cadre lvl%d %p \n", __func__, i-1, cadre);*/
	}

	/*printk("\n%s Gestion lvl%d %p \n", __func__, i, cadre);*/
	offset = INDEX(vaddr, i);
	cadre = cadre + offset;
	set_usr(&paddr);
	*cadre = paddr;
	/*printk("%s pointee %p \n", __func__, cadre);*/
	/*printk("%s alloc done (pointed) %p \n", __func__, *cadre);*/
	/*printk("\n\n");*/
}


// load_task: conseils de SMAIL
// IN THAT ORDER
/*	alloc new page table */
/*	link PMLs */
/*	ref. kernel code & data */
/*	map code & data */
/*	map bss */
void load_task(struct task *ctx)
{
	uint64_t size;
	vaddr_t bss_start_vaddr;
	paddr_t *address;

	paddr_t pml4;
	paddr_t pml3;
	paddr_t pml2;
	paddr_t pml1;

	// table alloc
	pml4 = alloc_page();
	pml3 = alloc_page();
	pml2 = alloc_page();
	pml1 = alloc_page();

	// table link and kernel ID mapping
	((paddr_t*)pml4)[0] = pml3 | 0x7;  // pml4[0] = pml3 | U | W | P
	((paddr_t*)pml3)[0] = pml2 | 0x7;  // pml3[0] = pml2 | U | W | P
	((paddr_t*)pml2)[0] = 0x0  | 0x19b;// pml2[0] = G | PS | PCD | PWT | W | P
	((paddr_t*)pml2)[1] = pml1 | 0x1b; // pml2[1] = apic | G | PCD | PWT | W | P

	// mise a jour du pgt
	ctx->pgt = (paddr_t)pml4;

	// taille du "segment" code+data+text+heap
	size = ctx->load_end_paddr - ctx->load_paddr;
	/*printk("%s %d number of page to alloc \n", __func__, (size / 0x1000));*/

	// adresse virtuelle de depart du bss
	bss_start_vaddr = ctx->load_vaddr + size;

	// map le debut du load map_page(*ctx, vaddr, paddr)
	map_page(ctx, ctx->load_vaddr, ctx->load_paddr);

	// map une nouvelle page physique sur le debut du bss puis mise a 0 du bss
	*address = alloc_page();
	memset(address, 0, size);
	map_page(ctx, bss_start_vaddr, *address);
}

// qui charge une nouvelle tâche en mémoire en modifiant le CR3
void set_task(struct task *ctx)
{
	uint64_t cr3;
	printk("%s PRE LOAD\n", __func__);
	cr3 = ctx->pgt;
	cr3 = store_cr3();
	print_pgt(cr3, 4);                                   //print page table
	load_cr3(ctx->pgt);
	printk("%s POST LOAD\n", __func__);
	cr3 = store_cr3();
	 print_pgt(cr3, 4);                                   //print page table
}

// alloue une page physique, l’initialise à zero et
// la mappe à l’adresse virtuelle donnée pour la tâche donnée.
void mmap(struct task *ctx, vaddr_t vaddr)
{
	printk("%s BEGIN\n", __func__);
	paddr_t *cadre;
	/*uint64_t cr3;*/

	// alloue
	*cadre = alloc_page();

	// init a 0
	memset(cadre, 0, 4000);

	// la mappe à l’adresse virtuelle donnée pour la tâche donnée.
	map_page(ctx, vaddr, *cadre);

	/*cr3 = ctx->pgt;*/
	/*print_pgt(cr3, 4);                                   //print page table*/

	/*printk("%s testme\n", __func__);*/
}

void munmap(struct task *ctx, vaddr_t vaddr)
{
}

void pgfault(struct interrupt_context *ctx)
{
	/*Contains a value called Page Fault Linear Address (PFLA).
	 * When a page fault occurs, the address the program attempted
	 * to access is stored in the CR2 register. */
	printk("Page fault at %p\n", ctx->rip);
	printk("  cr2 = %p\n", store_cr2());
	asm volatile ("hlt");
}

void duplicate_task(struct task *ctx)
{
}
