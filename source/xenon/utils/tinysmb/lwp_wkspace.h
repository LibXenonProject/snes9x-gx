#ifndef __LWP_WKSPACE_H__
#define __LWP_WKSPACE_H__

#include <xetypes.h>
#include "lwp_heap.h"

#ifdef __cplusplus
extern "C" {
#endif

extern heap_cntrl __wkspace_heap;

void __lwp_wkspace_init(u32 size);

#include "lwp_wkspace.inl"

#ifdef __cplusplus
	}
#endif

#endif
