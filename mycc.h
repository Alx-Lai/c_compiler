#include <stdint.h>
#include <string.h>

#ifndef MY_CC_H
#define MY_CC_H

/* lexer */
enum TokenType {
  PUNCTUATION,  // {} [] () ; ~ ! + - * /
  KEYWORD,      // keyword
  IDENTIFIER,   // identifier
  LITERAL,      // literal
};

enum {
  /* for those need two characters */
  PUNCTUATION_logical_and = 'a',          // &&
  PUNCTUATION_logical_or = 'b',           // ||
  PUNCTUATION_equal = 'c',                // ==
  PUNCTUATION_not_equal = 'd',            // !=
  PUNCTUATION_less_equal = 'e',           // <=
  PUNCTUATION_greater_equal = 'f',        // >=
  PUNCTUATION_bitwise_shift_left = 'g',   // <<
  PUNCTUATION_bitwise_shift_right = 'h',  // >>
  PUNCTUATION_add_equal = 'i',            // +=
  PUNCTUATION_sub_equal = 'j',            // -=
  PUNCTUATION_div_equal = 'k',            // /=
  PUNCTUATION_mul_equal = 'l',            // *=
  PUNCTUATION_mod_equal = 'm',            // %=
  PUNCTUATION_shift_left_equal = 'n',     // <<=
  PUNCTUATION_shift_right_equal = 'o',    // >>=
  PUNCTUATION_bitwise_and_equal = 'p',    // &=
  PUNCTUATION_bitwise_or_equal = 'q',     // |=
  PUNCTUATION_bitwise_xor_equal = 'r',    // ^=
};

typedef struct {
  enum TokenType type;
  /* https://stackoverflow.com/questions/1845482/what-is-the-uintptr-t-data-type
   */
  uintptr_t data;
} Token;

typedef struct {
  size_t size, capacity;
  Token *arr;
} TokenVector;

enum KeywordType {
  KEYWORD_return,
  KEYWORD_int,
  KEYWORD_if,
  KEYWORD_else,
  KEYWORD_unknown,
};

/* lexer.c */
#define BUF_SIZE 0x200
void lex(TokenVector *tokens, char code[]);

/* parser */
typedef struct Variable {
  char *name;
  int offset;  // offset on stack
} Variable;

typedef struct {
  size_t size, capacity;
  Variable *arr;
} VariableVector;

enum ASTType {
  AST_literal,
  AST_function,
  AST_declare,
  AST_return,
  AST_unary_op,
  AST_binary_op,
  AST_assign,
  AST_variable,
  AST_if,
  AST_ternary,
  AST_compound,
};

struct AST;
typedef struct {
  size_t size, capacity;
  struct AST **arr;
} ASTVector;

typedef struct AST {
  int ast_type;

  int type;  // including function return type
  union {
    /* literal integer */
    long val;

    /* return */
    struct AST *return_value;

    /* declare function */
    struct {
      char *func_name;
      ASTVector *body;
    };

    /* declare variable */
    struct {
      char *decl_name;
      struct AST *decl_init;
    };

    /* variable */
    struct {
      char *var_name;
    };

    /* variable assign */
    struct {
      char *assign_var_name;
      struct AST *assign_ast;
    };

    /* Unary operator */
    struct {
      struct AST *exp;
    };

    /* Binary operator */
    struct {
      struct AST *left, *right;
    };

    /* if statement or ternary expression distinguish with ast_type */
    struct {
      struct AST *condition, *if_body, *else_body;
    };

    /* Compound Statement */
    struct {
      ASTVector *statements;
    };
  };
} AST;

/* parser.c */
AST *parse_unary_expression(TokenVector *tokens);
AST *parse_expression(TokenVector *tokens);
AST *parse_conditional_expression(TokenVector *tokens);
AST *parse_assignment_or_expression(TokenVector *tokens);
AST *parse_statement(TokenVector *tokens);
AST *parse_statement_or_declaration(TokenVector *tokens);
AST *parse_function(TokenVector *tokens);
AST *parse_ast(TokenVector *tokens);
#endif