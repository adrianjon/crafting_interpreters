// This file is auto-generated. Do not edit

#ifndef STMT_H
#define STMT_H

#include "token.h"

// Forward declarations
typedef struct expr expr_t;
typedef struct stmt stmt_t;
typedef struct stmt_visitor stmt_visitor_t;

struct stmt_visitor {
	void * (*visit_block)(stmt_t const * stmt, void * context);
	void * (*visit_function)(stmt_t const * stmt, void * context);
	void * (*visit_class)(stmt_t const * stmt, void * context);
	void * (*visit_expression)(stmt_t const * stmt, void * context);
	void * (*visit_if)(stmt_t const * stmt, void * context);
	void * (*visit_print)(stmt_t const * stmt, void * context);
	void * (*visit_return)(stmt_t const * stmt, void * context);
	void * (*visit_var)(stmt_t const * stmt, void * context);
	void * (*visit_while)(stmt_t const * stmt, void * context);
};

// stmt types
typedef enum {
	STMT_BLOCK,
	STMT_FUNCTION,
	STMT_CLASS,
	STMT_EXPRESSION,
	STMT_IF,
	STMT_PRINT,
	STMT_RETURN,
	STMT_VAR,
	STMT_WHILE,
} stmt_type_t;

typedef struct {
	 stmt_t ** statements;
	 size_t count;
} stmt_block_t;

typedef struct {
	 token_t * name;
	 token_t ** params;
	 size_t params_count;
	 stmt_t ** body;
	 size_t count;
} stmt_function_t;

typedef struct {
	 token_t * name;
	 expr_t ** superclass;
	 size_t superclass_count;
	 stmt_t ** methods;
	 size_t methods_count;
} stmt_class_t;

typedef struct {
	 expr_t * expression;
} stmt_expression_t;

typedef struct {
	 expr_t * condition;
	 stmt_t * then_branch;
	 stmt_t * else_branch;
} stmt_if_t;

typedef struct {
	 expr_t * expression;
} stmt_print_t;

typedef struct {
	 token_t * keyword;
	 expr_t * value;
} stmt_return_t;

typedef struct {
	 token_t * name;
	 expr_t * initializer;
} stmt_var_t;

typedef struct {
	 expr_t * condition;
	 stmt_t * body;
} stmt_while_t;

struct stmt {
	stmt_type_t type;
	union {
		stmt_block_t block_stmt;
		stmt_function_t function_stmt;
		stmt_class_t class_stmt;
		stmt_expression_t expression_stmt;
		stmt_if_t if_stmt;
		stmt_print_t print_stmt;
		stmt_return_t return_stmt;
		stmt_var_t var_stmt;
		stmt_while_t while_stmt;
	} as;
};

void * stmt_accept(stmt_t const * stmt, stmt_visitor_t const * visitor, void * context);

#endif
