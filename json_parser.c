//
// Created by Gregory Istratov on 11/15/16.
//

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "stack.h"
#include "log.h"

#define JS_FSM_NUM_STATES 50
#define JS_LEXER_MAX_TOKENS 50

enum {
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_FLOAT,
    JSON_INTEGER,
    JSON_STRING
};


struct js_token_t
{
    int type;
    char* ts;
    char* te;
};

struct js_lexer_t
{
    struct js_token_t tokens[JS_LEXER_MAX_TOKENS];
    int p;
};

__always_inline int js_lexer_push_token(struct js_lexer_t* lexer, int type, char* ts, char* te)
{
    if(lexer->p >= JS_LEXER_MAX_TOKENS) {
        LOG_ERROR("Stack size exceeds");
        return ST_SIZE_EXCEED;
    }

    lexer->tokens[lexer->p].type = type;
    lexer->tokens[lexer->p].ts = ts;
    lexer->tokens[lexer->p].te = te;
    ++lexer->p;

    return ST_OK;
}

enum
{
    JS_FSM_STATE_ERROR = 0,
    JS_FSM_STATE_INITIAL,
    JS_FSM_STATE_OBJECT,
    JS_FSM_STATE_STRING,
    JS_FSM_STATE_CHAR,
    JS_FSM_STATE_NUMBER,
    JS_FSM_STATE_INT,
    JS_FSM_STATE_FLOAT,
    JS_FSM_STATE_BEGIN_OBJECT,
    JS_FSM_STATE_END_OBJECT,
    JS_FSM_STATE_QUOTE,
    JS_FSM_STATE_COLON,
    JS_FSM_STATE_DOT,
    JS_FSM_STATE_WS,
    JS_FSM_STATE_EOF
};

static const int js_fsm_translation_table[JS_FSM_NUM_STATES][3] =
{
        {JS_FSM_STATE_INITIAL, JS_FSM_STATE_WS, JS_FSM_STATE_INITIAL},
        {JS_FSM_STATE_INITIAL, JS_FSM_STATE_BEGIN_OBJECT, JS_FSM_STATE_OBJECT},
        {JS_FSM_STATE_OBJECT, JS_FSM_STATE_OBJECT, JS_FSM_STATE_OBJECT},
        {JS_FSM_STATE_OBJECT, JS_FSM_STATE_END_OBJECT, JS_FSM_STATE_INITIAL},
        {JS_FSM_STATE_INITIAL, JS_FSM_STATE_COLON, JS_FSM_STATE_INITIAL},
        {JS_FSM_STATE_INITIAL, JS_FSM_STATE_QUOTE, JS_FSM_STATE_STRING},
        {JS_FSM_STATE_STRING, JS_FSM_STATE_STRING, JS_FSM_STATE_STRING},
        {JS_FSM_STATE_STRING, JS_FSM_STATE_QUOTE, JS_FSM_STATE_INITIAL},

};

struct js_fsm_t
{
    struct fstack_int_t states;
};

static __always_inline int pack_2i16_i32(int lb16, int hb16)
{
    int i = lb16 & 0xFFFF;
    i |= (hb16<<16) & 0xFFFF0000;

    return i;
}

static __always_inline void unpack_2i16_i32(int i, int* lb16, int* hb16)
{
    *lb16 = i & 0xFFFF;
    *hb16 = (i & 0xFFFF0000)>>16;
}

int js_fsm_parse(struct js_fsm_t* fsm, const char* json, struct js_lexer_t* lexer)
{
    memset(fsm, 0, sizeof(struct js_fsm_t));

    int cur_state = JS_FSM_STATE_INITIAL;
    int i = 0;

    while(cur_state != JS_FSM_STATE_ERROR || cur_state != JS_FSM_STATE_EOF)
    {
        char ch = json[i];
        int ns = 0;

        switch(ch)
        {
            case '{': {
                ns = JS_FSM_STATE_BEGIN_OBJECT;
                if(js_lexer_push_token(lexer, ns, (char*)&json[i], (char*)&json[i+1]) != ST_OK)
                    return ST_ERR;

                break;
            }
            case '}': {
                ns = JS_FSM_STATE_END_OBJECT;

                break;
            }
            case ':':
                ns = JS_FSM_STATE_COLON;
                break;
            case ' ':
                ns = JS_FSM_STATE_WS;
                break;
            case '\t':
                ns = JS_FSM_STATE_WS;
                break;
            case '\n':
                ns = JS_FSM_STATE_WS;
                break;

            default: {
                if (isalnum(ch)) {
                    ns = JS_FSM_STATE_CHAR;
                } else
                {
                    ns = JS_FSM_STATE_ERROR;
                }

                break;
            }
        }

        ++i;

    }

}
