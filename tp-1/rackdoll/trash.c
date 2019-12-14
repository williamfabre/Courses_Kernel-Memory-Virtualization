void printpgt_uint64(uint64_t* pmlx, int layer, int iterations)
{
	uint64_t *pmlx_pointer = pmlx;

	if (layer == 4)
	{
		printk("\n actual layer 4 %p\n", *pmlx_pointer);
		return;
	}

	if (*pmlx_pointer != 0)
	{
		/*printk("\n non null = %p, layer = %d\n", *pmlx_pointer, layer);*/
		*pmlx_pointer = ((uint64_t)*pmlx_pointer & 0xFFF000); // bit masking

		printk("\n accessing %p\n", *pmlx_pointer);

		printpgt_uint64(pmlx_pointer, layer++, 256);

		printk("\n after %d\n");
	} else {
		printk("\n else %d\n");
		printpgt_uint64(pmlx++, layer, iterations--);
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

// SOME MORE FUN
/* lea eax, [var] — the value in var is placed in EAX. */
// mov [0x12], ax ; Copie le contenu de ax dans la zone mémoire 0x12.
// mov ah, [0x13] ; Copie le contenu de la zone mémoire 0x13 dans le registre ah.
//


//__asm__("movl\t%0, %1"

	//: "=&r" (a)
	//: "r" (b)
	//:[> liste des modifications <]
       //)
//static inline void mov(uint64_t val_64)
//{
	//asm volatile ("mov %0, %1"
			//: "rsp"
			//: "r"(val_64));
//}

// version recurssive
void printpgt(uint64_t cr3)
{
	int layer = 4;

	/*void print_tab_layer(layer);*/
	/*printk("layer %d => %p\n", layer, cr3);*/

	printpgt_rec((paddr_t)cr3, layer, 256);
}


void printpgt_rec(paddr_t pmlx, int layer, int iterations)
{
	paddr_t *pmlx_pointer = pmlx;
	paddr_t *pointed;

	if( iterations == 0)
		return;

	if (layer == 1){
		pointed = *pmlx_pointer;
		/*print_tab_layer(layer);*/
		/*printk("layer %d => accessing = %p\n", layer,  pointed);*/
	}

	if (layer == 2){
		pointed = *pmlx_pointer;
		pointed = (int)*pmlx_pointer & 0xFFF000; // bit masking
		print_tab_layer(layer);
		printk("layer %d => accessing = %p\n", layer,  pointed);
		layer = layer - 1;
		printpgt_rec((uint64_t)pointed, layer, 256);

	}
	if (layer == 3){
		/*if (CHECK_BIT((int)pmlx_pointer, 0)){			// VALID POINTER*/
		pointed = *pmlx_pointer;
		pointed = (int)*pmlx_pointer & 0xFFF000; // bit masking
		print_tab_layer(layer);
		printk("layer %d => accessing = %p\n", layer,  pointed);
		layer = layer - 1;
		printpgt_rec((uint64_t)pointed, layer, 256);
		/*}*/
	}


	if (layer == 4){
		/*printk("debug");*/
		/*if (CHECK_BIT((int)pmlx_pointer, 0)){			// VALID POINTER*/
		pointed = *pmlx_pointer;
		pointed = (int)*pmlx_pointer & 0xFFF000; // bit masking
		print_tab_layer(layer);
		printk("layer %d => accessing = %p\n", layer,  pointed);
		layer = layer - 1;
		printpgt_rec((uint64_t)pointed, layer, 256);
		/*}*/
	}

	pmlx = pmlx + 1;
	iterations = iterations - 1;
	printpgt_rec(pmlx, layer, iterations);


	/*paddr_t *pmlx_pointer = pmlx;*/
	/*paddr_t *pointed;*/

	/*// fin du tableau*/
	/*if( iterations == 0)*/
	/*{*/
	/*print_tab_layer(layer);*/
	/*printk("ending layer %d\n", layer);*/
	/*return;*/
	/*}*/

	/*// derniere couche (page normale 4KO)*/
	/*if (CHECK_BIT((int)pmlx_pointer, 0)			// VALID POINTER*/
	/*&& layer == 1)					// NORMAL PAGE*/
	/*{*/
	/*[>print_tab_layer(layer);<]*/
	/*[>printk("layer %d => accessing normal PAGE= %d\n", layer,  *pmlx_pointer);<]*/
	/*}*/

	/*// HUGE PAGE (2mo)*/
	/*if (CHECK_BIT((int)pmlx_pointer, 0)			// VALID POINTER*/
	/*&& layer == 2					// LAYER 2*/
	/*&& CHECK_BIT((int)pmlx_pointer, 7))		// HUGE PAGE*/
	/*{*/
	/*print_tab_layer(layer);*/
	/*printk("layer %d => accessing HUGE PAGE= %d\n", layer, *pmlx_pointer);*/
	/*}*/

	/*// valide (presence)*/
	/*if (CHECK_BIT((int)pmlx_pointer, 0)			// VALID POINTER*/
	/*&& layer == 3 || layer == 4)*/
	/*{*/
	/*print_tab_layer(layer);*/
	/*pointed = *pmlx_pointer;*/


	/*printk("layer %d => %p\n", layer, pmlx_pointer);*/

	/*pointed = (int)*pmlx_pointer & 0xFFF000; // bit masking*/

	/*print_tab_layer(layer);*/
	/*printk("layer %d => accessing = %p\n", layer,  pointed);*/


	/*layer = layer - 1;*/

	/*printpgt_rec((uint64_t)pointed, layer, 256);*/
	/*} else {*/
	/*pmlx = pmlx + 1;*/
	/*iterations = iterations - 1;*/
	/*printpgt_rec(pmlx, layer, iterations);*/
	/*}*/
}

// version iterative
void printpgt_test(uint64_t cr3)
{
	uint64_t *p_val_64 = 0;
	uint64_t nb_0 = 0;

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
