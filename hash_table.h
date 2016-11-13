//
// Created by Gregory Istratov on 11/13/16.
//

#include <stdlib.h>

#ifndef LIBSTRATUM_HASH_TABLE_H
#define LIBSTRATUM_HASH_TABLE_H

#define HASHTABLE_SIZE 7

enum {
    HT_ERR,
    HT_OK,
    HT_NOT_FOUND
};

struct ht_item_t
{
    char* name;
    char* value;
    unsigned long hash;
    struct ht_item_t* next;
};

struct hashtable_t
{
    struct ht_item_t* table[HASHTABLE_SIZE];
};

int ht_init(struct hashtable_t* ht);

int ht_set(struct hashtable_t* ht, const char* name, const char* value);

///@value don't need to free
int ht_get(struct hashtable_t* ht, const char* name, char** value);

void ht_destroy(struct hashtable_t* ht);

#endif //LIBSTRATUM_HASH_TABLE_H
