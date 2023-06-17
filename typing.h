#include <string.h>

/* lexer */
enum TokenType{
    PUNCTUATION, // {} [] () ; ~ ! + - * /
    KEYWORD, // keyword
    IDENTIFIER, // identifier
    LITERAL, // literal
};

typedef struct Token{
    enum TokenType type;
    /* https://stackoverflow.com/questions/1845482/what-is-the-uintptr-t-data-type */
    uintptr_t data;
} Token;

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

/* parser */

enum ASTType{
    AST_literal,
    AST_function,
    AST_return,
    AST_unary_op,
    AST_binary_op,
};

typedef struct AST{
    int ast_type;

    int type; // including function return type
    union{
        /* literal integer */
        long val;

        /* return */
        struct AST *return_value;

        /* declare function */
        struct{
            char *func_name;
            struct AST *body;
        };

        /* Unary operator */
        struct{
            struct AST *exp;
        };

        /* Binary operator */
        struct{
            struct AST *left, *right;
        };

    };
} AST;