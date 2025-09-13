// This file is auto-generated. Do not edit.

#include "Expr.h"
#include "Stmt.h"
#include "Token.h"
#include "Object.h"

void* expr_accept(expr_t* expr, expr_visitor_t* visitor, void* context) {
	switch(expr->type) {
		case EXPR_ASSIGN:			return visitor->visit_assign(expr, visitor, context);
		case EXPR_BINARY:			return visitor->visit_binary(expr, visitor, context);
		case EXPR_CALL:			return visitor->visit_call(expr, visitor, context);
		case EXPR_GET:			return visitor->visit_get(expr, visitor, context);
		case EXPR_GROUPING:			return visitor->visit_grouping(expr, visitor, context);
		case EXPR_LITERAL:			return visitor->visit_literal(expr, visitor, context);
		case EXPR_LOGICAL:			return visitor->visit_logical(expr, visitor, context);
		case EXPR_SET:			return visitor->visit_set(expr, visitor, context);
		case EXPR_SUPER:			return visitor->visit_super(expr, visitor, context);
		case EXPR_THIS:			return visitor->visit_this(expr, visitor, context);
		case EXPR_UNARY:			return visitor->visit_unary(expr, visitor, context);
		case EXPR_VARIABLE:			return visitor->visit_variable(expr, visitor, context);
		default: return NULL;
	}
}

