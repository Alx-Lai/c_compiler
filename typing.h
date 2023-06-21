#include <stdint.h>
#include <string.h>

#ifndef MY_TYPING_H
#define MY_TYPING_H

/* lexer */
enum TokenType {
  PUNCTUATION, // {} [] () ; ~ ! + - * /
  KEYWORD,     // keyword
  IDENTIFIER,  // identifier
  LITERAL,     // literal
};

enum {
  /* for those need two characters */
  PUNCTUATION_logical_and = 'a',   // &&
  PUNCTUATION_logical_or = 'b',    // ||
  PUNCTUATION_equal = 'c',         // ==
  PUNCTUATION_not_equal = 'd',     // !=
  PUNCTUATION_less_equal = 'e',    // <=
  PUNCTUATION_greater_equal = 'f', // >=
  PUNCTUATION_bitwise_shift_left = 'g', // <<
  PUNCTUATION_bitwise_shift_right = 'h', // >>
};

typedef struct Token {
  enum TokenType type;
  /* https://stackoverflow.com/questions/1845482/what-is-the-uintptr-t-data-type
   */
  uintptr_t data;
} Token;

enum KeywordType {
  KEYWORD_return,
  KEYWORD_int,
  KEYWORD_unknown,
};

/* parser */
typedef struct Variable{
  enum KeywordType type;
  char *name;
  uintptr_t data;
} Variable;


enum ASTType {
  AST_literal,
  AST_function,
  AST_declare,
  AST_return,
  AST_unary_op,
  AST_binary_op,
  AST_assign,
  AST_variable,
};

typedef struct AST {
  int ast_type;

  int type; // including function return type
  union {
    /* literal integer */
    long val;

    /* return */
    struct AST *return_value;

    /* declare function */
    struct {
      char *func_name;
      struct AST **body;
    };

    /* declare variable */
    struct{
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
  };
} AST;

#endif