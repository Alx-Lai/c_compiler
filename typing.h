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
  PUNCTUATION_add_equal = 'i', // +=
  PUNCTUATION_sub_equal = 'j', // -=
  PUNCTUATION_div_equal = 'k', // /=
  PUNCTUATION_mul_equal = 'l', // *=
  PUNCTUATION_mod_equal = 'm', // %=
  PUNCTUATION_shift_left_equal = 'n', // <<=
  PUNCTUATION_shift_right_equal = 'o', // >>=
  PUNCTUATION_bitwise_and_equal = 'p', // &=
  PUNCTUATION_bitwise_or_equal = 'q', // |=
  PUNCTUATION_bitwise_xor_equal = 'r', // ^=
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
/* util.c */
extern TokenVector *init_token_vector();
extern void push_back_token(TokenVector *, Token);

enum KeywordType {
  KEYWORD_return,
  KEYWORD_int,
  KEYWORD_unknown,
};

/* parser */
extern void seek_token(int);
extern Token peek_token(TokenVector *);
extern Token next_token(TokenVector *);
extern int getpos_token();
typedef struct Variable{
  char *name;
  int offset; // offset on stack
} Variable;

typedef struct {
    size_t size, capacity;
    Variable *arr;
} VariableVector;
extern VariableVector *init_variable_vector();
extern void push_back_variable(VariableVector *, Variable);


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

struct AST;
typedef struct {
    size_t size, capacity;
    struct AST **arr;
} ASTVector;
/* util.c */
extern ASTVector *init_AST_vector();
extern void push_back_AST(ASTVector *, struct AST *);

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
      ASTVector *body;
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