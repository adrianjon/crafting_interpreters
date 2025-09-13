# TODO List

- [ ] Add support for lexemes for all token types
- [ ] Move project to its own directory
- [ ] Start building the parser

Phase 1 — Finish scanner/token updates (you’re here)

Align token types with grammar
Add/verify keyword tokens: AND, OR, TRUE, FALSE, NIL, VAR, FUN, CLASS, THIS, SUPER, FOR, IF, ELSE, WHILE, PRINT, RETURN.
Ensure punctuation/operators: LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE, COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR, BANG, BANG_EQUAL, EQUAL, EQUAL_EQUAL, GREATER, GREATER_EQUAL, LESS, LESS_EQUAL.
Keep EOF_ at end of stream.
Implement lexeme capture
For identifiers, numbers, strings, and keywords, copy the exact lexeme into a newly allocated, null‑terminated buffer.
Strings: store without surrounding quotes.
Numbers: store the original lexeme (parser will use strtod).
Decide ownership: simplest is tokens own lexeme memory (freed when you discard token array).
Update keyword map in scanner
Map lowercase keywords to their token types (and/or to identifiers when not matching).
Sanity tests for scanning
Scan short programs that include:
Identifiers, TRUE/FALSE/NIL, numbers, strings.
Operators and punctuation.
A mix of print/var/if/while/for/return.
Verify token type sequence and lexeme string values.
Confirm EOF_ is present and line numbers are correct.
Phase 2 — Add a Statement Printer visitor (for debugging) 5) Plan reuse of your expression printer

Option A (recommended): refactor AstPrinter into ExprPrinter.h/ExprPrinter.c exposing a function like char* expr_to_string(expr_t*), returning heap-allocated text.
Option B: duplicate a minimal expression printer inside the statement printer (faster now, more code to maintain later).
Implement StmtPrinter visitor
Create StmtPrinter.c that:
Implements stmt_visitor_t and a dispatcher like void stmt_print(stmt_t*).
For each stmt type:
STMT_PRINT: print “print <expr_str>;”
STMT_EXPRESSION: print “<expr_str>;”
STMT_VAR: print “var name (= initializer)?;”
STMT_IF: print “if (<cond>) <then> (else <else>)?”
STMT_WHILE: print “while (<cond>) <body>”
STMT_BLOCK: print “{ … }” with nested statements.
STMT_RETURN: print “return (value)?;”
STMT_FUNCTION/CLASS: you can stub or format minimal info until you support parsing them.
Use your Windows.h print/print_error and Arrays.h string builder if you want single-line outputs.
Tests for statement printer
Manually construct a few stmt_t trees:
var x = 1 + 2;
print “hi”;
if (x) print x; else print 0;
while (x > 0) { x = x - 1; print x; }
Ensure output is deterministic and readable.
Phase 3 — Parser scaffolding (no visitors; recursive-descent) 8) Add Parser.h/Parser.c scaffolding

parser_t state: tokens array, count, current, had_error.
Helpers: peek, previous, is_at_end, advance, check, match, consume, synchronize.
Public API: parser_init, parse_program (returns dynamic_array_t of stmt_t*), parse_expression (for REPL/tests).
Expression grammar (first milestone)
Implement recursive-descent for:
primary, call, unary, factor, term, comparison, equality, assignment.
Node constructors for expr_t types you already have (literal, grouping, unary, binary, variable, assign, call, get/logical later).
Quick expression parsing tests
Feed tokens for “1 + 2 * 3;”, “-(1 + 2);”, “a = 3 * (b + 1);”
Run expr_to_string on parse_expression output to confirm shape.
Phase 4 — Statements parsing (incremental) 11) Minimal statements

expression statement: expression “;”.
print statement: “print” expression “;”.
block: “{” declaration* “}”.
Declarations
var declaration: “var” IDENTIFIER (“=” expression)? “;”.
If you don’t want functions/classes yet, have declaration fall back to statement when not “var”.
Control flow
if: “if” “(” expression “)” statement (“else” statement)?
while: “while” “(” expression “)” statement
for (desugar): “for” “init;cond;inc” statement => { init; while (cond) { body; inc; } }
return
“return” expression? “;” (you can allow it anywhere initially; enforce function context later)
Program parsing
parse_program: loop declaration() until EOF_.
Return a dynamic_array_t containing stmt_t* (ensure you always push with element_size == sizeof(stmt_t*)).
Error handling and synchronization
On parsing error, set had_error, call synchronize to skip to next statement boundary.
Ensure parser continues to find more errors in a single pass.
Phase 5 — Integration and debugging 17) Wire scanner → parser → stmt printer

Tokenize file, parse to statements, then print each statement using your StmtPrinter to verify end-to-end.
Edge-case tests
Empty files, unmatched braces/parentheses, missing semicolons, bad assignment targets, invalid tokens, string edge cases (escapes, empty).
Phase 6 — Memory and cleanup 19) Decide lifetimes and ownership

Tokens own lexemes.
AST nodes own their substructure; literals hold heap object_t.
Optional: write free functions for expr_t/stmt_t trees and for token arrays to avoid leaks in longer sessions/tests.
Optional improvements
Add logical and/or parsing (uses EXPR_LOGICAL).
Add this/super/call chaining, get/set.
Add function/class parsing.
Refine printers for those new constructs.
Acceptance checklist before moving beyond parser

Scanner produces correct tokens with accurate lexemes for a representative sample program.
Statement printer can format a hand-built AST.
Parser can handle expressions, var/print/blocks, if/while/for/return.
Full pipeline scan→parse→print reproduces the expected structure.
Basic panic-mode error recovery works (parser reports and continues).