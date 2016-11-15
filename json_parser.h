//
// Created by Gregory Istratov on 11/15/16.
//

#ifndef LIBSTRATUM_JSON_PARSER_H
#define LIBSTRATUM_JSON_PARSER_H

#define JS_FSM_NUM_STATES 50
#define JS_LEXER_MAX_TOKENS 50

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

int js_fsm_scan(const char* json, struct js_lexer_t* lexer);
void js_debug_token(struct js_token_t *t);

#endif //LIBSTRATUM_JSON_PARSER_H
