//
// Created by Gregory Istratov on 11/15/16.
//

#ifndef LIBSTRATUM_JSON_PARSER_H
#define LIBSTRATUM_JSON_PARSER_H

#define JS_FSM_NUM_STATES 30
#define JS_LEXER_MAX_TOKENS 50
#define JS_AST_OBJECT_MAX_ELEMENTS 10
#define JS_AST_ARRAY_MAX_ELEMENTS 20

#define CHECK_RETURN_ERR(x) if(x != ST_OK) return ST_ERR;

struct js_token_t
{
    int type;
    const char* ts;
    const char* te;
};

struct js_lexer_t
{
    struct js_token_t tokens[JS_LEXER_MAX_TOKENS];
    int p;
    int g;
};

int js_fsm_scan(const char* json, struct js_lexer_t* lexer);
void js_debug_token(struct js_token_t *t);

#endif //LIBSTRATUM_JSON_PARSER_H
