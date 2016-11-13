//
// Created by Gregory Istratov on 11/12/16.
//

#include <string.h>
#include <stdio.h>

#include "vstring.h"
#include "hash_table.h"

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

    printf("[test_vstring]: Initial full string: %s\n", s);

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

void run_tests()
{
    test_vstring();
    test_hashtable();
}
