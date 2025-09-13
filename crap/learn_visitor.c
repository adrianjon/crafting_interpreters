#ifndef AST_H
#define AST_H

#include <stdlib.h>

// Forward declarations
typedef struct Expr Expr;
typedef struct Stmt Stmt;
typedef struct Token Token;

// Visitor pattern for Expr
typedef struct ExprVisitor {
    void* (*visitAssign)(Expr* expr, void* context);
    void* (*visitBinary)(Expr* expr, void* context);
    void* (*visitCall)(Expr* expr, void* context);
    void* (*visitGet)(Expr* expr, void* context);
    void* (*visitGrouping)(Expr* expr, void* context);
    void* (*visitLiteral)(Expr* expr, void* context);
    void* (*visitLogical)(Expr* expr, void* context);
    void* (*visitSet)(Expr* expr, void* context);
    void* (*visitSuper)(Expr* expr, void* context);
    void* (*visitThis)(Expr* expr, void* context);
    void* (*visitUnary)(Expr* expr, void* context);
    void* (*visitVariable)(Expr* expr, void* context);
} ExprVisitor;

// Expr types
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
} ExprType;

// Example Token struct (expand as needed)
struct Token {
    char* lexeme;
    int type;
};

// AST node definitions
typedef struct {
    Token* name;
    Expr* value;
} ExprAssign;

typedef struct {
    Expr* left;
    Token* operator;
    Expr* right;
} ExprBinary;

typedef struct {
    Expr* callee;
    Token* paren;
    Expr** arguments;
    size_t arg_count;
} ExprCall;

typedef struct {
    Expr* object;
    Token* name;
} ExprGet;

typedef struct {
    Expr* expression;
} ExprGrouping;

typedef struct {
    void* value;
} ExprLiteral;

typedef struct {
    Expr* left;
    Token* operator;
    Expr* right;
} ExprLogical;

typedef struct {
    Expr* object;
    Token* name;
    Expr* value;
} ExprSet;

typedef struct {
    Token* keyword;
    Token* method;
} ExprSuper;

typedef struct {
    Token* keyword;
} ExprThis;

typedef struct {
    Token* operator;
    Expr* right;
} ExprUnary;

typedef struct {
    Token* name;
} ExprVariable;

// Discriminated union for Expr
struct Expr {
    ExprType type;
    union {
        ExprAssign assign;
        ExprBinary binary;
        ExprCall call;
        ExprGet get;
        ExprGrouping grouping;
        ExprLiteral literal;
        ExprLogical logical;
        ExprSet set;
        ExprSuper super_;
        ExprThis this_;
        ExprUnary unary;
        ExprVariable variable;
    } as;
};

// Accept function for Expr
void* expr_accept(Expr* expr, ExprVisitor* visitor, void* context);


int main(void){
    return 0;
}
#endif // AST_H