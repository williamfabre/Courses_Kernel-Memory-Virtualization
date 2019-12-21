#include <stdio.h>
#include <inttypes.h>

int main(void)
{
	for (uint64_t i = 0; i < 0x1000; i++)
	{
		// 0 -> 127
		// 18446744073709551488
		// FFFFFFFFFFFFFF80

		/*addr[i] = i & 0xff;*/
		// 0x1fffff3000
		printf("adversary addr %p = %p\n", (void*)i, (void*)(i & 0xff));
		/*syscall_printnum(addr[i]);*/
		/*syscall_printnum(i & 0xff);*/
		/*syscall_print("\n");*/
	}

	return 0;
}
