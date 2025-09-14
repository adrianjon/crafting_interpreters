// This file is auto-generated. Do not edit.

#include "Expr.h"
#include "Stmt.h"
#include "Token.h"
#include "Object.h"

void* expr_accept(const expr_t* expr, const expr_visitor_t* visitor, void* context) {
	switch(expr->type) {
		case EXPR_ASSIGN:
			if (visitor->visit_assign)
				return visitor->visit_assign(expr, visitor, context);
			else
				return NULL;
		case EXPR_BINARY:
			if (visitor->visit_binary)
				return visitor->visit_binary(expr, visitor, context);
			else
				return NULL;
		case EXPR_CALL:
			if (visitor->visit_call)
				return visitor->visit_call(expr, visitor, context);
			else
				return NULL;
		case EXPR_GET:
			if (visitor->visit_get)
				return visitor->visit_get(expr, visitor, context);
			else
				return NULL;
		case EXPR_GROUPING:
			if (visitor->visit_grouping)
				return visitor->visit_grouping(expr, visitor, context);
			else
				return NULL;
		case EXPR_LITERAL:
			if (visitor->visit_literal)
				return visitor->visit_literal(expr, visitor, context);
			else
				return NULL;
		case EXPR_LOGICAL:
			if (visitor->visit_logical)
				return visitor->visit_logical(expr, visitor, context);
			else
				return NULL;
		case EXPR_SET:
			if (visitor->visit_set)
				return visitor->visit_set(expr, visitor, context);
			else
				return NULL;
		case EXPR_SUPER:
			if (visitor->visit_super)
				return visitor->visit_super(expr, visitor, context);
			else
				return NULL;
		case EXPR_THIS:
			if (visitor->visit_this)
				return visitor->visit_this(expr, visitor, context);
			else
				return NULL;
		case EXPR_UNARY:
			if (visitor->visit_unary)
				return visitor->visit_unary(expr, visitor, context);
			else
				return NULL;
		case EXPR_VARIABLE:
			if (visitor->visit_variable)
				return visitor->visit_variable(expr, visitor, context);
			else
				return NULL;
		default: return NULL;
	}
}

