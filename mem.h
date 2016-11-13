//
// Created by Gregory Istratov on 11/13/16.
//

#include <stdlib.h>
#include <stdio.h>

#ifndef LIBSTRATUM_MEM_H
#define LIBSTRATUM_MEM_H

#define SAFE_RELEASE(x) { if(x) free(x); }

#define ALLOC_RET_CHECK(x) { if(x == NULL) { fprintf(stderr, "[%s] Can't alloc\n", __PRETTY_FUNCTION__); return ST_ERR; } }
#define ALLOC_RELRET_CHECK(x, y) { if(x == NULL) {  if(y) free(y); fprintf(stderr, "[%s] Can't alloc\n", __PRETTY_FUNCTION__); return ST_ERR; } }

static __always_inline void* alloc8(size_t size)
{
    return aligned_alloc(8, size);
}

static __always_inline void* alloc16(size_t size)
{
    return aligned_alloc(16, size);
}


static __always_inline void* alloc32(size_t size)
{
    return aligned_alloc(32, size);
}

static __always_inline void* alloc64(size_t size)
{
    return aligned_alloc(64, size);
}

#endif //LIBSTRATUM_MEM_H
