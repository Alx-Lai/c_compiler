#include <stdbool.h>
#include <stdio.h>

#include "mycc.h"

#ifndef MY_UTIL_H
#define MY_UTIL_H

/* vector tools */
TokenVector *init_token_vector();
void push_back_token(TokenVector *, Token);

VariableVector *init_variable_vector();
void push_back_variable(VariableVector *, Variable);
void pop_back_variable(VariableVector *);

ASTVector *init_AST_vector();
void push_back_AST(ASTVector *, struct AST *);

/* parser */
void seek_token(int);
Token peek_token(TokenVector *);
Token next_token(TokenVector *);
void back_token();
int getpos_token();
int get_variable_offset(VariableVector *, char *);

/* util */
char assign_to_origin(char);  // += to +
Token init_token(enum TokenType, uintptr_t);
#define init_punctuation(ch) init_token(PUNCTUATION, (uintptr_t)ch)
#define init_keyword(type) init_token(KEYWORD, (uintptr_t)type)
#define init_identifier(str) init_token(IDENTIFIER, (uintptr_t)str)
bool is_punctuation(Token, char);
bool is_assignment(Token);
bool is_keyword(Token, enum KeywordType);
int get_label_counter();
bool is_binary_op(Token);
int get_precedence(Token);

/* debug related */
void print_lex(TokenVector *);
void print_ast(AST *);
#define errf(...) fprintf(stderr, __VA_ARGS__)
void _fail(char *, int);
#define fail() _fail(__FILE__, __LINE__)
void _fail_if(char *, int, bool);
#define fail_if(x) _fail_if(__FILE__, __LINE__, (x))
#define fail_ifn(x) fail_if(!(x))

#endif