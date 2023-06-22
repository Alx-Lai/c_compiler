#include <stdbool.h>

#include "typing.h"

/* += to + */
char assign_to_origin(char);
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
bool is_binary_op(Token);
int get_precedence(Token);
enum KeywordType parse_keyword(char *);