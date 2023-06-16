#include <string.h>

enum TokenType{
    PUNCTUATION, // {} [] () ;
    KEYWORD, // keyword
    IDENTIFIER, // identifier
    LITERAL, // literal
};

typedef struct Token{
    enum TokenType token_type;
    uintptr_t data;
} Token;

enum StatementTYPE{
    STAT_return,
    STAT_unknown,
};

typedef struct Statement{
    int type;
    union{
        /* for return */
        int return_value;
    }; 
} Statement;
typedef struct Function{
    char *name;
    Statement statement;
} Function;

typedef struct Program{
    Function func;
} Program;


enum KeywordType{
    KEYWORD_return,
    KEYWORD_int,
    KEYWORD_unknown,
};

enum KeywordType parse_keyword(char *word){
    if(!strcmp(word, "return")) return KEYWORD_return;
    if(!strcmp(word, "int")) return KEYWORD_int;
    return KEYWORD_unknown;
};
