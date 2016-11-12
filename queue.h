//
// Created by Greg Miller on 11/12/16.
//

#ifndef LIBSTRATUM_QUEUE_H
#define LIBSTRATUM_QUEUE_H

#include <stdlib.h>

//*****************************************************//
//*************** SIMPLE LINKED-LIST QUEUE ************//
//*****************************************************//

enum
{
    QUEUE_OK,
    QUEUE_ERR,
    QUEUE_EMPTY
};

struct queue_node_t
{
    void* data;
    size_t size;
    struct queue_node_t* prev;
};

struct queue_t
{
    struct queue_node_t* head;
};

void queue_init(struct queue_t* queue);
void queue_free(struct queue_t* queue);
int queue_push(struct queue_t* queue, void* data, size_t size);
/// you don't need too free data
int queue_top(struct queue_t* queue, void** pdata, size_t* size);
int queue_pop(struct queue_t* queue, void** pdata, size_t* size);

#endif //LIBSTRATUM_QUEUE_H
