//
// Created by greg on 11/13/16.
//

#include <immintrin.h>

#ifndef LIBSTRATUM_MEM_H
#define LIBSTRATUM_MEM_H

#define SAFE_RELEASE(x) { if(x) release(x); }

static inline void* alloc(size_t size, size_t align)
{
    return _mm_malloc(size, align);
}

static inline void* alloc8(size_t size)
{
    return _mm_malloc(size, 8);
}

static inline void* alloc16(size_t size)
{
    return _mm_malloc(size, 16);
}


static inline void* alloc32(size_t size)
{
    return _mm_malloc(size, 32);
}

static inline void release(void* p)
{
    _mm_free(p);
}

#endif //LIBSTRATUM_MEM_H
