#include <string.h>

typedef struct __TOKEN{
    int token_type;
    long val;
    char *name; // including literal
    union{
        int keyword_type;
    };
} Token;

enum StatementTYPE{
    STAT_return,
    STAT_unknown,
};

typedef struct _Statement{
    int type;
    union{
        /* for return */
        int return_value;
    }; 
} Statement;
typedef struct _Function{
    char *name;
    Statement statement;
} Function;

typedef struct _Program{
    Function func;
} Program;

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
