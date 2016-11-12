//
// Created by greg on 11/12/16.
//

#include "vstring.h"

#include <string.h>
#include <stdio.h>

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

    printf("[test_vstring]: Initial full string: %s", s);

    struct vstring_t vs;
    vstring_init(&vs);
    vstring_append(&vs, str1, strlen(str1));
    vstring_append(&vs, str2, strlen(str2));
    vstring_append(&vs, str3, strlen(str3));

    if(memcmp(s, vs.data, vs.size) != 0)
    {
        sprintf(stderr, "[test_vstring] Error: data is not equal [%s] != [%s]", s, vs.data);
        exit(1);
    }

    vstring_clear(&vs);

    vstring_append(&vs, str1, strlen(str1));
    vstring_append(&vs, str2, strlen(str2));
    vstring_append(&vs, str3, strlen(str3));

    if(memcmp(s, vs.data, vs.size) != 0)
    {
        sprintf(stderr, "[test_vstring] Error: data is not equal [%s] != [%s]", s, vs.data);
        exit(1);
    }

    vstring_destroy(&vs);

    free(s);
}