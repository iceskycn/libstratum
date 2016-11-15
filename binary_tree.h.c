//
// Created by Gregory Istratov on 11/13/16.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "mem.h"

#define BNODE_MAX_LEAFS 16

enum
{
    BNODE_COPY = 0x1,
    BNODE_INSERT_OVERWRITE = 0x1 << 1
};

struct bnode_t
{
    void* key;
    size_t key_len;

    int flags;

    struct bnode_t* left;
    struct bnode_t* right;
    struct bnode_t* leafs[BNODE_MAX_LEAFS];
    size_t leaf_size;
};

typedef int (*bnode_compare)(const void* a, size_t a_len, const void* b, size_t b_len);

static __always_inline int bnode_set(void* key, size_t key_len, struct bnode_t* node, int flags)
{
    if(flags & BNODE_COPY)
    {
        node->key = malloc(key_len);
        ALLOC_RET_CHECK(node->key);
        memcpy(node->key, key, key_len);
        node->key_len = key_len;
        node->flags |= BNODE_COPY;
    }
    else
    {
        node->key_len = key_len;
        node->key = key;
    }

    return ST_OK;
}

static __always_inline int bnode_create(void* key, size_t key_len, struct bnode_t** node, int flags)
{
    struct bnode_t* new_node = (struct bnode_t*)calloc(1, sizeof(struct bnode_t));
    ALLOC_RET_CHECK(new_node);

    if(bnode_set(key, key_len, new_node, flags) != ST_OK) {
        free(new_node);
        return ST_ERR;
    }

    *node = new_node;
    return ST_OK;
}

int bnode_insert(void* key, size_t key_len, struct bnode_t* node, bnode_compare cmp, int flags)
{
    int cres = cmp(key, key_len, node->key, node->key_len);

    if(cres < 0)
    {
        if(node->left)
            return bnode_insert(key, key_len, node, cmp, flags);

        struct bnode_t* new_node = NULL;
        if(bnode_create(key, key_len, &new_node, flags) != ST_OK)
        {
            return ST_ERR;
        }

        node->left = new_node;

        return ST_OK;
    }
    else if(cres > 0)
    {
        if(node->right)
            return bnode_insert(key, key_len, node, cmp, flags);

        struct bnode_t* new_node = NULL;
        if(bnode_create(key, key_len, &new_node, flags) != ST_OK)
        {
            return ST_ERR;
        }

        node->right = new_node;

        return ST_OK;
    }
    else
    {
        if(flags & BNODE_INSERT_OVERWRITE)
        {
            if(bnode_set(key, key_len, node, flags) != ST_OK)
                return ST_ERR;

            return ST_OK;
        }
        else
        {
            return ST_EXISTS;
        }
    }
}

int bnode_append_leaf(void* key, size_t key_len, void* leaf_key, size_t leaf_key_size, struct bnode_t* node, bnode_compare cmp, int flags)
{
    int cres = cmp(key, key_len, node->key, node->key_len);

    if(cres < 0)
    {
        if(node->left)
            return bnode_insert(key, key_len, node, cmp, flags);

        return ST_NOT_FOUND;
    }
    else if(cres > 0)
    {
        if(node->right)
            return bnode_insert(key, key_len, node, cmp, flags);

        return ST_NOT_FOUND;
    }
    else
    {
        if(node->leaf_size >= BNODE_MAX_LEAFS)
            return ST_SIZE_EXCEED;


        struct bnode_t* new_node = NULL;
        if(bnode_create(leaf_key, leaf_key_size, &new_node, flags) != ST_OK)
        {
            return ST_ERR;
        }

        node->leafs[node->leaf_size++] = new_node;

        return ST_OK;
    }
}

int bnode_search(void* key, size_t key_len, struct bnode_t* node, bnode_compare cmp, int flags, void** found_key, size_t* found_key_size)
{
    int cres = cmp(key, key_len, node->key, node->key_len);

    if(cres < 0)
    {
        if(node->left)
            return bnode_search(key, key_len, node, cmp, flags, found_key, found_key_size);
    }
    else if(cres > 0)
    {
        if(node->right)
            return bnode_search(key, key_len, node, cmp, flags, found_key, found_key_size);
    }
    else
    {
        if(found_key)
        {
            if(flags & BNODE_COPY)
            {
                *found_key = alloc16(node->key_len);
                ALLOC_RET_CHECK((*found_key));
                memcpy(*found_key, node->key, node->key_len);

                *found_key_size = node->key_len;
            }
            else
            {
                *found_key = node->key;
                *found_key_size = node->key_len;
            }
        }
        return ST_OK;
    }

    return ST_NOT_FOUND;
}

int bnode_search_n(void* key, size_t key_len, struct bnode_t* node, bnode_compare cmp, int flags, struct bnode_t** found)
{
    int cres = cmp(key, key_len, node->key, node->key_len);

    if(cres < 0)
    {
        if(node->left)
            return bnode_search_n(key, key_len, node, cmp, flags, found);
    }
    else if(cres > 0)
    {
        if(node->right)
            return bnode_search_n(key, key_len, node, cmp, flags, found);
    }
    else
    {
        if(found)
        {
            *found = node;
        }
        return ST_OK;
    }

    return ST_NOT_FOUND;
}

int bnode_recursive_remove(struct bnode_t* node)
{
    if(!node)
        return ST_OK;

    if(node->flags & BNODE_COPY)
    {
        free(node->key);
        node->key = NULL;
    }

    for(size_t i = 0; i < node->leaf_size; ++i)
        bnode_recursive_remove(node->leafs[i]);

    bnode_recursive_remove(node->left);
    bnode_recursive_remove(node->right);

    free(node);

    return ST_OK;
}

int string_compare(const void* a, size_t a_len, const void* b, size_t b_len)
{
    if(a_len != b_len)
        return (int)(a_len - b_len);

    return strcmp((const char*)a, (const char*)b);
}
