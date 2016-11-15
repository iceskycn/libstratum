//
// Created by Gregory Istratov on 11/15/16.
//

#include <stdlib.h>
#include <string.h>
#include "mem.h"
#include "types.h"

#ifndef LIBSTRATUM_STACK_H
#define LIBSTRATUM_STACK_H

#define FIXED_STACK_SIZE 100

struct fstack_int_t
{
    int arr[FIXED_STACK_SIZE];
    int p;
};


__always_inline void fstack_int_init(struct fstack_int_t* stack)
{
    memset(stack, 0, sizeof(struct fstack_int_t));
}

__always_inline int fstack_int_push(struct fstack_int_t* stack, int i)
{
    if(stack->p >= FIXED_STACK_SIZE)
        return ST_SIZE_EXCEED;

    stack->arr[stack->p++] = i;

    return ST_OK;
}

__always_inline int fstack_int_top(struct fstack_int_t* stack, int* i)
{
    if(stack->p == 0)
        return ST_EMPTY;

    *i = stack->arr[stack->p-1];

    return ST_OK;
}

__always_inline int fstack_int_pop(struct fstack_int_t* stack, int* i)
{
    if(stack->p == 0)
        return ST_EMPTY;

    if(i)
        *i = stack->arr[stack->p--];
    else
        --stack->p;

    return ST_OK;
}

#endif //LIBSTRATUM_STACK_H
