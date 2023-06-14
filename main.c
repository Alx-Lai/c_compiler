#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <assert.h>
#include "typing.h"
#define MAX_CODE 0x1000
#define BUF_SIZE 0x80

char code[MAX_CODE];

AST tokens[MAX_CODE];

int lex(char code[]){
  int token_counter = 0, code_counter = 0, line_counter = 0, word_counter = 0;
  char word_buf[BUF_SIZE];
  while(code[code_counter]){
    switch(code[code_counter]){
      case '{':
        tokens[token_counter++] = (AST){
          .token_type = OPEN_BRACE,
        };
        code_counter++;
        break;
      case '}':
        tokens[token_counter++] = (AST){
          .token_type = CLOSE_BRACE,
        };
        code_counter++;
        break;
      case '(':
        tokens[token_counter++] = (AST){
          .token_type = OPEN_PARENTHESIS,
        };
        code_counter++;
        break;
      case ')':
        tokens[token_counter++] = (AST){
          .token_type = CLOSE_PARENTHESIS,
        };
        code_counter++;
        break;
      case ';':
        tokens[token_counter++] = (AST){
          .token_type = SEMICOLON,
        };
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
            word_buf[word_counter++] = code[code_counter++]; // todo: vector
          }while(isalnum(code[code_counter]));
          word_buf[word_counter] = '\0';
          char *name = (char *)malloc((word_counter+1) * sizeof(char));
          strcpy(name, word_buf);
          int keyword_type = is_keyword(word_buf);
          if(keyword_type != KEYWORD_unknown){
            tokens[token_counter++] = (AST){
              .token_type = KEYWORD,
                .name = name,
            };
          }else{
            tokens[token_counter++] = (AST){
              .token_type = IDENTIFIER,
                .name = name,
            };
          }
        }else if(isdigit(code[code_counter])){
          do{
            word_buf[word_counter++] = code[code_counter++];
          }while(isdigit(code[code_counter]));
          word_buf[word_counter] = '\0';
          char *name = (char *)malloc((word_counter+1) * sizeof(char));
          strcpy(name, word_buf);
          tokens[token_counter++] = (AST){
            .token_type = LITERAL,
              .name = name,
          };
        }
    }
  }
  return token_counter;
}

int main(int argc, char *argv[]){
  assert(argc>=2);
  int fd = open(argv[1], O_RDONLY);
  read(fd, code, MAX_CODE);
  int token_count = lex(code);
  for(int i=0;i<token_count;i++){
    if(tokens[i].token_type == OPEN_BRACE) printf("BRACE{ ");
    if(tokens[i].token_type == CLOSE_BRACE) printf("BRACE} ");
    if(tokens[i].token_type == OPEN_BRACKET) printf("BRACKET[ ");
    if(tokens[i].token_type == CLOSE_BRACKET) printf("BRACKET] ");
    if(tokens[i].token_type == OPEN_PARENTHESIS) printf("PARENTHESIS( ");
    if(tokens[i].token_type == CLOSE_PARENTHESIS) printf("PARENTHESIS) ");
    if(tokens[i].token_type == SEMICOLON) printf("SEMICOLON; ");
    if(tokens[i].token_type == KEYWORD) printf("KEYWORD(%s) ", tokens[i].name);
    if(tokens[i].token_type == IDENTIFIER) printf("IDENTIFIER(%s) ", tokens[i].name);
    if(tokens[i].token_type == LITERAL) printf("LITERAL(%s) ", tokens[i].name);
  }
}
