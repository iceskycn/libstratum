//
// Created by Gregory Istratov on 11/13/16.
//

#ifndef LIBSTRATUM_GRAPH_H
#define LIBSTRATUM_GRAPH_H

//*************************************************
// ******* simple non-copy string graph ***********
//*************************************************

#include <stdlib.h>
#include <stdint.h>


#define SG_MAX_EDGE_COUNT 32
#define SG_MAX_SEARCH_DEPTH 64

struct string_slice_t
{
    char* start;
    size_t len;
};

struct sg_vertex
{
    struct string_slice_t str;

    struct sg_vertex* edges[SG_MAX_EDGE_COUNT];
    size_t edge_count;
};

int sg_vertex_create(struct string_slice_t* str, struct sg_vertex** vx);

void sg_recursive_free(struct sg_vertex* v);

int sg_vertex_append(struct string_slice_t* str, struct sg_vertex* v, struct sg_vertex** new_vertex);

int sg_search(struct string_slice_t* str, struct sg_vertex* v, struct sg_vertex** found, uint32_t depth);

#endif //LIBSTRATUM_GRAPH_H
