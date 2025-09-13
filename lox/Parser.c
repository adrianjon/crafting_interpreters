


// // typedef struct {
// //     enum { BINARY, GROUPING, LITERAL, UNARY } type;
// //     union {
// //         struct {
// //             struct Expr* left;
// //             char operator;
// //             struct Expr* right;
// //         } binary;
// //         struct Expr* grouping;
// //         union {
// //             int number;
// //             char* string;
// //             int boolean;
// //             void* null;
// //         } literal;
// //         struct {
// //             char operator;
// //             struct Expr* right;
// //         } unary;
// //     };
// // } Expr;

// // typedef enum {
// //     OP_ADD,
// //     OP_SUB,
// //     OP_MUL,
// //     OP_DIV
// // } Operator;

// // typedef struct {
// //     struct Expr* left;
// //     Operator operator;
// //     struct Expr* right;
// // } BinaryExpr;
// // void interpret_binary_expr(BinaryExpr* expr) {
// //     interpret_expr(expr->left);
// //     interpret_expr(expr->right);
// //     // Here you would typically perform the operation based on expr->operator
// // }
// // void interpret_expr(Expr* expr) {
// //     switch (expr->type) {
// //         case BINARY:
// //             interpret_binary_expr(&expr->binary);
// //             break;
// //         case GROUPING:
// //             interpret_expr(expr->grouping);
// //             break;
// //         case LITERAL:
// //             interpret_literal_expr(&expr->literal);
// //             break;
// //         case UNARY:
// //             interpret_unary_expr(&expr->unary);
// //             break;
// //     }
// // }

// #ifndef NULL
// #define NULL ((void*)0)
// #endif

// // forward declaration
// struct Pastry;
// struct PastryVisitor;
// struct Beignet;
// struct Cruller;

// // Base pastry struct
// typedef struct Pastry {
//     void (*accept)(struct Pastry* self, struct PastryVisitor* visitor);
// } Pastry;

// typedef struct {
//     Pastry base;
//     char filling[32];
// } Beignet;

// typedef struct {
//     Pastry base;
//     char icing[32];
// } Cruller;

// typedef struct PastryVisitor {
//     void (*visit_beignet)(struct PastryVisitor* self, Beignet* beignet);
//     void (*visit_cruller)(struct PastryVisitor* self, Cruller* cruller);
// } PastryVisitor;

// void beignet_accept(Pastry* pastry, PastryVisitor* visitor) {
//     visitor->visit_beignet(visitor, (Beignet*)pastry);
// }

// void cruller_accept(Pastry* pastry, PastryVisitor* visitor) {
//     visitor->visit_cruller(visitor, (Cruller*)pastry);
// }

// void print_beignet(PastryVisitor* visitor, Beignet* beignet) {
//     print("Beignet filling: ");
//     print(beignet->filling);
//     print("\n");
// }

// void print_cruller(PastryVisitor* visitor, Cruller* cruller) {
//     print("Cruller icing: ");
//     print(cruller->icing);
//     print("\n");
// }

// void test_visitor_pattern(void) {
//     Beignet beignet = { .base.accept = beignet_accept, .filling = "Cream" };
//     Cruller cruller = { .base.accept = cruller_accept, .icing = "Sugar" };
    
//     PastryVisitor printer = { 
//         .visit_beignet = print_beignet, 
//         .visit_cruller = print_cruller 
//     };

//     beignet.base.accept((Pastry*)&beignet, &printer);
//     cruller.base.accept((Pastry*)&cruller, &printer);

// }

// // real

// static const char* ast_expr_grammar[] = {
// //> Statements and State assign-expr
//       "Assign   : Token name, Expr value",
// //< Statements and State assign-expr
//       "Binary   : Expr left, Token operator, Expr right",
// //> Functions call-expr
//       "Call     : Expr callee, Token paren, List<Expr> arguments",
// //< Functions call-expr
// //> Classes get-ast
//       "Get      : Expr object, Token name",
// //< Classes get-ast
//       "Grouping : Expr expression",
//       "Literal  : Object value",
// //> Control Flow logical-ast
//       "Logical  : Expr left, Token operator, Expr right",
// //< Control Flow logical-ast
// //> Classes set-ast
//       "Set      : Expr object, Token name, Expr value",
// //< Classes set-ast
// //> Inheritance super-expr
//       "Super    : Token keyword, Token method",
// //< Inheritance super-expr
// //> Classes this-ast
//       "This     : Token keyword",
// //< Classes this-ast
// /* Representing Code call-define-ast < Statements and State var-expr
//       "Unary    : Token operator, Expr right"
// */
// //> Statements and State var-expr
//       "Unary    : Token operator, Expr right",
//       "Variable : Token name",
// //< Statements and State var-expr
// };

// static const char* ast_stmt_grammar[] = {
//     //> block-ast
//       "Block      : List<Stmt> statements",
// //< block-ast
// /* Classes class-ast < Inheritance superclass-ast
//       "Class      : Token name, List<Stmt.Function> methods",
// */
// //> Inheritance superclass-ast
//       "Class      : Token name, Expr.Variable superclass, List<Stmt.Function> methods",
// //< Inheritance superclass-ast
//       "Expression : Expr expression",
// //> Functions function-ast
//       "Function   : Token name, List<Token> params, List<Stmt> body",
// //< Functions function-ast
// //> Control Flow if-ast
//       "If         : Expr condition, Stmt thenBranch, Stmt elseBranch",
// //< Control Flow if-ast
// /* Statements and State stmt-ast < Statements and State var-stmt-ast
//       "Print      : Expr expression"
// */
// //> var-stmt-ast
//       "Print      : Expr expression",
// //< var-stmt-ast
// //> Functions return-ast
//       "Return     : Token keyword, Expr value",
// //< Functions return-ast
// /* Statements and State var-stmt-ast < Control Flow while-ast
//       "Var        : Token name, Expr initializer"
// */
// //> Control Flow while-ast
//       "Var        : Token name, Expr initializer",
//       "While      : Expr condition, Stmt body"
// //< Control Flow while-ast
// };