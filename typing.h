#include <string.h>

typedef struct __TOKEN{
    int token_type;
    long val;
    union{
        char *name; // including literal
    };
} Token;

enum TokenType{
    OPEN_BRACE, // {
    CLOSE_BRACE, // }
    OPEN_BRACKET, // [
    CLOSE_BRACKET, // ]
    OPEN_PARENTHESIS, // (
    CLOSE_PARENTHESIS, // )
    SEMICOLON, // ;
    KEYWORD, // keyword
    IDENTIFIER, // identifier
    LITERAL, // literal
};

enum KeywordType{
    KEYWORD_return,
    KEYWORD_int,
    KEYWORD_unknown,
};

enum KeywordType is_keyword(char *word){
    if(!strcmp(word, "return")) return KEYWORD_return;
    if(!strcmp(word, "INT")) return KEYWORD_int;
    return KEYWORD_unknown;
};
