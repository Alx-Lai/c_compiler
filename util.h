#include <stdbool.h>

#include "mycc.h"

#ifndef MY_UTIL_H
#define MY_UTIL_H

/* vector tools */
TokenVector *init_token_vector();
void push_back_token(TokenVector *, Token);

VariableVector *init_variable_vector();
void push_back_variable(VariableVector *, Variable);

ASTVector *init_AST_vector();
void push_back_AST(ASTVector *, struct AST *);

/* parser */
void seek_token(int);
Token peek_token(TokenVector *);
Token next_token(TokenVector *);
void back_token();
int getpos_token();

/* util */
char assign_to_origin(char);  // += to +
Token init_token(enum TokenType, uintptr_t);
Token init_punctuation(char);
Token init_keyword(enum KeywordType);
Token init_identifier(char *);
bool is_punctuation(Token, char);
bool is_assignment(Token);
bool is_keyword(Token, enum KeywordType);
int get_label_counter();
bool is_binary_op(Token);
void fail(int);
int get_precedence(Token);
enum KeywordType parse_keyword(char *);

#endif