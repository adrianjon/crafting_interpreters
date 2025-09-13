// This file is auto-generated. Do not edit.

#include "Expr.h"
#include "Stmt.h"
#include "Token.h"
#include "Object.h"

void* stmt_accept(stmt_t* stmt, stmt_visitor_t* visitor, void* context) {
	switch(stmt->type) {
		case STMT_BLOCK:			return visitor->visit_block(stmt, visitor, context);
		case STMT_FUNCTION:			return visitor->visit_function(stmt, visitor, context);
		case STMT_CLASS:			return visitor->visit_class(stmt, visitor, context);
		case STMT_EXPRESSION:			return visitor->visit_expression(stmt, visitor, context);
		case STMT_IF:			return visitor->visit_if(stmt, visitor, context);
		case STMT_PRINT:			return visitor->visit_print(stmt, visitor, context);
		case STMT_RETURN:			return visitor->visit_return(stmt, visitor, context);
		case STMT_VAR:			return visitor->visit_var(stmt, visitor, context);
		case STMT_WHILE:			return visitor->visit_while(stmt, visitor, context);
		default: return NULL;
	}
}

