//
// Created by Gregory Istratov on 11/13/16.
//
#include <string.h>
#include <stdio.h>
#include "mem.h"

#include "hash_table.h"

static __always_inline unsigned long ht_hash(const char* _str)
{
    const unsigned char* str = (const unsigned char*)_str;
    unsigned long hash = 5381;
    int c;

    while((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}


int ht_init(struct hashtable_t* ht)
{
    memset(ht, 0, sizeof(struct hashtable_t));
    /*
    size_t data_to_alloc = HASHTABLE_SIZE * sizeof(struct ht_item_t);
    void* data_space = NULL;

    if((data_space = alloc16(data_to_alloc)) == NULL)
    {
        fprintf(stderr, "[ht_init] can't alloc");
        return HT_ERR;
    }

    memset(data_space, 0, data_to_alloc);

    struct ht_item_t* p = (struct ht_item_t*)data_space;
    for(size_t i = 0; i < HASHTABLE_SIZE; ++i)
    {
        ht->table[i]= p++;
    }
    */

    return HT_OK;
}

static __always_inline void ht_destroy_item(struct ht_item_t* item)
{
    if(item->name) release(item->name);
    if(item->value) release(item->value);
    release(item);
}

static __always_inline void ht_destroy_items_line(struct ht_item_t* start_item)
{
    struct ht_item_t* next = start_item;
    struct ht_item_t* tmp = NULL;
    while(next)
    {
        tmp = next;
        next = next->next;

        ht_destroy_item(tmp);
    }
}


void ht_destroy(struct hashtable_t* ht)
{
    for(size_t i = 0; i < HASHTABLE_SIZE; ++i) {
        ht_destroy_items_line(ht->table[i]);
    }
}

int ht_create_item(struct ht_item_t** pitem, const char* name, unsigned long name_hash, const char* value)
{
    struct ht_item_t* item;
    if((item = alloc8(sizeof(struct ht_item_t))) == NULL)
    {
        fprintf(stderr, "[ht_create_item] can't alloc");
        return HT_ERR;
    }
    memset(item, 0, sizeof(struct ht_item_t));

    if((item->name = alloc8(strlen(name))) == NULL)
    {
        release(item);
        fprintf(stderr, "[ht_create_item] can't alloc");
        return HT_ERR;
    }

    strcpy(item->name, name);

    if((item->value = alloc8(strlen(value))) == NULL)
    {
        release(item->name);
        release(item);
        fprintf(stderr, "[ht_create_item] can't alloc");
        return HT_ERR;
    }

    strcpy(item->value, value);

    item->hash = name_hash;

    *pitem = item;

    return HT_OK;
}
int ht_set(struct hashtable_t* ht, const char* name, const char* value)
{
    unsigned long hash = ht_hash(name);
    unsigned long bin =  hash % HASHTABLE_SIZE;

    struct ht_item_t* item = ht->table[bin];
    struct ht_item_t* prev = NULL;
    while(item)
    {
        if(item->hash == hash)
            break;

        prev = item;
        item = item->next;
    }


    if(item && item->hash == hash)
    {
        char* tmp_val = NULL;
        if((tmp_val = alloc8(strlen(value))) == NULL)
        {
            fprintf(stderr, "[ht_set] can't alloc");
            return HT_ERR;
        }

        release(item->value);
        item->value = tmp_val;

        strcpy(item->value, value);
    }
    else
    {
        struct ht_item_t* new_item = NULL;
        if((ht_create_item(&new_item, name, hash, value)) != HT_OK)
        {
            return HT_ERR;
        }

        if(prev)
            prev->next = new_item;
        else
            ht->table[bin] = new_item;
    }

    return HT_OK;
}


int ht_get(struct hashtable_t* ht, const char* name, char** value)
{
    unsigned long hash = ht_hash(name);
    unsigned long bin =  hash % HASHTABLE_SIZE;

    struct ht_item_t* item = ht->table[bin];

    while(item)
    {
        if(item->hash == hash)
        {
            *value = item->value;
            return HT_OK;
        }

        item = item->next;
    }

    return HT_NOT_FOUND;
}

