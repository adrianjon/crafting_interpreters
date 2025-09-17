//
// Created by adrian on 2025-09-15.
//


#include "../extra/Arrays.h"
#include "../extra/Windows.h"
#include "../extra/Memory.h"

// static const char* types[] = {
//     "Binary     : Expr left, Token operator, Expr right",
//     "Grouping   : Expr expression",
//     "Literal    : Object value",
//     "Unary      : Token operator, Expr right"
// };
static const char* ast_expr_grammar[] = {
//> Statements and State assign-expr
      "assign   : token_t* name, expr_t* value",
//< Statements and State assign-expr
      "binary   : expr_t* left, token_t* operator, expr_t* right",
//> Functions call-expr
      "call     : expr_t* callee, token_t* paren, dynamic_array_t* arguments",
//< Functions call-expr
//> Classes get-ast
      "get      : expr_t* object, token_t* name",
//< Classes get-ast
      "grouping : expr_t* expression",
      "literal  : token_t* kind",
//> Control Flow logical-ast
      "logical  : expr_t* left, token_t* operator, expr_t* right",
//< Control Flow logical-ast
//> Classes set-ast
      "set      : expr_t* object, token_t* name, expr_t* value",
//< Classes set-ast
//> Inheritance super-expr
      "super    : token_t* keyword, token_t* method",
//< Inheritance super-expr
//> Classes this-ast
      "this     : token_t* keyword",
//< Classes this-ast
/* Representing Code call-define-ast < Statements and State var-expr
      "Unary    : token_t operator, expr_t right"
*/
//> Statements and State var-expr
      "unary    : token_t* operator, expr_t* right",
      "variable : token_t* name",
      NULL
//< Statements and State var-expr
};

static const char* ast_stmt_grammar[] = {
    //> block-ast
      "block    : stmt_t** statements, size_t* count", //change to c syntax
//< block-ast

//> Functions function-ast
      "function   : token_t* name, token_t* params, stmt_t* body",
//< Functions function-ast

/* Classes class-ast < Inheritance superclass-ast
      "Class      : Token name, List<Stmt.Function> methods",
*/
//> Inheritance superclass-ast
      "class     : token_t* name, expr_t* superclass, stmt_function_t* methods",
//< Inheritance superclass-ast
      "expression : expr_t* expression",

//> Control Flow if-ast
      "if         : expr_t* condition, stmt_t* then_branch, stmt_t* else_branch",
//< Control Flow if-ast
/* Statements and State stmt-ast < Statements and State var-stmt-ast
      "Print      : Expr expression"
*/
//> var-stmt-ast
      "print     : expr_t* expression",
//< var-stmt-ast
//> Functions return-ast
      "return    : token_t* keyword, expr_t* value",
//< Functions return-ast
/* Statements and State var-stmt-ast < Control Flow while-ast
      "Var        : Token name, Expr initializer"
*/
//> Control Flow while-ast
      "var        : token_t* name, expr_t* initializer",
      "while      : expr_t* condition, stmt_t* body",
      NULL
//< Control Flow while-ast
};

void GenerateAst(const char* output_dir, const char* string_base_name, const char* types[]) {
    string_builder_t sb = create_string_builder();
    string_builder_t sb_to_c = create_string_builder();

    append_string(&sb, "// This file is auto-generated. Do not edit.\n\n");
    append_string(&sb_to_c, "// This file is auto-generated. Do not edit.\n\n");
    append_string(&sb, "#ifndef ");

    //print_int(sb.capacity);
    //print("\n");
    //print_int(sb.length);
    //print("\n");
    char* buffer = string_to_uppercase(string_base_name);
    append_string(&sb, buffer);
    append_string(&sb, "_H\n#define ");
    append_string(&sb, buffer);
    append_string(&sb, "_H\n\n");
    append_string(&sb, "#include \"../extra/Arrays.h\"\n");
    append_string(&sb, "#include \"Expr.h\"\n");
    append_string(&sb, "//#include \"Stmt.h\"\n");
    append_string(&sb, "#include \"Token.h\"\n\n");
    append_string(&sb, "#include \"Object.h\"\n");
    append_string(&sb, "\n");

    append_string(&sb_to_c, "#include \"Expr.h\"\n");
    append_string(&sb_to_c, "#include \"Stmt.h\"\n");
    append_string(&sb_to_c, "#include \"Token.h\"\n");
    append_string(&sb_to_c, "#include \"Object.h\"\n");
    append_string(&sb_to_c, "\n");

    append_string(&sb, "// Forward declarations\n");
    append_string(&sb, "typedef struct ");
    append_string(&sb, string_base_name);
    append_string(&sb, " ");
    append_string(&sb, string_base_name);
    append_string(&sb, "_t;\n");

    append_string(&sb, "typedef struct ");
    append_string(&sb, string_base_name);
    append_string(&sb, "_visitor ");
    append_string(&sb, string_base_name);
    append_string(&sb, "_visitor_t;\n");

    append_string(&sb, "struct ");
    append_string(&sb, string_base_name);
    append_string(&sb, "_visitor {\n");

    append_string(&sb_to_c, "void* ");
    append_string(&sb_to_c, string_base_name);
      append_string(&sb_to_c, "_accept(const ");
      append_string(&sb_to_c, string_base_name);
      append_string(&sb_to_c, "_t* ");
      append_string(&sb_to_c, string_base_name);
      append_string(&sb_to_c, ", const ");
      append_string(&sb_to_c, string_base_name);
      append_string(&sb_to_c, "_visitor_t* visitor, void* context) {\n");
      append_string(&sb_to_c, "\tswitch(");
      append_string(&sb_to_c, string_base_name);
      append_string(&sb_to_c, "->type) {\n");
      //
      // append_string(&sb_to_c, "\t}\n");
      // append_string(&sb_to_c, "\treturn NULL;\n");
      // append_string(&sb_to_c, "}\n\n");

    int num_types = sizeof(types) / sizeof(types[0]);
//     print_int(sizeof(types));
// print(*(types + 1));
//     print("\n");
      for (int i = 0; types[i]; i++) {
            const char* end = memory_character(types[i], ' ', 256); // 256 change to max size
            const size_t type_name_len = end - types[i];
            char type_name[256] = {0};
            memory_copy(type_name, types[i], type_name_len);
            type_name[type_name_len] = '\0';

            append_string(&sb, "\t\tvoid* (*visit_");
            append_string(&sb, type_name);
            append_string(&sb, ")(const ");
            append_string(&sb, string_base_name);
            append_string(&sb, "_t");
            append_string(&sb, "* ");
            char *buffer = string_to_lowercase(string_base_name);
            append_string(&sb, buffer);
            memory_free((void**)&buffer);
            append_string(&sb, ", const ");
            append_string(&sb, string_base_name);
            append_string(&sb, "_visitor_t* visitor, void* context);\n");
      }

    append_string(&sb, "};\n\n");

    // string_base_name typedef enums
      append_string(&sb, "\n");
      append_string(&sb, "// ");
      append_string(&sb, string_base_name);
      append_string(&sb, " types\n");
      append_string(&sb, "typedef enum {\n");
      for (int i = 0; types[i]; i++) {

            const char* end = memory_character(types[i], ' ', 256); // 256 change to max size
            size_t type_name_len = end - types[i];
            char type_name[256] = {0};
            memory_copy(type_name, types[i], type_name_len);
            type_name[type_name_len] = '\0';

            append_string(&sb, "\t");
            append_string(&sb, string_to_uppercase(string_base_name));
            append_string(&sb, "_");
            append_string(&sb, string_to_uppercase(type_name));

/*
if (visitor->visit_assign)
				return visitor->visit_assign(expr, visitor, context);
			else
				return NULL;
                        */
            append_string(&sb_to_c, "\t\tcase ");
            append_string(&sb_to_c, string_to_uppercase(string_base_name));
            append_string(&sb_to_c, "_");
            append_string(&sb_to_c, string_to_uppercase(type_name));
            append_string(&sb_to_c, ":\n");
            append_string(&sb_to_c, "\t\t\t");
            append_string(&sb_to_c, "if (visitor->visit_");
            append_string(&sb_to_c, type_name);
            append_string(&sb_to_c, ")\n");
            append_string(&sb_to_c, "\t\t\t\t");
            append_string(&sb_to_c, "return visitor->visit_");
            append_string(&sb_to_c, type_name);
            append_string(&sb_to_c, "(");
            append_string(&sb_to_c, string_base_name);
            append_string(&sb_to_c, ", ");
            append_string(&sb_to_c, "visitor, context);\n");


            if (types[i + 1]) {
                append_string(&sb, ",\n");
            } else {
                append_string(&sb, "\n");
            }
      }
      append_string(&sb, "} ");
      append_string(&sb, string_base_name);;
      append_string(&sb, "_type_t;\n");

      append_string(&sb_to_c, "\t\tdefault: return NULL;\n");
      append_string(&sb_to_c, "\t}\n");
      append_string(&sb_to_c, "}\n\n");

      // Create the structs for each type by splitting at the colon
      for(int i = 0; types[i]; i++) {
            append_string(&sb, "\n");
            append_string(&sb, "typedef struct {\n");
            // split types after the colon and extract fields
            const char* colon = memory_character(types[i], ':', 256); // 256
            if (!colon) {
                print_error("Invalid type definition, missing colon");
                memory_free((void**)&buffer);
                memory_free((void**)&sb.buffer);
                return;
            }
            while(*colon == ' ' || *colon == ':') colon++;
            int x = 0;
            const char* fields[16] = {0}; // max 16 fields
            fields[x++] = colon; // first field
            //printf("C: '%s'\n", colon); // debug: each struct fields
            size_t grm_len = 0;
            while(types[i][grm_len] != '\0') grm_len++;
            // printf("Grammar length: %zu\n", grm_len);
            while(memory_character(colon, ',', grm_len)) {
                  //printf(".");

                const char* next_comma = memory_character(colon, ',', grm_len);
                grm_len = grm_len - (next_comma - colon);
                if (next_comma) {
                    while(*next_comma == ' ' || *next_comma == ',') next_comma++;
                    fields[x++] = next_comma;
                    colon = next_comma;
                } else {
                    break;
                }
            }
            //printf("\n");
            fields[x] = NULL; // null terminate
            for(int j = 0; fields[j]; j++) {
                const char* end = memory_character(fields[j], ',', 256); // 256
                size_t field_len = 0;
                if (end) {
                    field_len = end - fields[j];
                } else {
                    while(fields[j][field_len] != '\0') field_len++;
                }
                char field[256] = {0};
                memory_copy(field, fields[j], field_len);
                field[field_len] = '\0';
                append_string(&sb, "\t");
                append_string(&sb, field);
            //     printf("Field: '%s'\n", field);
                append_string(&sb, ";\n");
            }
            //printf("\n");

            append_string(&sb, "\t");
            append_string(&sb, "\n");
            append_string(&sb, "} ");
            append_string(&sb, string_base_name);
            append_string(&sb, "_");
            char type_name[256] = {0};
            const char* end = memory_character(types[i], ' ', 256); // 256
            const size_t type_name_len = end - types[i];
            memory_copy(type_name, types[i], type_name_len);
            append_string(&sb, type_name);
            append_string(&sb, "_t");
            append_string(&sb, ";\n");

            // Split string after : and extract types


      }

      append_string(&sb, "\n");
      append_string(&sb, "struct ");
      append_string(&sb, string_base_name);
      append_string(&sb, " {\n");
      append_string(&sb, "\t");
      append_string(&sb, string_base_name);
      append_string(&sb, "_type_t type;\n");
      append_string(&sb, "\tunion {\n");
      for (int i = 0; types[i]; i++) {
            const char* end = memory_character(types[i], ' ', 256); // 256 change to max size
            const size_t type_name_len = end - types[i];
            char type_name[256] = {0};
            memory_copy(type_name, types[i], type_name_len);
            type_name[type_name_len] = '\0';

            append_string(&sb, "\t\t");
            append_string(&sb, string_base_name);
            append_string(&sb, "_");
            append_string(&sb, type_name);
            append_string(&sb, "_t");
            append_string(&sb, " ");
            char *buffer = string_to_lowercase(type_name);
            append_string(&sb, buffer);
            append_string(&sb, "_");
            buffer = string_to_lowercase(string_base_name);
            append_string(&sb, buffer);
            memory_free((void**)&buffer);
            append_string(&sb, ";\n");
      }
      append_string(&sb, "\t} as;\n");
      append_string(&sb, "};\n");


      append_string(&sb, "\n");
      append_string(&sb, "void* ");
      append_string(&sb, string_base_name);
      append_string(&sb, "_accept(const ");
      append_string(&sb, string_base_name);
      append_string(&sb, "_t* ");
      char *buffer2 = string_to_lowercase(string_base_name);
      append_string(&sb, buffer2);
      memory_free((void**)&buffer2);
      append_string(&sb, ", const ");
      append_string(&sb, string_base_name);
      append_string(&sb, "_visitor_t* visitor, void* context);\n");

      append_string(&sb, "\n#endif // ");
      append_string(&sb, buffer);
      memory_free((void**)&buffer);
      append_string(&sb, "_H\n");
      // debug_memory_pool();
      string_builder_t target_file_h = create_string_builder();
      string_builder_t target_file_c = create_string_builder();
      append_string(&target_file_h, output_dir);
      append_string(&target_file_c, output_dir);
      append_string(&target_file_h, "/");
      append_string(&target_file_c, "/");
      append_string(&target_file_h, string_base_name); // bug here, should have first letter capitalized
      append_string(&target_file_c, string_base_name); // bug here, should have first letter capitalized
      append_string(&target_file_h, ".h");
      append_string(&target_file_c, ".c");
      append_string(&target_file_h, "\0");
      append_string(&target_file_c, "\0");
      // print(target_file.buffer);
      // print("\n");

      char* ptr = target_file_h.buffer;
      size_t sb_len = 0;
      while(*ptr != '\0') {
          sb_len++;
          ptr++;
      }
      print_int(sb_len);
      print("\n");
      // debug_memory_pool();
      print("Writing AST to file: ");
    print(target_file_h.buffer);
      //*(target_file_h.buffer + sb_len - 2) = '\0'; // remove null terminator for printing
      print("\n");

    file_t* p_ast_file_ptr = write_file(target_file_h.buffer, sb.buffer, sb.length);
    memory_free((void**)&target_file_h.buffer);
    memory_free((void**)&p_ast_file_ptr);


    print("Writing AST to file: "); // .c file name same length as .h file
    print(target_file_c.buffer);
    //*(target_file_c.buffer + sb_len - 2) = '\0'; // remove null terminator for printing

      print("\n");
    file_t* p_ast_file_c_ptr = write_file(target_file_c.buffer, sb_to_c.buffer, sb_to_c.length);
//file_t* p_ast_file_ptr = write_file("./lox/poop", sb.buffer, sb.length);

    memory_free((void**)&target_file_c.buffer);
    memory_free((void**)&p_ast_file_c_ptr);
    // free_string_builder(&sb);
}

int main(void) {
      GenerateAst("./lox", "expr", ast_expr_grammar);
      GenerateAst("./lox", "stmt", ast_stmt_grammar);

      // for(int i = 0; ast_expr_grammar[i]; i++) {
      //     char buffer[20][MAX_TOKEN_LEN] = {0};
      //     const char* p = (const char*)memory_character(ast_expr_grammar[i], ':', MAX_TOKEN_LEN) + 1; // move past colon
      //     size_t n = split_string_copy(p, ',', buffer, 20, MAX_TOKEN_LEN);
      //     for(size_t j = 0; j < n; j++) {
      //         trim(buffer[j], ' ');
      //         printf("Field %zu: '%s'\n", j, buffer[j]);
      //     }
      //         printf("\n");
      // }
      //
      // int *x = (int*)memory_allocate(10);
      //       memory_free(&x);
      // printf("Freed malloced memory at %p\n", x);
      //
      // int* p = memory_allocate(sizeof(int) * 10);
      // printf("Allocated memory at %p\n", p);
      // printf("Stack address at %p\n", &p);
      // memory_free((void**)&p);
      // printf("Freed memory at %p\n", p);
      return 0;
}