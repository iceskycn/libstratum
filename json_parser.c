//
// Created by Gregory Istratov on 11/15/16.
//

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "stack.h"
#include "log.h"
#include "json_parser.h"

enum
{
    JS_FSM_STATE_ERROR = 0,
    JS_FSM_STATE_INITIAL,
    JS_FSM_STATE_STRING,
    JS_FSM_STATE_NUMBER,
    JS_FSM_STATE_INT,
    JS_FSM_STATE_FLOAT,
    JS_FSM_STATE_BEGIN_OBJECT,
    JS_FSM_STATE_END_OBJECT,
    JS_FSM_STATE_QUOTE,
    JS_FSM_STATE_COLON,
    JS_FSM_STATE_DOT,
    JS_FSM_STATE_WS,
    JS_FSM_STATE_COMMA,
    JS_FSM_STATE_EOF
};

static __always_inline int js_lexer_push_token(struct js_lexer_t* lexer, int type, char* ts, char* te)
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

const char* js_token_type_s(int t)
{
    switch(t)
    {
        case JS_FSM_STATE_STRING:
            return "STRING";
        case JS_FSM_STATE_BEGIN_OBJECT:
            return "BEGIN_OBJECT";
        case JS_FSM_STATE_END_OBJECT:
            return "END_OBJECT";
        case JS_FSM_STATE_COLON:
            return "COLON";
        case JS_FSM_STATE_COMMA:
            return "COMMA";
        default:
            return "UNKNOWN";
    }
}

void js_debug_token(struct js_token_t *t)
{
    char buf[1024];
    memset(buf, 0, 1024);

    size_t slen = (size_t)(t->te - t->ts);
    int res = snprintf(buf, 1024, "\nTOKEN_TYPE: %s\nTOKEN_STRING: %.*s\n", js_token_type_s(t->type), slen, t->ts);
    if(res < 0)
    {
        LOG_ERROR("can't snprinf");
        return;
    }

    LOG_DEBUG(buf);
}

static const int js_fsm_translation_table[JS_FSM_NUM_STATES][3] =
{
        {JS_FSM_STATE_INITIAL, JS_FSM_STATE_WS, JS_FSM_STATE_INITIAL},
        {JS_FSM_STATE_INITIAL, JS_FSM_STATE_BEGIN_OBJECT, JS_FSM_STATE_INITIAL},
        {JS_FSM_STATE_INITIAL, JS_FSM_STATE_END_OBJECT, JS_FSM_STATE_INITIAL},
        {JS_FSM_STATE_INITIAL, JS_FSM_STATE_COLON, JS_FSM_STATE_INITIAL},
        {JS_FSM_STATE_INITIAL, JS_FSM_STATE_COMMA, JS_FSM_STATE_INITIAL},
        {JS_FSM_STATE_INITIAL, JS_FSM_STATE_QUOTE, JS_FSM_STATE_STRING},
        {JS_FSM_STATE_STRING, JS_FSM_STATE_STRING, JS_FSM_STATE_STRING},
        {JS_FSM_STATE_STRING, JS_FSM_STATE_QUOTE, JS_FSM_STATE_INITIAL},
};

static __always_inline int js_fsm_translate(int cur_state, int next_state)
{
    for(int j = 0; j < JS_FSM_NUM_STATES; ++j)
    {
        if(js_fsm_translation_table[j][0] == cur_state && js_fsm_translation_table[j][1] == next_state) {
            return js_fsm_translation_table[j][2];
        }
    }

    LOG_ERROR("Can't translate");

    return JS_FSM_STATE_ERROR;
}

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

int js_fsm_scan(const char* json, struct js_lexer_t* lexer)
{
    memset(lexer, 0, sizeof(struct js_lexer_t));

    int cur_state = JS_FSM_STATE_INITIAL;
    int i = 0;
    char* ts = NULL;
    char* te = NULL;

    while(1)
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
                if(js_lexer_push_token(lexer, ns, (char*)&json[i], (char*)&json[i+1]) != ST_OK)
                    return ST_ERR;

                break;
            }
            case ':':
                ns = JS_FSM_STATE_COLON;
                if(js_lexer_push_token(lexer, ns, (char*)&json[i], (char*)&json[i+1]) != ST_OK)
                    return ST_ERR;
                break;
            case ',':
                ns = JS_FSM_STATE_COMMA;
                if(js_lexer_push_token(lexer, ns, (char*)&json[i], (char*)&json[i+1]) != ST_OK)
                    return ST_ERR;
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
            case '"': {
                ns = JS_FSM_STATE_QUOTE;

                if(!ts) ts = (char*)&json[i+1];
                else te = (char*)&json[i];

                if(ts && te) {
                    if (js_lexer_push_token(lexer, JS_FSM_STATE_STRING, ts, te) != ST_OK)
                        return ST_ERR;

                    ts = NULL;
                    te = NULL;
                }

                break;
            }
            case '\0':
                ns = JS_FSM_STATE_EOF;
                break;

            default: {
                if (isalnum(ch)) {
                    ns = JS_FSM_STATE_STRING;
                } else
                {
                    ns = JS_FSM_STATE_ERROR;
                }

                break;
            }
        }

        if(ns == JS_FSM_STATE_ERROR)
        {
            LOG_ERROR("Can't understand input symbol");
            return ST_ERR;
        }

        if(ns == JS_FSM_STATE_EOF)
            break;


        if((cur_state = js_fsm_translate(cur_state, ns)) == JS_FSM_STATE_ERROR)
            return ST_ERR;

        ++i;

    }

    return ST_OK;

}
