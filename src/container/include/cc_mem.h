#ifndef __CC_MEM_H__
#define __CC_MEM_H__

#include <stddef.h>
#include "../cc_conf.h"


void * cc_alloc(size_t size);
void cc_free(void * ptr);

#endif /* __CC_MEM_H__ */
