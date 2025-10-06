// This file is auto-generated. Do not edit.

#ifndef EXPR_H
#define EXPR_H

#include "../extra/Arrays.h"
#include "Expr.h"
//#include "Stmt.h"
#include "Token.h"

#include "Object.h"

// Forward declarations
typedef struct expr expr_t;
typedef struct expr_visitor expr_visitor_t;
struct expr_visitor {
		void* (*visit_assign)(const expr_t* expr, void * context);
		void* (*visit_binary)(const expr_t* expr, void * context);
		void* (*visit_call)(const expr_t* expr, void * context);
		void* (*visit_get)(const expr_t* expr, void * context);
		void* (*visit_grouping)(const expr_t* expr, void * context);
		void* (*visit_literal)(const expr_t* expr, void * context);
		void* (*visit_logical)(const expr_t* expr, void * context);
		void* (*visit_set)(const expr_t* expr, void * context);
		void* (*visit_super)(const expr_t* expr, void * context);
		void* (*visit_this)(const expr_t* expr, void * context);
		void* (*visit_unary)(const expr_t* expr, void * context);
		void* (*visit_variable)(const expr_t* expr, void * context);
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
	EXPR_VARIABLE
} expr_type_t;

typedef struct {
	expr_t * target;
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
	size_t * count;
	
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

void* expr_accept(const expr_t* expr, const expr_visitor_t* visitor, void* context);

#endif // EXPR_H
