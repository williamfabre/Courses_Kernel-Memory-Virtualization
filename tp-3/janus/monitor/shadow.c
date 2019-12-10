#include "shadow.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "memory.h"
#include "state.h"
#include "vector.h"


void set_flat_mapping(size_t ram)
{
	fprintf(stderr, "set_flat_mapping unimplemented\n");
	exit(EXIT_FAILURE);
}

void set_page_table(void)
{
	paddr_t cr3 = mov_from_control(3);
}

int trap_read(vaddr_t addr, size_t size, uint64_t *val)
{
	display_mapping();
	fprintf(stderr, "trap_read unimplemented at %lx\n", addr);
	display_vga();
	exit(EXIT_FAILURE);
}

int trap_write(vaddr_t addr, size_t size, uint64_t val)
{
	display_mapping();
	fprintf(stderr, "trap_write unimplemented at %lx\n", addr);
	exit(EXIT_FAILURE);
}
