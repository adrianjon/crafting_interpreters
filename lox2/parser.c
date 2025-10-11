//
// Created by adrian on 2025-10-11.
//

#include "parser.h"

#include "stmt.h"

static stmt_t * parse_statement(parser_t * p_parser);

list_t parse(parser_t * p_parser) {
    p_parser->current_p = p_parser->tokens.data[0];
    list_t statements = {0};

    while (p_parser->current_p->type != END_OF_FILE ) {
        stmt_t * p_stmt = parse_statement(p_parser);
        list_add(&statements, p_stmt);
    }
    return statements;
}

static stmt_t * parse_statement(parser_t * p_parser) {
    return NULL;
}