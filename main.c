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


/* https://stackoverflow.com/questions/1845482/what-is-the-uintptr-t-data-type */
Token init_token(enum TokenType type, uintptr_t data){
  return (Token){
    .token_type = type,
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
  return token.token_type == PUNCTUATION && token.data == (uintptr_t) ch;
}

bool is_keyword(Token token, enum KeywordType type){
  return token.token_type == KEYWORD && token.data == (uintptr_t) type;
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
    if(tokens[i].token_type == PUNCTUATION) printf("PUNCTUATION%c ", (char)tokens[i].data);
    if(tokens[i].token_type == KEYWORD) printf("KEYWORD(%d) ", (int)tokens[i].data);
    if(tokens[i].token_type == IDENTIFIER) printf("IDENTIFIER(%s) ", (char *)tokens[i].data);
    if(tokens[i].token_type == LITERAL) printf("LITERAL(%s) ", (char *)tokens[i].data);
  }
  puts("");
}

Statement parse_statement(Token tokens[], int *token_count){
  if(is_keyword(tokens[(*token_count)++], KEYWORD_return)){
    if(tokens[*token_count].token_type == LITERAL){
      int return_value = atoi((char *)tokens[(*token_count)++].data);
      assert(is_punctuation(tokens[*token_count], ';'));
      (*token_count)++;
      return (Statement){
        .type = STAT_return,
        .return_value = return_value,
      };
    }else{
      return (Statement){
        .type = STAT_unknown,
      };
    }
  }
  return (Statement){
    .type = STAT_unknown,
  };
}

Function parse_function(Token tokens[], int *token_count){
  /* int */
  assert(is_keyword(tokens[(*token_count)++], KEYWORD_int));
  
  /* main */
  assert(tokens[*token_count].token_type == IDENTIFIER);
  char *name = (char *)tokens[(*token_count)++].data;

  /* () */
  assert(is_punctuation(tokens[(*token_count)++], '('));
  assert(is_punctuation(tokens[(*token_count)++], ')'));

  /* { */
  assert(is_punctuation(tokens[(*token_count)++], '{'));

  Statement statement = parse_statement(tokens, token_count);

  assert(is_punctuation(tokens[(*token_count)++], '}'));

  return (Function){
    .name = name,
    .statement = statement,
  };
}

Program parse_program(Token tokens[], int token_count){
  int parse_counter = 0;
  Function func = parse_function(tokens, &parse_counter);
  return (Program){
    .func = func,
  };
}

void print_program(Program prog){
  printf("func: %s\n", prog.func.name);
  printf("statement type: %d\n", prog.func.statement.type);
  printf("return value: %d\n", prog.func.statement.return_value);
}

void output_statement(Statement stat){
  if(stat.type == STAT_return){
    sprintf(buf, "mov $%d, %%rax\n" "ret\n", stat.return_value);
    strcat(output, buf);
  }
}

void output_function(Function func){
  sprintf(buf, ".globl %s\n" "%s:\n", func.name, func.name);
  strcat(output, buf);
  output_statement(func.statement);
}

void output_program(Program prog, char output_fn[]){
  output[0] = '\0';
  output_function(prog.func);
  // printf("%s\n", output_fn);
  // printf("%s\n", output);
  int fd = open(output_fn, O_WRONLY | O_CREAT, 0644);
  write(fd, output, strlen(output)+1);
  close(fd);
}

int main(int argc, char *argv[]){
  assert(argc>=2);
  int fd = open(argv[1], O_RDONLY);
  read(fd, code, MAX_CODE);
  close(fd);
  int token_count = lex(code);
  print_lex(tokens, token_count);
  Program prog = parse_program(tokens, token_count);
  print_program(prog);
  output_program(prog, "a.s");
}
