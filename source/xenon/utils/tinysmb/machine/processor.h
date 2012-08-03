#ifndef __PROCESSOR_H__
#define __PROCESSOR_H__

#include <xetypes.h>
#include "asm.h"

#define __stringify(rn)								#rn
#define ATTRIBUTE_ALIGN(v)							__attribute__((aligned(v)))
// courtesy of Marcan
#define STACK_ALIGN(type, name, cnt, alignment)		u8 _al__##name[((sizeof(type)*(cnt)) + (alignment) + (((sizeof(type)*(cnt))%(alignment)) > 0 ? ((alignment) - ((sizeof(type)*(cnt))%(alignment))) : 0))]; \
													type *name = (type*)(((u32)(_al__##name)) + ((alignment) - (((u32)(_al__##name))&((alignment)-1))))

#define ticks_to_millisecs(ticks)	(((u64)(ticks)/(u64)(PPC_TIMEBASE_FREQ)))

#define gettime mftb

#ifdef __cplusplus
   }
#endif /* __cplusplus */

#endif
