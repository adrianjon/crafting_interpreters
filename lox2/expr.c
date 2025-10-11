// This file is auto-generated. Do not edit

#include "expr.h"
#include "stmt.h"

void * expr_accept(expr_t const * expr, expr_visitor_t const * visitor, void * context) {
	switch(expr->type) {
		case EXPR_ASSIGN:
			if (visitor->visit_assign)
				return visitor->visit_assign(expr, context);
		case EXPR_BINARY:
			if (visitor->visit_binary)
				return visitor->visit_binary(expr, context);
		case EXPR_CALL:
			if (visitor->visit_call)
				return visitor->visit_call(expr, context);
		case EXPR_GET:
			if (visitor->visit_get)
				return visitor->visit_get(expr, context);
		case EXPR_GROUPING:
			if (visitor->visit_grouping)
				return visitor->visit_grouping(expr, context);
		case EXPR_LITERAL:
			if (visitor->visit_literal)
				return visitor->visit_literal(expr, context);
		case EXPR_LOGICAL:
			if (visitor->visit_logical)
				return visitor->visit_logical(expr, context);
		case EXPR_SET:
			if (visitor->visit_set)
				return visitor->visit_set(expr, context);
		case EXPR_SUPER:
			if (visitor->visit_super)
				return visitor->visit_super(expr, context);
		case EXPR_THIS:
			if (visitor->visit_this)
				return visitor->visit_this(expr, context);
		case EXPR_UNARY:
			if (visitor->visit_unary)
				return visitor->visit_unary(expr, context);
		case EXPR_VARIABLE:
			if (visitor->visit_variable)
				return visitor->visit_variable(expr, context);
		default: return NULL;
	}
}

