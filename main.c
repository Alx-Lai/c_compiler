#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <assert.h>
#include <stdint.h>
#include "typing.h"
#define MAX_CODE 0x1000
#define BUF_SIZE 0x80

Token tokens[MAX_CODE];
char code[MAX_CODE];
char output[MAX_CODE] = {0};
char buf[BUF_SIZE];


Token init_token(enum TokenType type, uintptr_t data){
  return (Token){
    .type = type,
    .data = data,
  };
}

Token init_punctuation(char ch){
  return init_token(PUNCTUATION, (uintptr_t) ch);
}

Token init_keyword(enum KeywordType type){
  return init_token(KEYWORD, (uintptr_t) type);
}

Token init_identifier(char *str){
  return init_token(IDENTIFIER, (uintptr_t) str);
}

bool is_punctuation(Token token, char ch){
  return token.type == PUNCTUATION && token.data == (uintptr_t) ch;
}

bool is_keyword(Token token, enum KeywordType type){
  return token.type == KEYWORD && token.data == (uintptr_t) type;
}

void fail(){
    assert(0);
}


int lex(char code[]){
  int token_counter = 0, code_counter = 0, line_counter = 0, word_counter = 0;
  while(code[code_counter]){
    switch(code[code_counter]){
      case '{':
      case '}':
      case '(':
      case ')':
      case ';':
      case '+':
      case '-':
      case '*':
      case '/':
      case '~':
      case '!':
        tokens[token_counter++] = init_punctuation(code[code_counter]);
        code_counter++;
        break;
      case '\n':
        line_counter++;
      case ' ':
      case '\t':
      case '\r':
        code_counter++;
        break;
      default:
        word_counter = 0;
        if(isalpha(code[code_counter])){
          do{
            buf[word_counter++] = code[code_counter++]; // todo: vector
          }while(isalnum(code[code_counter]));
          buf[word_counter] = '\0';
          char *name = (char *)malloc((word_counter+1) * sizeof(char));
          strcpy(name, buf);
          int keyword_type = parse_keyword(buf);
          if(keyword_type != KEYWORD_unknown){
            tokens[token_counter++] = init_keyword(keyword_type);
          }else{
            tokens[token_counter++] = init_identifier(name);
          }
        }else if(isdigit(code[code_counter])){
          do{
            buf[word_counter++] = code[code_counter++];
          }while(isdigit(code[code_counter]));
          buf[word_counter] = '\0';
          char *name = (char *)malloc((word_counter+1) * sizeof(char));
          strcpy(name, buf);
          tokens[token_counter++] = init_token(LITERAL, (uintptr_t)name);
        }
    }
  }
  return token_counter;
}

void print_lex(Token tokens[], int token_count){
  for(int i=0;i<token_count;i++){
    printf("%03d:", i);
    if(tokens[i].type == PUNCTUATION) printf("PUNCTUATION%c ", (char)tokens[i].data);
    if(tokens[i].type == KEYWORD) printf("KEYWORD(%d) ", (int)tokens[i].data);
    if(tokens[i].type == IDENTIFIER) printf("IDENTIFIER(%s) ", (char *)tokens[i].data);
    if(tokens[i].type == LITERAL) printf("LITERAL(%s) ", (char *)tokens[i].data);
    puts("");
  }
}

AST *parse_expression(Token tokens[], int *counter){
    AST *ret = (AST *)malloc(sizeof(AST));
    /* unary */
    if(is_punctuation(tokens[(*counter)], '-') ||
       is_punctuation(tokens[(*counter)], '~') ||
       is_punctuation(tokens[(*counter)], '!')){
        int type = (char) tokens[(*counter)].data;
        (*counter)++;
        *ret = (AST){
            .ast_type = AST_unary_op,
            .type = type,
            .exp = parse_expression(tokens, counter),
        };
        return ret;
    }else if(tokens[(*counter)].type == LITERAL ||
             is_punctuation(tokens[(*counter)], '(')){
        AST *left;

        if(tokens[(*counter)].type == LITERAL){
            left = (AST *)malloc(sizeof(AST));
            *left = (AST){
                .ast_type = AST_literal,
                .type = KEYWORD_int,
                .val = atoi((char *)tokens[(*counter)].data),
            };
            (*counter)++;
        }else if(is_punctuation(tokens[(*counter)], '(')){
            (*counter)++;
            left = parse_expression(tokens, counter);
            assert(is_punctuation(tokens[(*counter)++], ')'));
        }

        /* binary op */
        if(is_punctuation(tokens[(*counter)], '+') ||
           is_punctuation(tokens[(*counter)], '-') ||
           is_punctuation(tokens[(*counter)], '*') ||
           is_punctuation(tokens[(*counter)], '/')){
            char type = (char)tokens[(*counter)].data;
            (*counter)++;
            *ret = (AST){
                .ast_type = AST_binary_op,
                .type = type,
                .left = left,
                .right = parse_expression(tokens, counter),
            };
            return ret;
        }

        return left;
    }
    fail();
    return NULL;
}

AST *parse_statement(Token tokens[], int *counter){
    AST *ret = (AST *)malloc(sizeof(AST)); // TODO: prevent memory leak
    if(is_keyword(tokens[(*counter)], KEYWORD_return)){
        (*counter)++;
        *ret = (AST){
            .ast_type = AST_return,
            .return_value = parse_expression(tokens, counter),
        };
        assert(is_punctuation(tokens[(*counter)++], ';'));
        return ret;
    }
    fail();
    return NULL;
}

AST *parse_function(Token tokens[], int *counter){
    AST *ret = (AST *)malloc(sizeof(AST));
    /* int */
    assert(is_keyword(tokens[(*counter)], KEYWORD_int));
    int return_type = (enum KeywordType)tokens[(*counter)++].data;

    /* main */
    assert(tokens[*counter].type == IDENTIFIER);
    char *name = (char *)tokens[(*counter)++].data;

    /* () */
    assert(is_punctuation(tokens[(*counter)++], '('));
    assert(is_punctuation(tokens[(*counter)++], ')'));

    /* { */
    assert(is_punctuation(tokens[(*counter)++], '{'));

    AST *body = parse_statement(tokens, counter);

    /* } */
    assert(is_punctuation(tokens[(*counter)++], '}'));

    *ret = (AST){
        .ast_type = AST_function,
        .type = return_type,
        .func_name = name,
        .body = body,
    };
    return ret;
}

AST *parse_ast(Token tokens[], int token_count){ // todo: parse (1) / 2
    int counter = 0;
    AST *ret = parse_function(tokens, &counter);
    assert(counter == token_count);
    return ret;
}

void output_ast(AST *ast){
  if(ast->ast_type == AST_function){
    sprintf(buf, ".globl %s\n" "%s:\n", ast->func_name, ast->func_name);
    strcat(output, buf);
    output_ast(ast->body);
  }else if(ast->ast_type == AST_return){
    output_ast(ast->return_value);
    strcat(output, "ret\n");
  }else if(ast->ast_type == AST_literal){
    sprintf(buf, "mov $%ld, %%rax\n", ast->val);
    strcat(output, buf);
  }else if(ast->ast_type == AST_unary_op){
    output_ast(ast->exp);
    if(ast->type == '-'){
      strcat(output, "neg %rax\n");
    }else if(ast->type == '!'){
      strcat(output, "not %rax\n");
    }else if(ast->type == '~'){
      strcat(output, "cmp $0, %rax\n" "mov $0, %rax\n" "sete %al\n");
    }
  }else if(ast->ast_type == AST_binary_op){
    output_ast(ast->left);
    strcat(output, "push %rax\n");
    output_ast(ast->right);
    strcat(output, "pop %rcx\n");
    if(ast->type == '+'){
      strcat(output, "add %rcx, %rax\n");
    }else if(ast->type == '-'){
      strcat(output, "sub %rcx, %rax\n");
    }else if(ast->type == '*'){
      strcat(output, "mul %rcx, %rax\n");
    }else if(ast->type == '/'){
      strcat(output, "div %rcx, %rax\n");
    }
  }else{
    fail();
  }
}

void output_program(AST *ast){
  output_ast(ast);
  int fd = open("a.s", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  write(fd, output, strlen(output));
  close(fd);
}

int main(int argc, char *argv[]){
  assert(argc>=2);
  int fd = open(argv[1], O_RDONLY);
  read(fd, code, MAX_CODE);
  close(fd);
  int token_count = lex(code);
  print_lex(tokens, token_count);
  AST *ast = parse_ast(tokens, token_count);
  output_program(ast);
}
