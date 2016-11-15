//
// Created by Gregory Istratov on 11/12/16.
//

#include <string.h>
#include <stdio.h>

#include "types.h"
#include "vstring.h"
#include "hash_table.h"
#include "graph.h"
#include "json_parser.h"
#include "log.h"

void test_vstring()
{
    static const char* str1 = "Hello, World! ";
    static const char* str2 = "What's up? ";
    static const char* str3 = "How's ya doin?";

    size_t s_len = strlen(str1)+strlen(str2)+strlen(str3);
    char* s = malloc(s_len);
    memset(s, 0, s_len);
    memcpy(s, str1, strlen(str1));
    memcpy(s+strlen(str1), str2, strlen(str2));
    memcpy(s+strlen(str1)+strlen(str2), str3, strlen(str3));

    struct vstring_t vs;
    vstring_init(&vs);
    vstring_append(&vs, str1, strlen(str1));
    vstring_append(&vs, str2, strlen(str2));
    vstring_append(&vs, str3, strlen(str3));

    if(memcmp(s, vs.data, vs.size) != 0)
    {
        fprintf(stderr, "[test_vstring] Error: data is not equal [%s] != [%s]\n", s, vs.data);
        exit(1);
    }

    vstring_clear(&vs);

    vstring_append(&vs, str1, strlen(str1));
    vstring_append(&vs, str2, strlen(str2));
    vstring_append(&vs, str3, strlen(str3));

    if(memcmp(s, vs.data, vs.size) != 0)
    {
        fprintf(stderr, "[test_vstring] Error: data is not equal [%s] != [%s]\n", s, vs.data);
        exit(1);
    }

    vstring_destroy(&vs);

    free(s);
}

void test_hashtable()
{
#define ASSERT_HT_SET(key, value) { if(ht_set(&ht, key, value) != HT_OK) exit(1); }

#define ASSERT_HT_GET(key) {     if(ht_get(&ht, key, &found) != HT_OK) \
                                { \
                                    fprintf(stderr, "[test_hashtable] %s not found", key); \
                                    exit(1); \
                                } }

#define ASSERT_HT_VALUE(value) { if(strcmp(value, found) != 0) { fprintf(stderr, "[test_hashtable] failed [%s] != [%s] ", value, found); \
                                 exit(1);   }}\


    struct hashtable_t ht;
    char* found = NULL;

    if(ht_init(&ht) != HT_OK)
    {
        exit(1);
    }

    ASSERT_HT_SET("key0s0", "value0s0");
    ASSERT_HT_SET("key0s1", "value0s1");
    ASSERT_HT_SET("key1s0", "value1s0");
    ASSERT_HT_SET("key1s1", "value1s1");
    ASSERT_HT_SET("key2s0", "value2s0");
    ASSERT_HT_SET("key2s1", "value2s1");
    ASSERT_HT_SET("key2s2", "value2s2");
    ASSERT_HT_SET("key3s0", "value3s0");
    ASSERT_HT_SET("key3s1", "value3s1");
    ASSERT_HT_SET("key3s2", "value3s2");
    ASSERT_HT_SET("key3s3", "value3s3");

    ASSERT_HT_GET("key0s0");
    ASSERT_HT_VALUE("value0s0");

    ASSERT_HT_GET("key0s1");
    ASSERT_HT_VALUE("value0s1");

    ASSERT_HT_GET("key1s0");
    ASSERT_HT_VALUE("value1s0");

    ASSERT_HT_GET("key1s1");
    ASSERT_HT_VALUE("value1s1");

    ASSERT_HT_GET("key2s0");
    ASSERT_HT_VALUE("value2s0");

    ASSERT_HT_GET("key2s1");
    ASSERT_HT_VALUE("value2s1");

    ASSERT_HT_GET("key2s2");
    ASSERT_HT_VALUE("value2s2");

    ASSERT_HT_GET("key3s0");
    ASSERT_HT_VALUE("value3s0");

    ASSERT_HT_GET("key3s1");
    ASSERT_HT_VALUE("value3s1");

    ASSERT_HT_GET("key3s2");
    ASSERT_HT_VALUE("value3s2");

    ASSERT_HT_GET("key3s3");
    ASSERT_HT_VALUE("value3s3");

    ASSERT_HT_SET("key1s0", "test");

    ASSERT_HT_GET("key1s0");
    ASSERT_HT_VALUE("test");

    ht_destroy(&ht);

#undef ASSERT_HT_SET
#undef ASSERT_HT_GET
#undef ASSERT_HT_VALUE
}

static __always_inline struct string_slice_t get_str_slice(const char* str)
{
    struct string_slice_t ss;
    ss.start = (char*)str;
    ss.len = strlen(str);

    return ss;
}

void test_graph()
{

#define SG_VERTEX_APPEND_STR(v, str, nv) { \
    struct string_slice_t ss; \
    ss.start = (char*)str; \
    ss.len = strlen(str); \
    if(sg_vertex_append(&ss, v, nv) != ST_OK) \
    { \
        fprintf(stderr, "[test_graph] sg_vertex_append failed"); \
        exit(1);    \
    } \
}

#define SG_VERTEX_SEARCH_STR(str) { \
    struct string_slice_t ss = get_str_slice(str); \
    struct sg_vertex* found; \
    if(sg_search(&ss, root, &found, 0) != ST_OK) \
    { \
        fprintf(stderr, "[test_graph] sg_search failed"); \
        exit(1);    \
    } \
}


    struct sg_vertex *root = NULL;


    struct string_slice_t ssroot = get_str_slice("root");
    if(sg_vertex_create(&ssroot , &root) != ST_OK)
    {
        fprintf(stderr, "[test_graph] sg_vertex_create failed");
        exit(1);
    }

    struct sg_vertex* nv;
    SG_VERTEX_APPEND_STR(root, "key1", &nv);
    SG_VERTEX_APPEND_STR(nv, "value1", NULL);

    SG_VERTEX_APPEND_STR(root, "key2", &nv);
    SG_VERTEX_APPEND_STR(nv, "k2value1", NULL);
    SG_VERTEX_APPEND_STR(nv, "k2value2", NULL);
    SG_VERTEX_APPEND_STR(nv, "k2value3", &nv);

    struct sg_vertex* nv2;
    SG_VERTEX_APPEND_STR(nv, "subkey1", &nv2);
    SG_VERTEX_APPEND_STR(nv2, "subvalue1", NULL);
    SG_VERTEX_APPEND_STR(nv2, "subvalue2", NULL);
    SG_VERTEX_APPEND_STR(nv2, "subvalue3", NULL);

    SG_VERTEX_APPEND_STR(nv, "subkey2", &nv2);
    SG_VERTEX_APPEND_STR(nv2, "k2subvalue1", NULL);
    SG_VERTEX_APPEND_STR(nv2, "k2subvalue2", NULL);
    SG_VERTEX_APPEND_STR(nv2, "k2subvalue3", NULL);

    SG_VERTEX_APPEND_STR(nv, "subkey3", &nv2);
    SG_VERTEX_APPEND_STR(nv2, "k3subvalue1", NULL);
    SG_VERTEX_APPEND_STR(nv2, "k3subvalue2", NULL);
    SG_VERTEX_APPEND_STR(nv2, "k3subvalue3", &nv);

    SG_VERTEX_APPEND_STR(nv, "subsubkey1", &nv2);
    SG_VERTEX_APPEND_STR(nv2, "subsubvalue1", NULL);
    SG_VERTEX_APPEND_STR(nv2, "subsubvalue2", NULL);
    SG_VERTEX_APPEND_STR(nv2, "subsubvalue3", NULL);


    SG_VERTEX_SEARCH_STR("key1");
    SG_VERTEX_SEARCH_STR("value1");
    SG_VERTEX_SEARCH_STR("key2");
    SG_VERTEX_SEARCH_STR("k2value1");
    SG_VERTEX_SEARCH_STR("k2value2");
    SG_VERTEX_SEARCH_STR("k2value3");
    SG_VERTEX_SEARCH_STR("subkey1");
    SG_VERTEX_SEARCH_STR("subvalue1");
    SG_VERTEX_SEARCH_STR("subvalue2");
    SG_VERTEX_SEARCH_STR("subvalue3");
    SG_VERTEX_SEARCH_STR("subkey2");
    SG_VERTEX_SEARCH_STR("k2subvalue1");
    SG_VERTEX_SEARCH_STR("k2subvalue2");
    SG_VERTEX_SEARCH_STR("k2subvalue3");
    SG_VERTEX_SEARCH_STR("subkey3");
    SG_VERTEX_SEARCH_STR("k3subvalue1");
    SG_VERTEX_SEARCH_STR("k3subvalue2");
    SG_VERTEX_SEARCH_STR("k3subvalue3");
    SG_VERTEX_SEARCH_STR("subsubkey1");
    SG_VERTEX_SEARCH_STR("subsubvalue1");
    SG_VERTEX_SEARCH_STR("subsubvalue1");
    SG_VERTEX_SEARCH_STR("subsubvalue1");


    sg_recursive_free(root);
}

void test_json()
{
    const char* json = "{ \"key1\" : [ \"test\", 432, 3.14 ], \"val\" : true, \"obj\" : { \"dev\" : null } }";
    struct js_lexer_t lexer;
    if(js_fsm_scan(json, &lexer) != ST_OK)
    {
        LOG_ERROR("Error in json scanner");
        exit(EXIT_FAILURE);
    }

    LOG_DEBUG("== Parsed tokens == ");

    for(int i = 0; i < lexer.p; ++i)
        js_debug_token(&lexer.tokens[i]);
}

void run_tests()
{
    test_vstring();
    test_hashtable();
    test_graph();
    test_json();
}
