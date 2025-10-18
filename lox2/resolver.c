//
// Created by adrian on 2025-10-11.
//

#include "resolver.h"

#include "stmt.h"
#include "expr.h"

#include "utils/stack.h"

#undef NULL
#define NULL nullptr

static void resolve_statement(resolver_t * p_resolver, stmt_t const * p_stmt);
static void resolve_expression(resolver_t * p_resolver, expr_t * p_expr);
static void begin_scope(resolver_t const * p_resolver);
static void end_scope(resolver_t const * p_resolver);
static bool * new_bool(bool v);
static void declare(resolver_t const * p_resolver, char const * p_name);
static void define(resolver_t const * p_resolver, char const * p_name);
static int resolve_local(resolver_t const * p_resolver, expr_t * p_expr, char const * p_name);
/*
 * Expects list_t of type List<stmt_t*>
 */
void resolve(resolver_t * p_resolver, list_t * p_statements) {
    if (!p_resolver || !p_statements) return;
    if (!p_resolver->scopes || p_resolver->scopes->capacity == 0) p_resolver->scopes =  stack_create(4);
    for (size_t i = 0; i < p_statements->count; i++) {
        resolve_statement(p_resolver, p_statements->data[i]);
        // TODO error handling
    }
}

void free_resolver(resolver_t * p_resolver) {
    if (!p_resolver) return;
    stack_destroy(p_resolver->scopes);
}
static void resolve_statement(resolver_t * p_resolver, stmt_t const * p_stmt) {
    switch (p_stmt->type) {
        case STMT_BLOCK:
            begin_scope(p_resolver);
            list_t statements = {
                .data = (void**)p_stmt->as.block_stmt.statements,
                .count = p_stmt->as.block_stmt.count,
                .capacity = p_stmt->as.block_stmt.count,
                .free_fn = NULL // no need to free, points to existing statements
            };
            resolve(p_resolver, &statements);
            end_scope(p_resolver);
            break;
        case STMT_FUNCTION:
            declare(p_resolver, p_stmt->as.function_stmt.name->lexeme);
            define(p_resolver, p_stmt->as.function_stmt.name->lexeme);

            //resolve_function(p_resolver, &p_stmt->as.function_stmt, FUNCTION_TYPE_FUNCTION);

            function_type_t const enclosing = p_resolver->current_function;
            p_resolver->current_function = FUNCTION_TYPE_FUNCTION;
            begin_scope(p_resolver);

            for (size_t i = 0; i < p_stmt->as.function_stmt.params_count; i++) {
                token_t const * param = p_stmt->as.function_stmt.params[i];
                declare(p_resolver, param->lexeme);
                define(p_resolver, param->lexeme);
            }
            list_t function_body = {
                .data = (void**)p_stmt->as.function_stmt.body,
                .count = p_stmt->as.function_stmt.count,
                .capacity = p_stmt->as.function_stmt.count,
                .free_fn = NULL // no need to free, points to existing statements
            };
            resolve(p_resolver, &function_body);
            end_scope(p_resolver);
            p_resolver->current_function = enclosing;
            break;
        case STMT_CLASS:
            stmt_class_t const * s = &p_stmt->as.class_stmt;
            declare(p_resolver, s->name->lexeme);
            define(p_resolver, s->name->lexeme);

            class_type_t const class_enclosing = p_resolver->current_class;
            p_resolver->current_class = CLASS_TYPE_CLASS;

            if (s->superclass_count > 0) {
                p_resolver->current_class = CLASS_TYPE_SUBCLASS;

                for (size_t i = 0; i < s->superclass_count; i++) {
                    resolve_expression(p_resolver, s->superclass[i]);
                    if (s->superclass[i]->type == EXPR_VARIABLE) {
                        char const * super_name =
                            s->superclass[i]->as.variable_expr.name->lexeme;
                        if (strcmp(s->name->lexeme, super_name) == 0) {
                            fprintf(stderr, "Resolver error: class '%s' cannot inherit from itself.\n",
                                super_name);
                            exit(EXIT_FAILURE);
                        }
                    }
                }
                begin_scope(p_resolver);
                map_put(stack_peek(p_resolver->scopes), "super", new_bool(true));
            }
            begin_scope(p_resolver);
            map_put(stack_peek(p_resolver->scopes), "this", new_bool(true));

            for (size_t i = 0; i < s->methods_count; i++) {
                stmt_t const * p_method = s->methods[i];
                function_type_t decl = FUNCTION_TYPE_METHOD;
                if (strcmp(p_method->as.function_stmt.name->lexeme, "init") == 0) {
                    decl = FUNCTION_TYPE_INITIALIZER;
                }


                //resolve_function(p_resolver, &p_method->as.function_stmt, decl);

                function_type_t const method_enclosing = p_resolver->current_function;
                p_resolver->current_function = decl;
                begin_scope(p_resolver);

                for (size_t j = 0; i < p_method->as.function_stmt.params_count; j++) {
                    token_t const * param = p_method->as.function_stmt.params[j];
                    declare(p_resolver, param->lexeme);
                    define(p_resolver, param->lexeme);
                }
                list_t method_body = {
                    .data = (void**)p_method->as.function_stmt.body,
                    .count = p_method->as.function_stmt.count,
                    .capacity = p_method->as.function_stmt.count,
                    .free_fn = NULL // no need to free, points to existing statements
                };
                resolve(p_resolver, &method_body);
                end_scope(p_resolver);
                p_resolver->current_function = method_enclosing;

            }

            end_scope(p_resolver);

            if (s->superclass_count > 0) end_scope(p_resolver);

            p_resolver->current_class = class_enclosing;
            break;
        case STMT_EXPRESSION:
            resolve_expression(p_resolver, p_stmt->as.expression_stmt.expression);
            break;
        case STMT_IF:
            resolve_expression(p_resolver, p_stmt->as.if_stmt.condition);
            resolve_statement(p_resolver, p_stmt->as.if_stmt.then_branch);
            if (p_stmt->as.if_stmt.else_branch)
                resolve_statement(p_resolver, p_stmt->as.if_stmt.else_branch);
            break;
        case STMT_PRINT:
            resolve_expression(p_resolver, p_stmt->as.print_stmt.expression);
            break;
        case STMT_RETURN:
            if (p_resolver->current_function == FUNCTION_TYPE_NONE) {
                fprintf(stderr, "Resolver error: return outside function.\n");
                exit(EXIT_FAILURE);
            }
            if (p_stmt->as.return_stmt.value) {
                if (p_resolver->current_function == FUNCTION_TYPE_INITIALIZER) {
                    fprintf(stderr, "Resolver error: can't return a value from an initializer.\n");
                    exit(EXIT_FAILURE);
                }
                resolve_expression(p_resolver, p_stmt->as.return_stmt.value);
            }
            break;
        case STMT_VAR:
            const char * name = p_stmt->as.var_stmt.name->lexeme;
            declare(p_resolver, name);
            if (p_stmt->as.var_stmt.initializer)
                resolve_expression(p_resolver, p_stmt->as.var_stmt.initializer);
            define(p_resolver, p_stmt->as.var_stmt.name->lexeme);
            break;
        case STMT_WHILE:
            resolve_expression(p_resolver, p_stmt->as.while_stmt.condition);
            resolve_statement(p_resolver, p_stmt->as.while_stmt.body);
            break;
        default:
            fprintf(stderr, "Not implemented (%d)\n", p_stmt->type);
            exit(EXIT_FAILURE);
    }
}
static void resolve_expression(resolver_t * p_resolver, expr_t * p_expr) {
    if (!p_resolver || !p_expr) return;
    switch (p_expr->type) {
        case EXPR_ASSIGN:
            resolve_expression(p_resolver, p_expr->as.assign_expr.value);
            if (p_expr->as.assign_expr.target->type == EXPR_VARIABLE) {
                char const * name = p_expr->as.assign_expr.target->as.variable_expr.name->lexeme;
                // if resolve local returns -1, assignment is in global scope
                p_expr->as.assign_expr.target->as.variable_expr.depth =
                    resolve_local(p_resolver, p_expr, name);
            }
            break;
        case EXPR_BINARY:
            resolve_expression(p_resolver, p_expr->as.binary_expr.left);
            resolve_expression(p_resolver, p_expr->as.binary_expr.right);
            break;
        case EXPR_CALL:
            resolve_expression(p_resolver, p_expr->as.call_expr.callee);
            for (size_t i = 0; i < p_expr->as.call_expr.count; i++) {
                resolve_expression(p_resolver, p_expr->as.call_expr.arguments[i]);
            }
            break;
        case EXPR_GET:
            resolve_expression(p_resolver, p_expr->as.get_expr.object);
            break;
        case EXPR_GROUPING:
            resolve_expression(p_resolver, p_expr->as.grouping_expr.expression);
            break;
        case EXPR_LITERAL:
            break;
        case EXPR_LOGICAL:
            resolve_expression(p_resolver, p_expr->as.logical_expr.left);
            resolve_expression(p_resolver, p_expr->as.logical_expr.right);
            break;
        case EXPR_SET:
            resolve_expression(p_resolver, p_expr->as.set_expr.value);
            resolve_expression(p_resolver, p_expr->as.set_expr.object);
        case EXPR_SUPER:
            if (p_resolver->current_class == CLASS_TYPE_NONE) {
                fprintf(stderr, "Resolver error: 'super' used outside of a class.\n");
                exit(EXIT_FAILURE);
            }
            if (p_resolver->current_class != CLASS_TYPE_SUBCLASS) {
                fprintf(stderr, "Resolver error: 'super' used in a class with no superclass.\n");
                exit(EXIT_FAILURE);
            }
            resolve_local(p_resolver, p_expr, "super");
            break;
        case EXPR_THIS:
            if (p_resolver->current_class == CLASS_TYPE_NONE) {
                fprintf(stderr, "Resolver error: 'this' used outside of a class.\n");
                return;
            }
            resolve_local(p_resolver, p_expr, "this");
            break;
        case EXPR_UNARY:
            resolve_expression(p_resolver, p_expr->as.unary_expr.right);
            break;
        case EXPR_VARIABLE:
            if (!stack_is_empty(p_resolver->scopes)) {
                map_t * scope = stack_peek(p_resolver->scopes);
                if (map_contains(scope, p_expr->as.variable_expr.name->lexeme)) {
                    bool * ret;
                    if (!map_get(scope, p_expr->as.variable_expr.name->lexeme, (void**)&ret)) {
                        fprintf(stderr, "failed to get from map\n");
                        exit(EXIT_FAILURE);
                    }
                    bool const * defined = ret;
                    if (defined && *defined == false) {
                        fprintf(
                            stderr,
                            "Resolver error: cannot read local variable '%s' in its own initializer\n",
                            p_expr->as.variable_expr.name->lexeme);
                        exit(EXIT_FAILURE);
                    }
                }
            }
            p_expr->as.variable_expr.depth =
                resolve_local(p_resolver, p_expr, p_expr->as.variable_expr.name->lexeme);
            break;
        default:
            fprintf(stderr, "Not implemented (%d)\n", p_expr->type);
            exit(EXIT_FAILURE);

    }
}
static void * bool_copy(void const * ptr) {
    bool * ret = malloc(sizeof(bool));
    if (!ret) exit(EXIT_FAILURE);
    *ret = *(bool*)ptr;
    return ret;
}
static void * str_copy(void const * ptr) {
    size_t const len = strlen(ptr);
    char * copy = malloc(len + 1);
    memcpy(copy, ptr, len);
    copy[len] = '\0';
    return copy;
}
static bool str_equal(void const * a, void const * b) {
    if (!a || !b) return false;
    return strcmp(a, b) == 0;
}
static size_t str_hash(void const * key) {
    if (!key) return 0;
    unsigned char const * s = key;
    size_t hash = 5381;
    int c;
    while ((c = *s++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}
// Map of type <char*, bool*>
static void begin_scope(resolver_t const * p_resolver) {
    map_config_t const charptr_bool_cfg = {
        .value_copy = bool_copy,
        .value_free = free,
        .key_copy = str_copy,
        .key_free = free,
        .key_equals = str_equal,
        .key_hash = str_hash,
        .key_size = sizeof(char*),
        .value_size = sizeof(bool),
    };
    if (!stack_push(p_resolver->scopes,
        map_create(1, &charptr_bool_cfg)))
        exit(EXIT_FAILURE);
}

static void end_scope(resolver_t const * p_resolver) {
    if (!stack_is_empty(p_resolver->scopes)) {
        map_t * scope = stack_pop(p_resolver->scopes);
        map_destroy(scope);
    } else {
        exit(EXIT_FAILURE);
    }
}

static bool * new_bool(bool const v) {
    bool * p = malloc(sizeof(bool));
    if (!p) return NULL;
    *p = v;
    return p;
}
static void declare(resolver_t const * p_resolver, char const * p_name) {
    if (stack_is_empty(p_resolver->scopes)) return;
    map_t * scope = stack_peek(p_resolver->scopes);
    if (map_contains(scope, p_name)) {
        fprintf(stderr, "Resolver error: variable already declared in this scope");
        exit(EXIT_FAILURE);
    }
    bool constexpr defined = false;
    map_put(scope, p_name, &defined);

}
static void define(resolver_t const * p_resolver, char const * p_name) {
    if (stack_is_empty(p_resolver->scopes)) return;
    map_t * scope = stack_peek(p_resolver->scopes);
    if (!map_contains(scope, p_name)) return;

    // bool * ret;
    // if (!map_get(scope, p_name, (void**)&ret)) {
    //     fprintf(stderr, "failed to get from map\n");
    //     exit(EXIT_FAILURE);
    // }

    ////bool * p_old = ret;
    //if (p_old) free(p_old);
    bool constexpr defined = true;
    map_put(scope, p_name, &defined);
}

static int resolve_local(resolver_t const * p_resolver, expr_t * p_expr, char const * p_name) {
    for (int i = (int)stack_size(p_resolver->scopes) - 1; i >= 0; i--) {
        const map_t * scope = (map_t*)p_resolver->scopes->data[i];
        if (map_contains(scope, p_name)) {
            const int distance = (int)stack_size(p_resolver->scopes) - 1 - i;
            if (p_expr->type == EXPR_VARIABLE)
                p_expr->as.variable_expr.depth = distance;
            //interpreter_resolve(p_resolver->interpreter, p_expr, distance);
            return distance;
        }
    }
    return -1;
}