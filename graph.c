//
// Created by Gregory Istratov on 11/13/16.
//

//*************************************************
// ******* simple non-copy string graph ***********
//*************************************************

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "graph.h"

#include "mem.h"
#include "types.h"


static __always_inline int string_slice_cmp(const struct string_slice_t* a, const struct string_slice_t* b)
{
    if(a->len != b->len)
        return (int)(a->len - b->len);

    return memcmp(a->start, b->start, a->len);
}

int sg_vertex_create(struct string_slice_t* str, struct sg_vertex** vx)
{
    struct sg_vertex* v = calloc(1, sizeof(struct sg_vertex));
    ALLOC_RET_CHECK(v);

    v->str = *str;

    *vx = v;

    return ST_OK;
}

void sg_recursive_free(struct sg_vertex* v)
{
    if(!v)
        return;

    for(size_t i = 0; i < v->edge_count; ++i)
    {
        sg_recursive_free(v->edges[i]);
    }

    free(v);
}


int sg_vertex_append(struct string_slice_t* str, struct sg_vertex* v, struct sg_vertex** new_vertex)
{
    if(v->edge_count >= SG_MAX_EDGE_COUNT)
        return ST_SIZE_EXCEED;

    struct sg_vertex* nv;
    if(sg_vertex_create(str, &nv) != ST_OK)
        return ST_ERR;

    v->edges[v->edge_count++] = nv;

    if(new_vertex)
        *new_vertex = nv;

    return ST_OK;
}

int sg_search(struct string_slice_t* str, struct sg_vertex* v, struct sg_vertex** found, uint32_t depth)
{
    if(depth >= SG_MAX_SEARCH_DEPTH)
        return ST_NOT_FOUND;

    ++depth;

    if(string_slice_cmp(str, &v->str) == 0)
    {
        *found = v;
        return ST_OK;
    }

    for(size_t i = 0; i < v->edge_count; ++i)
    {
        if(sg_search(str, v->edges[i], found, depth) == ST_OK)
        {
            return ST_OK;
        }
    }

    return ST_NOT_FOUND;
}
