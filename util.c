#include "util.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

TokenVector *init_token_vector() {
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
void seek_token(int x) { token_pointer = x; }
Token peek_token(TokenVector *vec) { return vec->arr[token_pointer]; }
Token next_token(TokenVector *vec) { return vec->arr[token_pointer++]; }
void back_token() { --token_pointer; }
int getpos_token() { return token_pointer; }

ASTVector *init_AST_vector() {
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

VariableVector *init_variable_vector() {
  VariableVector *ret = (VariableVector *)malloc(sizeof(VariableVector));
  Variable *arr = (Variable *)malloc(2 * sizeof(Variable));
  *ret = (VariableVector){
      .size = 0,
      .capacity = 2,
      .arr = arr,
  };
  return ret;
}

void push_back_variable(VariableVector *vec, Variable v) {
  if (vec->size == vec->capacity) {
    vec->capacity *= 2;
    Variable *newarr =
        (Variable *)realloc(vec->arr, vec->capacity * sizeof(Variable));
    assert(newarr);
    vec->arr = newarr;
  }
  vec->arr[vec->size++] = v;
}

extern char assign_to_origin(char ch) {
  switch (ch) {
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

Token init_token(enum TokenType type, uintptr_t data) {
  return (Token){
      .type = type,
      .data = data,
  };
}

Token init_punctuation(char ch) {
  return init_token(PUNCTUATION, (uintptr_t)ch);
}

Token init_keyword(enum KeywordType type) {
  return init_token(KEYWORD, (uintptr_t)type);
}

Token init_identifier(char *str) {
  return init_token(IDENTIFIER, (uintptr_t)str);
}

bool is_punctuation(Token token, char ch) {
  return token.type == PUNCTUATION && token.data == (uintptr_t)ch;
}

bool is_assignment(Token token) {
  char ch = (char)token.data;
  return token.type == PUNCTUATION &&
         (ch == '=' || ch == PUNCTUATION_add_equal ||
          ch == PUNCTUATION_sub_equal || ch == PUNCTUATION_div_equal ||
          ch == PUNCTUATION_mul_equal || ch == PUNCTUATION_mod_equal ||
          ch == PUNCTUATION_shift_left_equal ||
          ch == PUNCTUATION_shift_right_equal ||
          ch == PUNCTUATION_bitwise_and_equal ||
          ch == PUNCTUATION_bitwise_or_equal ||
          ch == PUNCTUATION_bitwise_xor_equal);
}

bool is_keyword(Token token, enum KeywordType type) {
  return token.type == KEYWORD && token.data == (uintptr_t)type;
}

int get_label_counter() {
  static int counter = 0;
  return counter++;
}

bool is_binary_op(Token token) {
  char type = (char)token.data;
  static char binary_op_set[] = {
      '+',
      '-',
      '*',
      '/',
      PUNCTUATION_logical_and,
      PUNCTUATION_logical_or,
      PUNCTUATION_equal,
      PUNCTUATION_not_equal,
      '<',
      PUNCTUATION_less_equal,
      '>',
      PUNCTUATION_greater_equal,
      '%',
      '&',
      '|',
      '^',
      PUNCTUATION_bitwise_shift_left,
      PUNCTUATION_bitwise_shift_right,
      PUNCTUATION_add_equal,
      PUNCTUATION_sub_equal,
      PUNCTUATION_div_equal,
      PUNCTUATION_mul_equal,
      PUNCTUATION_mod_equal,
      PUNCTUATION_shift_left_equal,
      PUNCTUATION_shift_right_equal,
      PUNCTUATION_bitwise_and_equal,
      PUNCTUATION_bitwise_or_equal,
      PUNCTUATION_bitwise_xor_equal,
  };
  int len = 28;
  for (int i = 0; i < len; i++)
    if (type == binary_op_set[i]) return true;
  return false;
}

void fail(int line_number) {
  fprintf(stderr, "fail at line %d\n", line_number);
  assert(0);
}

int get_precedence(Token token) {
  if (token.type != PUNCTUATION) return 0;
  char type = (char)token.data;
  /* not deal with unary op */
  switch (type) {
    case '*':
    case '/':
    case '%':
      return 3;
    case '+':
    case '-':
      return 4;
    case PUNCTUATION_bitwise_shift_left:
    case PUNCTUATION_bitwise_shift_right:
      return 5;
    case '<':
    case '>':
    case PUNCTUATION_less_equal:
    case PUNCTUATION_greater_equal:
      return 6;
    case PUNCTUATION_equal:
    case PUNCTUATION_not_equal:
      return 7;
    case '&':
      return 8;
    case '^':
      return 9;
    case '|':
      return 10;
    case PUNCTUATION_logical_and:
      return 11;
    case PUNCTUATION_logical_or:
      return 12;
    case '=':
    case PUNCTUATION_add_equal:
    case PUNCTUATION_sub_equal:
    case PUNCTUATION_div_equal:
    case PUNCTUATION_mul_equal:
    case PUNCTUATION_mod_equal:
    case PUNCTUATION_shift_left_equal:
    case PUNCTUATION_shift_right_equal:
    case PUNCTUATION_bitwise_and_equal:
    case PUNCTUATION_bitwise_or_equal:
    case PUNCTUATION_bitwise_xor_equal:
      return 14;
    default:
      return 0;
  }
}

enum KeywordType parse_keyword(char *word) {
  if (!strcmp(word, "return")) return KEYWORD_return;
  if (!strcmp(word, "int")) return KEYWORD_int;
  return KEYWORD_unknown;
};