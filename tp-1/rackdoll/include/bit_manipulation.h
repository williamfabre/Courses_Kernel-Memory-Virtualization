#ifndef _INCLUDE_BIT_MANIPULATION_H_
#define _INCLUDE_BIT_MANIPULATION_H_

// retrouve l'index pour une adresse virtuel et un niveau donne.
#define INDEX(vaddr, lvl)    (((((vaddr<<16)>>28)>>((lvl-1))*9)<<55)>>55)

// invalide le invalieme bit de l'adresse pointee par addr
void mask_1bit(paddr_t* addr, int inval)
{
	unsigned long long val = (1ULL << inval);
	*addr &= ~val;
}

// met le ieme bit a 1 de l'adresse pointee par addr
void set_1bit(paddr_t* addr, int i)
{
	unsigned long long val = (1ULL << i);
	*addr |= val;
}

// verifie si le bit check est actif dans l'adresse addr
int check_1bit(paddr_t addr, int check)
{
	unsigned long long val = (1ULL << check);
	return (addr & val);
}

void mask_63_11downto0(paddr_t* pml)
{
	// correction du probleme de depassement du shift avec un uint64_t
	// bit masking bit 63
	mask_1bit(pml, 63);

	// bit masking bits [11 downto 0]
	for (uint64_t i = 0; i < 12; i++){
		mask_1bit(pml, i);
	}
}
void set_usr(paddr_t* addr)
{
	set_1bit(addr, 0);
	set_1bit(addr, 1);
	set_1bit(addr, 2);
}

#endif
