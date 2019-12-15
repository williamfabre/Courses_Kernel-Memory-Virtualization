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
 * +----------------------+ 0xffffffffffffffff
 * | Higher half          |
 * | (unused)             |
 * +----------------------+ 0xffff800000000000
 * | (impossible address) |
 * +----------------------+ 0x00007fffffffffff
 * | User                 |
 * | (text + data + heap) |
 * +----------------------+ 0x2000000000
 * | User                 |
 * | (stack)              |
 * +----------------------+ 0x40000000
 * | Kernel               |
 * | (valloc)             |
 * +----------------------+ 0x201000
 * | Kernel               |
 * | (APIC)               |
 * +----------------------+ 0x200000
 * | Kernel               |
 * | (text + data)        |
 * +----------------------+ 0x100000
 * | Kernel               |
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
void map_page(struct task *ctx, vaddr_t vaddr, paddr_t paddr)
{
	uint64_t addr_mask = 0x0007FFFFFFFFF800;

	vaddr_t *cadre = (vaddr_t *)ctx->pgt;
	int offset, i;

	printk("\nMapping v_addr %p to p_addr %p...\n", vaddr ,paddr);

	for (i = 4; i > 0; --i)
	{
		offset = INDEX(vaddr, i);
		printk("%s offset lvl%d %p \n", __func__, i, offset);
		printk("%s cadre without offset lvl%d %p \n", __func__, i, cadre);
		cadre = cadre + offset;
		printk("%s cadre with offset lvl%d %p \n", __func__, i, cadre);
		if (!cadre || !check_1bit(*cadre, 1))// if empty or invalid
		{
			printk("alloc needed\n");
			/*printk("%s *cadre lvl%d %p \n", __func__, i, *cadre);*/
			memcpy(cadre, (uint64_t*)alloc_page(), 8);
			set_usr(cadre); // set right to value pointed by cadre
		}
		cadre = (uint64_t*) (*cadre & addr_mask);
		printk("%s next cadre lvl%d %p \n", __func__, i-1, cadre);
	}


	printk("\n%s Gestion lvl%d %p \n", __func__, i, cadre);
	offset = INDEX(vaddr, i);
	cadre = cadre + offset;
	set_usr(&paddr);
	memcpy(cadre, &paddr, 8);
	printk("%s pointee %p \n", __func__, cadre);
	printk("%s alloc done (pointed) %p \n", __func__, *cadre);
}

// mappe l’adresse virtuelle vaddr sur l’adresse physique paddr sur
// un espace d’une page pour la tâche ctx
void map_page2(struct task *ctx, vaddr_t vaddr, paddr_t paddr)
{
	paddr_t *cadre;
	paddr_t tmp;

	paddr_t pml4;
	paddr_t pml3;
	paddr_t pml2;
	paddr_t pml1;

	paddr_t pml4_value;
	paddr_t pml3_value;
	paddr_t pml2_value;
	paddr_t pml1_value;

	vaddr_t index_lvl4;
	vaddr_t index_lvl3;
	vaddr_t index_lvl2;
	vaddr_t index_lvl1;

	// recuperation de la table du ctx
	pml4 = ctx->pgt;

	// recuperation de l'adresse de pml3, pml2, pml1
	pml3 = 0x105000;
	pml2 = 0x106000;
	pml1 = 0x107000;

	printk("%s virtual address %p \n", __func__, vaddr);
	printk("%s physical address %p \n", __func__, paddr);
	/*printk("%s adress lvl 4 %p \n", __func__, pml4);*/
	/*printk("%s adress lvl 3 %p \n", __func__, pml3);*/
	/*printk("%s adress lvl 2 %p \n", __func__, pml2);*/
	/*printk("%s adress lvl 1 %p \n", __func__, pml1);*/

	// calcule des index dans la table a partir de l'adresse virtuelle
	index_lvl4 = INDEX(vaddr, 4);
	index_lvl3 = INDEX(vaddr, 3);
	index_lvl2 = INDEX(vaddr, 2);
	index_lvl1 = INDEX(vaddr, 1);

	printk("%s index lvl 4 %p \n", __func__, index_lvl4);
	printk("%s index lvl 3 %p \n", __func__, index_lvl3);
	printk("%s index lvl 2 %p \n", __func__, index_lvl2);
	printk("%s index lvl 1 %p \n", __func__, index_lvl1);

	pml4_value = pml3+index_lvl3;
	set_1bit(&pml4_value, 0);
	set_1bit(&pml4_value, 1);
	set_1bit(&pml4_value, 2);

	pml3_value = pml2+index_lvl2;
	set_1bit(&pml3_value, 0);
	set_1bit(&pml3_value, 1);
	set_1bit(&pml3_value, 2);

	pml2_value = pml1+index_lvl1;
	set_1bit(&pml2_value, 0);
	set_1bit(&pml2_value, 1);
	set_1bit(&pml2_value, 2);

	pml1_value = paddr;

	printk("%s value lvl 4 %p \n", __func__, pml4_value);
	printk("%s value lvl 3 %p \n", __func__, pml3_value);
	printk("%s value lvl 2 %p \n", __func__, pml2_value);
	printk("%s value lvl 1 %p \n", __func__, pml1_value);

	cadre = (paddr_t*)pml4+index_lvl4;
	*cadre = pml4_value;
	printk("%s verification value lvl 4 %p \n", __func__, *cadre);

	cadre = (paddr_t*)pml3+index_lvl3;
	*cadre = pml3_value;
	printk("%s verification value lvl 3 %p \n", __func__, *cadre);

	cadre = (paddr_t*)pml2+index_lvl2;
	*cadre = pml2_value;
	printk("%s verification value lvl 2 %p \n", __func__, *cadre);

	cadre = (paddr_t*)pml1+index_lvl1;
	*cadre = pml1_value;
	printk("%s verification value lvl 1 %p \n", __func__, *cadre);
}

void load_task(struct task *ctx)
{
}

void set_task(struct task *ctx)
{
}

void mmap(struct task *ctx, vaddr_t vaddr)
{
}

void munmap(struct task *ctx, vaddr_t vaddr)
{
}

void pgfault(struct interrupt_context *ctx)
{
	printk("Page fault at %p\n", ctx->rip);
	printk("  cr2 = %p\n", store_cr2());
	asm volatile ("hlt");
}

void duplicate_task(struct task *ctx)
{
}
