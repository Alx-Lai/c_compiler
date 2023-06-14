#include <string.h>

typedef struct __AST{
    int token_type;
    long val;
    union{
        char *name;
    };
} AST;

enum Token{
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

enum Keyword{
    KEYWORD_return,
    KEYWORD_int,
    KEYWORD_unknown,
};

enum Keyword is_keyword(char *word){
    if(!strcmp(word, "return")) return KEYWORD_return;
    if(!strcmp(word, "INT")) return KEYWORD_int;
};
