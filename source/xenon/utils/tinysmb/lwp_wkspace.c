#include <stdlib.h>
#include <string.h>
#include "machine/asm.h"
#include "machine/processor.h"
#include "lwp_wkspace.h"

#define ROUND32UP(v)			(((u32)(v)+31)&~31)

heap_cntrl __wkspace_heap;
static heap_iblock __wkspace_iblock;
static u32 __wkspace_heap_size = 0;

u32 __lwp_wkspace_heapsize()
{
	return __wkspace_heap_size;
}

u32 __lwp_wkspace_heapfree()
{
	__lwp_heap_getinfo(&__wkspace_heap,&__wkspace_iblock);
	return __wkspace_iblock.free_size;
}

u32 __lwp_wkspace_heapused()
{
	__lwp_heap_getinfo(&__wkspace_heap,&__wkspace_iblock);
	return __wkspace_iblock.used_size;
}

void __lwp_wkspace_init(u32 size)
{
	void * arLo;

	arLo = malloc(size*4);

	memset((void*)arLo,0,size);
	__wkspace_heap_size += __lwp_heap_init(&__wkspace_heap,(void*)arLo,size,PPC_ALIGNMENT);
}
