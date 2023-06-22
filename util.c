#include "typing.h"
#include "util.h"
#include <assert.h>
#include <stdlib.h>

TokenVector *init_token_vector() { // TODO: move to util.c
  TokenVector *ret = (TokenVector *)malloc(sizeof(TokenVector));
  Token *arr = (Token *)malloc(2 * sizeof(Token));
  *ret = (TokenVector){
      .size = 0,
      .capacity = 2,
      .arr = arr,
  };
  return ret;
};

void push_back_token(TokenVector *vec, Token t) {
  if (vec->size == vec->capacity) {
    vec->capacity *= 2;
    Token *newarr = (Token *)realloc(vec->arr, vec->capacity * sizeof(Token));
    assert(newarr);
    vec->arr = newarr;
  }
  vec->arr[vec->size++] = t;
}

static int token_pointer;
void seek_token(int x){
    token_pointer = x;
}
Token peek_token(TokenVector *vec){
    return vec->arr[token_pointer];
}
Token next_token(TokenVector *vec){
    return vec->arr[token_pointer++];
}
int getpos_token(){
    return token_pointer;
}


ASTVector *init_AST_vector() { // TODO: move to util.c
  ASTVector *ret = (ASTVector *)malloc(sizeof(ASTVector));
  AST **arr = (AST **)malloc(2 * sizeof(AST *));
  *ret = (ASTVector){
      .size = 0,
      .capacity = 2,
      .arr = arr,
  };
  return ret;
}

void push_back_AST(ASTVector *vec, AST *ast) {
  if (vec->size == vec->capacity) {
    vec->capacity *= 2;
    AST **newarr = (AST **)realloc(vec->arr, vec->capacity * sizeof(AST *));
    assert(newarr);
    vec->arr = newarr;
  }
  vec->arr[vec->size++] = ast;
}

VariableVector *init_variable_vector(){
  VariableVector *ret = (VariableVector *)malloc(sizeof(VariableVector));
  Variable *arr = (Variable *)malloc(2 * sizeof(Variable));
  *ret = (VariableVector){
      .size = 0,
      .capacity = 2,
      .arr = arr,
  };
  return ret;
}

void push_back_variable(VariableVector *vec, Variable v){
  if (vec->size == vec->capacity) {
    vec->capacity *= 2;
    Variable *newarr = (Variable *)realloc(vec->arr, vec->capacity * sizeof(Variable));
    assert(newarr);
    vec->arr = newarr;
  }
  vec->arr[vec->size++] = v;
}

extern char assign_to_origin(char ch){
  switch (ch){
  case PUNCTUATION_add_equal:
    return '+';
  case PUNCTUATION_sub_equal:
    return '-';
  case PUNCTUATION_div_equal:
    return '/';
  case PUNCTUATION_mul_equal:
    return '*';
  case PUNCTUATION_mod_equal:
    return '%';
  case PUNCTUATION_shift_left_equal:
    return PUNCTUATION_bitwise_shift_left;
  case PUNCTUATION_shift_right_equal:
    return PUNCTUATION_bitwise_shift_right;
  case PUNCTUATION_bitwise_and_equal:
    return '&';
  case PUNCTUATION_bitwise_or_equal:
    return '|';
  case PUNCTUATION_bitwise_xor_equal:
    return '^';
  default:
    break;
  }
  return -1;
}