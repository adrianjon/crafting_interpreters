// This file is auto-generated. Do not edit.

#include "Expr.h"
#include "Stmt.h"
#include "Token.h"
#include "Object.h"

void* stmt_accept(const stmt_t* stmt, const stmt_visitor_t* visitor, void* context) {
	switch(stmt->type) {
		case STMT_BLOCK:
			if (visitor->visit_block)
				return visitor->visit_block(stmt, context);
		case STMT_FUNCTION:
			if (visitor->visit_function)
				return visitor->visit_function(stmt, context);
		case STMT_CLASS:
			if (visitor->visit_class)
				return visitor->visit_class(stmt, context);
		case STMT_EXPRESSION:
			if (visitor->visit_expression)
				return visitor->visit_expression(stmt, context);
		case STMT_IF:
			if (visitor->visit_if)
				return visitor->visit_if(stmt, context);
		case STMT_PRINT:
			if (visitor->visit_print)
				return visitor->visit_print(stmt, context);
		case STMT_RETURN:
			if (visitor->visit_return)
				return visitor->visit_return(stmt, context);
		case STMT_VAR:
			if (visitor->visit_var)
				return visitor->visit_var(stmt, context);
		case STMT_WHILE:
			if (visitor->visit_while)
				return visitor->visit_while(stmt, context);
		default: return NULL;
	}
}

