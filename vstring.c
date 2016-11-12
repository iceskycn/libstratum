//
// Created by Gregory Istratov on 11/12/16.
//


#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "vstring.h"

void vstring_init(struct vstring_t* vs)
{
    memset(vs, 0, sizeof(struct vstring_t));
}

int vstring_create(struct vstring_t* vs, const char* data, size_t size)
{
    size_t to_allocate = size*2;
    vs->data = malloc(to_allocate);
    if(vs->data == NULL)
    {
        fprintf(stderr, "vstring_create can't alloc %lu bytes", to_allocate);
        return VSTRING_ERR;
    }

    vs->allocated = to_allocate;

    memcpy(vs->data, data, size);
    vs->size = size;

    return VSTRING_OK;
}

int vstring_append(struct vstring_t* vs, const char* data, size_t size)
{
    if(vs->allocated - vs->size < size)
    {
        size_t to_alloc = (vs->allocated + size) * 2;
        char* new_buffer = malloc(to_alloc);
        if(new_buffer == NULL)
        {
            fprintf(stderr, "vstring_append can't alloc %lu bytes", to_alloc);
            return VSTRING_ERR;
        }

        memcpy(new_buffer, vs->data, vs->size);

        vs->allocated = to_alloc;

        free(vs->data);

        vs->data = new_buffer;
    }

    memcpy(vs->data+vs->size, data, size);
    vs->size = vs->size + size;

    return VSTRING_OK;
}

void vstring_clear(struct vstring_t* vs)
{
    vs->size = 0;
}

void vstring_destroy(struct vstring_t* vs)
{
    free(vs->data);
    vs->data = NULL;
    vs->size = 0;
    vs->allocated = 0;
}