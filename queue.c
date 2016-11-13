//
// Created by Gregory Istratov on 11/12/16.
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "queue.h"

/*
static inline int _queue_node_create(struct queue_node_t** node)
{
    *node = malloc(sizeof(struct queue_node_t));
    if(*node == NULL)
    {
        fprintf(stderr, "queue_node malloc failed");
        return QUEUE_ERR;
    }
    memset(*node, 0, sizeof(struct queue_node_t));

    return QUEUE_OK;
}


static inline void _queue_node_free(struct queue_node_t* node)
{
    if(node->data) free(node->data);

    free(node);
}
*/

void queue_init(struct queue_t* queue)
{
    memset(queue, 0, sizeof(struct queue_t));
}

int queue_push(struct queue_t* queue, void* data, size_t size)
{
    struct queue_node_t** plast = &queue->head;

    while(*plast)
    {
        plast = &(*plast)->prev;
    }

    struct queue_node_t* node = malloc(sizeof(struct queue_node_t));
    if(node == NULL)
    {
        fprintf(stderr, "queue_node malloc failed");
        return QUEUE_ERR;
    }

    memset(node, 0, sizeof(struct queue_node_t));

    node->data = malloc(size);
    if(node->data == NULL)
    {
        free(node);
        fprintf(stderr, "queue data malloc failed");
        return QUEUE_ERR;
    }

    node->size = size;

    memcpy(node->data, data, size);

    *plast = node;

    return QUEUE_OK;
}

int queue_top(struct queue_t* queue, void** pdata, size_t* size)
{
    if(queue->head == NULL)
        return QUEUE_EMPTY;

    *pdata = queue->head->data;
    *size = queue->head->size;

    return QUEUE_OK;
}

int queue_pop(struct queue_t* queue, void** pdata, size_t* size)
{
    if(queue->head == NULL)
        return QUEUE_EMPTY;

    if(pdata != NULL) {
        *pdata = malloc(queue->head->size);
        if (*pdata == NULL) {
            fprintf(stderr, "queue_pop data malloc failed");
            return QUEUE_ERR;
        }

        *size = queue->head->size;
        bcopy(queue->head->data, *pdata, queue->head->size);
    }

    struct queue_node_t* tmp = queue->head;
    queue->head = queue->head->prev;

    free(tmp);

    return QUEUE_OK;
}

void queue_free(struct queue_t* queue)
{
    while(queue_pop(queue, NULL, NULL) != QUEUE_EMPTY);
}

