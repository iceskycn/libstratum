//
// Created by Gregory Istratov on 11/12/16.
//

#ifndef LIBSTRATUM_VSTRING_H
#define LIBSTRATUM_VSTRING_H

#include <stdlib.h>

struct vstring_t
{
    char* data;
    size_t size;
    size_t allocated;
};

enum
{
    VSTRING_OK,
    VSTRING_ERR
};

void vstring_init(struct vstring_t* vs);
int vstring_create(struct vstring_t* vs, const char* data, size_t size);

int vstring_append(struct vstring_t* vs, const char* data, size_t size);
void vstring_clear(struct vstring_t* vs);
void vstring_destroy(struct vstring_t* vs);

#endif //LIBSTRATUM_VSTRING_H
