// This file is auto-generated. Do not edit

#ifndef EXPR_H
#define EXPR_H

#include "token.h"

// Forward declarations
typedef struct expr expr_t;
typedef struct stmt stmt_t;
typedef struct expr_visitor expr_visitor_t;

struct expr_visitor {
	void * (*visit_assign)(expr_t const * expr, void * context);
	void * (*visit_binary)(expr_t const * expr, void * context);
	void * (*visit_call)(expr_t const * expr, void * context);
	void * (*visit_get)(expr_t const * expr, void * context);
	void * (*visit_grouping)(expr_t const * expr, void * context);
	void * (*visit_literal)(expr_t const * expr, void * context);
	void * (*visit_logical)(expr_t const * expr, void * context);
	void * (*visit_set)(expr_t const * expr, void * context);
	void * (*visit_super)(expr_t const * expr, void * context);
	void * (*visit_this)(expr_t const * expr, void * context);
	void * (*visit_unary)(expr_t const * expr, void * context);
	void * (*visit_variable)(expr_t const * expr, void * context);
};

// expr types
typedef enum {
	EXPR_ASSIGN,
	EXPR_BINARY,
	EXPR_CALL,
	EXPR_GET,
	EXPR_GROUPING,
	EXPR_LITERAL,
	EXPR_LOGICAL,
	EXPR_SET,
	EXPR_SUPER,
	EXPR_THIS,
	EXPR_UNARY,
	EXPR_VARIABLE,
} expr_type_t;

typedef struct {
	 token_t * target;
	 expr_t * value;
} expr_assign_t;

typedef struct {
	 expr_t * left;
	 token_t * operator;
	 expr_t * right;
} expr_binary_t;

typedef struct {
	 expr_t * callee;
	 token_t * paren;
	 expr_t ** arguments;
	 size_t count;
} expr_call_t;

typedef struct {
	 expr_t * object;
	 token_t * name;
} expr_get_t;

typedef struct {
	 expr_t * expression;
} expr_grouping_t;

typedef struct {
	 token_t * kind;
} expr_literal_t;

typedef struct {
	 expr_t * left;
	 token_t * operator;
	 expr_t * right;
} expr_logical_t;

typedef struct {
	 expr_t * object;
	 token_t * name;
	 expr_t * value;
} expr_set_t;

typedef struct {
	 token_t * keyword;
	 token_t * method;
} expr_super_t;

typedef struct {
	 token_t * keyword;
} expr_this_t;

typedef struct {
	 token_t * operator;
	 expr_t * right;
} expr_unary_t;

typedef struct {
	 token_t * name;
} expr_variable_t;

struct expr {
	expr_type_t type;
	union {
		expr_assign_t assign_expr;
		expr_binary_t binary_expr;
		expr_call_t call_expr;
		expr_get_t get_expr;
		expr_grouping_t grouping_expr;
		expr_literal_t literal_expr;
		expr_logical_t logical_expr;
		expr_set_t set_expr;
		expr_super_t super_expr;
		expr_this_t this_expr;
		expr_unary_t unary_expr;
		expr_variable_t variable_expr;
	} as;
};

void * expr_accept(expr_t const * expr, expr_visitor_t const * visitor, void * context);

#endif
