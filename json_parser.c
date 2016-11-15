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
    JS_FSM_STATE_ESCAPE,
    JS_FSM_STATE_CHAR,
    JS_FSM_STATE_STRING,
    JS_FSM_STATE_NUMBER,
    JS_FSM_STATE_FLOAT,
    JS_FSM_STATE_BEGIN_OBJECT,
    JS_FSM_STATE_END_OBJECT,
    JS_FSM_STATE_BEGIN_ARRAY,
    JS_FSM_STATE_END_ARRAY,
    JS_FSM_STATE_LITERAL,
    JS_FSM_STATE_QUOTE,
    JS_FSM_STATE_COLON,
    JS_FSM_STATE_DOT,
    JS_FSM_STATE_COMMA,
    JS_FSM_STATE_EOF
};

static __always_inline int js_lexer_push_token(struct js_lexer_t* lexer, int type, const char* ts, const char* te)
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
        case JS_FSM_STATE_BEGIN_ARRAY:
            return "BEGIN_ARRAY";
        case JS_FSM_STATE_END_ARRAY:
            return "END_ARRAY";
        case JS_FSM_STATE_COLON:
            return "COLON";
        case JS_FSM_STATE_COMMA:
            return "COMMA";
        case JS_FSM_STATE_NUMBER:
            return "INTEGER";
        case JS_FSM_STATE_FLOAT:
            return "FLOAT";
        case JS_FSM_STATE_LITERAL:
            return "LITERAL";
        default:
            return "UNKNOWN";
    }
}

void js_debug_token(struct js_token_t *t)
{
    size_t slen = (size_t)(t->te - t->ts);
    LOG_DEBUG("\nTOKEN_TYPE: %s\nTOKEN_STRING: %.*s\n", js_token_type_s(t->type), slen, t->ts);
}

struct js_fsm_t
{
    const char* json;
    int state;
    int i;
    const char* ts;
    const char* te;
    struct js_lexer_t* lex;
};

typedef int(*js_fsm_trans_callback_t)(struct js_fsm_t*);

#define FSM_CALLBACK(x) ((size_t)&x)

static __always_inline int js_fsm_push_single_token(struct js_fsm_t* fsm, int token_type)
{
    if (js_lexer_push_token(fsm->lex, token_type, &fsm->json[fsm->i], &fsm->json[fsm->i+1]) == ST_OK)
        return ST_OK;

    LOG_ERROR("An error occurred pos=%d", fsm->i);

    return ST_ERR;
}

static int js_fsm_comma(struct js_fsm_t* fsm)
{
    return js_fsm_push_single_token(fsm, JS_FSM_STATE_COMMA);
}

static int js_fsm_colon(struct js_fsm_t* fsm)
{
    return js_fsm_push_single_token(fsm, JS_FSM_STATE_COLON);
}

static int js_fsm_begin_object(struct js_fsm_t* fsm)
{
    return js_fsm_push_single_token(fsm, JS_FSM_STATE_BEGIN_OBJECT);
}

static int js_fsm_end_object(struct js_fsm_t* fsm)
{
    return js_fsm_push_single_token(fsm, JS_FSM_STATE_END_OBJECT);
}

static int js_fsm_begin_array(struct js_fsm_t* fsm)
{
    return js_fsm_push_single_token(fsm, JS_FSM_STATE_BEGIN_ARRAY);
}

static int js_fsm_end_array(struct js_fsm_t* fsm)
{
    return js_fsm_push_single_token(fsm, JS_FSM_STATE_END_ARRAY);
}

static int js_fsm_literal_start(struct js_fsm_t* fsm)
{
    fsm->ts = &fsm->json[fsm->i];

    return ST_OK;
}

static int js_fsm_literal_end(struct js_fsm_t* fsm)
{
    fsm->te = &fsm->json[fsm->i];

    if(fsm->ts && fsm->te) {
        if (js_lexer_push_token(fsm->lex, JS_FSM_STATE_LITERAL, fsm->ts, fsm->te) == ST_OK)
        {
            fsm->ts = NULL;
            fsm->te = NULL;
            return ST_OK;
        }
    }

    LOG_ERROR("An error occurred pos=%d", fsm->i);

    return ST_ERR;
}

static int js_fsm_number_start(struct js_fsm_t* fsm)
{
    fsm->ts = &fsm->json[fsm->i];

    return ST_OK;
}

static int js_fsm_number_end(struct js_fsm_t* fsm)
{
    fsm->te = &fsm->json[fsm->i];

    if(fsm->ts && fsm->te) {
        if (js_lexer_push_token(fsm->lex, JS_FSM_STATE_NUMBER, fsm->ts, fsm->te) == ST_OK)
        {
            fsm->ts = NULL;
            fsm->te = NULL;
            return ST_OK;
        }
    }

    LOG_ERROR("An error occurred pos=%d", fsm->i);

    return ST_ERR;
}

static int js_fsm_float_end(struct js_fsm_t* fsm)
{
    fsm->te = &fsm->json[fsm->i];

    if(fsm->ts && fsm->te) {
        if (js_lexer_push_token(fsm->lex, JS_FSM_STATE_FLOAT, fsm->ts, fsm->te) == ST_OK)
        {
            fsm->ts = NULL;
            fsm->te = NULL;
            return ST_OK;
        }
    }

    LOG_ERROR("An error occurred pos=%d", fsm->i);

    return ST_ERR;
}

static int js_fsm_string_start(struct js_fsm_t* fsm)
{
    fsm->ts = &fsm->json[fsm->i+1];

    return ST_OK;
}

static int js_fsm_string_end(struct js_fsm_t* fsm)
{
    fsm->te = &fsm->json[fsm->i];

    if(fsm->ts && fsm->te) {
        if (js_lexer_push_token(fsm->lex, JS_FSM_STATE_STRING, fsm->ts, fsm->te) == ST_OK)
        {
            fsm->ts = NULL;
            fsm->te = NULL;
            return ST_OK;
        }
    }

    LOG_ERROR("An error occurred pos=%d", fsm->i);

    return ST_ERR;
}

static const size_t js_fsm_translation_table[JS_FSM_NUM_STATES][4] =
{
{JS_FSM_STATE_INITIAL,   JS_FSM_STATE_BEGIN_OBJECT,      JS_FSM_STATE_INITIAL,       0},
{JS_FSM_STATE_INITIAL,   JS_FSM_STATE_END_OBJECT,        JS_FSM_STATE_INITIAL,       0},
{JS_FSM_STATE_INITIAL,   JS_FSM_STATE_BEGIN_ARRAY,       JS_FSM_STATE_INITIAL,       0},
{JS_FSM_STATE_INITIAL,   JS_FSM_STATE_END_ARRAY,         JS_FSM_STATE_INITIAL,       0},
{JS_FSM_STATE_INITIAL,   JS_FSM_STATE_COLON,             JS_FSM_STATE_INITIAL,       0},
{JS_FSM_STATE_INITIAL,   JS_FSM_STATE_COMMA,             JS_FSM_STATE_INITIAL,       0},
{JS_FSM_STATE_INITIAL,   JS_FSM_STATE_QUOTE,             JS_FSM_STATE_STRING,        FSM_CALLBACK(js_fsm_string_start)},
{JS_FSM_STATE_STRING,    JS_FSM_STATE_QUOTE,             JS_FSM_STATE_INITIAL,       FSM_CALLBACK(js_fsm_string_end)},
{JS_FSM_STATE_INITIAL,   JS_FSM_STATE_NUMBER,            JS_FSM_STATE_NUMBER,        FSM_CALLBACK(js_fsm_number_start)},
{JS_FSM_STATE_NUMBER,    JS_FSM_STATE_NUMBER,            JS_FSM_STATE_NUMBER,        0},
{JS_FSM_STATE_NUMBER,    JS_FSM_STATE_COMMA,             JS_FSM_STATE_INITIAL,       FSM_CALLBACK(js_fsm_number_end)},
{JS_FSM_STATE_NUMBER,    JS_FSM_STATE_END_OBJECT,        JS_FSM_STATE_INITIAL,       FSM_CALLBACK(js_fsm_number_end)},
{JS_FSM_STATE_NUMBER,    JS_FSM_STATE_END_ARRAY,         JS_FSM_STATE_INITIAL,       FSM_CALLBACK(js_fsm_number_end)},
{JS_FSM_STATE_NUMBER,    JS_FSM_STATE_DOT,               JS_FSM_STATE_FLOAT,         0},
{JS_FSM_STATE_FLOAT,     JS_FSM_STATE_NUMBER,            JS_FSM_STATE_FLOAT,         0},
{JS_FSM_STATE_FLOAT,     JS_FSM_STATE_COMMA,             JS_FSM_STATE_INITIAL,       FSM_CALLBACK(js_fsm_float_end)},
{JS_FSM_STATE_FLOAT,     JS_FSM_STATE_END_OBJECT,        JS_FSM_STATE_INITIAL,       FSM_CALLBACK(js_fsm_float_end)},
{JS_FSM_STATE_FLOAT,     JS_FSM_STATE_END_ARRAY,         JS_FSM_STATE_INITIAL,       FSM_CALLBACK(js_fsm_float_end)},
{JS_FSM_STATE_INITIAL,   JS_FSM_STATE_CHAR,              JS_FSM_STATE_LITERAL,       FSM_CALLBACK(js_fsm_literal_start)},
{JS_FSM_STATE_LITERAL,   JS_FSM_STATE_CHAR,              JS_FSM_STATE_LITERAL,       0},
{JS_FSM_STATE_LITERAL,   JS_FSM_STATE_END_OBJECT,        JS_FSM_STATE_INITIAL,       FSM_CALLBACK(js_fsm_literal_end)},
{JS_FSM_STATE_LITERAL,   JS_FSM_STATE_COMMA,             JS_FSM_STATE_INITIAL,       FSM_CALLBACK(js_fsm_literal_end)},
{JS_FSM_STATE_LITERAL,   JS_FSM_STATE_END_ARRAY,         JS_FSM_STATE_INITIAL,       FSM_CALLBACK(js_fsm_literal_end)},
};

static const size_t js_fsm_single_state_callback[JS_FSM_NUM_STATES][2] =
{
{JS_FSM_STATE_ERROR,                0},
{JS_FSM_STATE_INITIAL,              0},
{JS_FSM_STATE_ESCAPE,               0},
{JS_FSM_STATE_CHAR,                 0},
{JS_FSM_STATE_STRING,               0},
{JS_FSM_STATE_NUMBER,               0},
{JS_FSM_STATE_FLOAT,                0},
{JS_FSM_STATE_BEGIN_OBJECT,         FSM_CALLBACK(js_fsm_begin_object)},
{JS_FSM_STATE_END_OBJECT,           FSM_CALLBACK(js_fsm_end_object)},
{JS_FSM_STATE_BEGIN_ARRAY,          FSM_CALLBACK(js_fsm_begin_array)},
{JS_FSM_STATE_END_ARRAY,            FSM_CALLBACK(js_fsm_end_array)},
{JS_FSM_STATE_LITERAL,              0},
{JS_FSM_STATE_QUOTE,                0},
{JS_FSM_STATE_COLON,                FSM_CALLBACK(js_fsm_colon)},
{JS_FSM_STATE_DOT,                  0},
{JS_FSM_STATE_COMMA,                FSM_CALLBACK(js_fsm_comma)},
{JS_FSM_STATE_EOF,                  0}
};

static __always_inline js_fsm_trans_callback_t js_fsm_find_scallback(int state)
{
    for(size_t j = 0; j < JS_FSM_NUM_STATES; ++j)
    {
        if((int)js_fsm_single_state_callback[j][0] == state)
            return (js_fsm_trans_callback_t)js_fsm_single_state_callback[j][1];
    }

    return (js_fsm_trans_callback_t)NULL;
}


static __always_inline int js_fsm_translate(struct js_fsm_t* fsm, int next_state)
{
    for(size_t j = 0; j < JS_FSM_NUM_STATES; ++j)
    {
        if((int)js_fsm_translation_table[j][0] == fsm->state && (int)js_fsm_translation_table[j][1] == next_state) {

            js_fsm_trans_callback_t cb = (js_fsm_trans_callback_t)js_fsm_translation_table[j][3];

            if(cb && cb(fsm) != ST_OK)
                return JS_FSM_STATE_ERROR;


            js_fsm_trans_callback_t scb = js_fsm_find_scallback(next_state);

            if(scb && scb(fsm) != ST_OK)
                return JS_FSM_STATE_ERROR;

            return (int)js_fsm_translation_table[j][2];
        }
    }

    LOG_ERROR("Can't translate state %d to %d", fsm->state, next_state);

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

    struct js_fsm_t fsm;
    memset(&fsm, 0, sizeof(struct js_fsm_t));

    fsm.json = json;
    fsm.state = JS_FSM_STATE_INITIAL;
    fsm.lex = lexer;

    while(1)
    {
        char ch = json[fsm.i];
        int ns = 0;

        // take any symbols inside a string except comma
        if(fsm.state == JS_FSM_STATE_STRING && ch != '\0')
        {
            if(ch == '"')
                goto STATE_QUOTE;

            ++fsm.i;
            continue;
        }

        switch(ch)
        {
            case '{':
                ns = JS_FSM_STATE_BEGIN_OBJECT;
                break;
            case '}':
                ns = JS_FSM_STATE_END_OBJECT;
                break;
            case '[':
                ns = JS_FSM_STATE_BEGIN_ARRAY;
                break;
            case ']':
                ns = JS_FSM_STATE_END_ARRAY;
                break;
            case ':':
                ns = JS_FSM_STATE_COLON;
                break;
            case ',':
                ns = JS_FSM_STATE_COMMA;
                break;
            case '.':
                ns = JS_FSM_STATE_DOT;
                break;
            case '"': {
                STATE_QUOTE:
                ns = JS_FSM_STATE_QUOTE;
                break;
            }
            case '\0':
                ns = JS_FSM_STATE_EOF;
                break;

            default: {
                if(isspace(ch))
                {
                    goto INCREMENT;
                }
                else if(isdigit(ch))
                {
                    ns = JS_FSM_STATE_NUMBER;
                }
                else if (isalpha(ch)) {
                    ns = JS_FSM_STATE_CHAR;
                } else
                {
                    ns = JS_FSM_STATE_ERROR;
                }

                break;
            }
        }

        if(ns == JS_FSM_STATE_ERROR)
        {
            LOG_ERROR("Can't understand input symbol '%c' at %d position", ch, fsm.i);
            return ST_ERR;
        }

        if(ns == JS_FSM_STATE_EOF)
            break;


        if((fsm.state = js_fsm_translate(&fsm, ns)) == JS_FSM_STATE_ERROR)
            return ST_ERR;

        INCREMENT:
        ++fsm.i;

    }

    return ST_OK;

}

