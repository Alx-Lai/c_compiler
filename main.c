#include <stdio.h>
#include <stdlib.h>
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


int lex(char code[]){
  int token_counter = 0, code_counter = 0, line_counter = 0, word_counter = 0;
  while(code[code_counter]){
    switch(code[code_counter]){
      case '{':
        tokens[token_counter++] = init_token(OPEN_BRACE, (uintptr_t)'{');
        code_counter++;
        break;
      case '}':
        tokens[token_counter++] = init_token(CLOSE_BRACE, (uintptr_t)'}');
        code_counter++;
        break;
      case '(':
        tokens[token_counter++] = init_token(OPEN_PARENTHESIS, (uintptr_t)'(');
        code_counter++;
        break;
      case ')':
        tokens[token_counter++] = init_token(CLOSE_PARENTHESIS, (uintptr_t)')');
        code_counter++;
        break;
      case ';':
        tokens[token_counter++] = init_token(SEMICOLON, (uintptr_t)';');
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
          int keyword_type = is_keyword(buf);
          if(keyword_type != KEYWORD_unknown){
            tokens[token_counter++] = (Token){
              .token_type = KEYWORD,
              .name = name,
              .keyword_type = keyword_type,
            };
          }else{
            tokens[token_counter++] = (Token){
              .token_type = IDENTIFIER,
              .name = name,
            };
          }
        }else if(isdigit(code[code_counter])){
          do{
            buf[word_counter++] = code[code_counter++];
          }while(isdigit(code[code_counter]));
          buf[word_counter] = '\0';
          char *name = (char *)malloc((word_counter+1) * sizeof(char));
          strcpy(name, buf);
          tokens[token_counter++] = (Token){
            .token_type = LITERAL,
              .name = name,
          };
        }
    }
  }
  return token_counter;
}

void print_lex(Token tokens[], int token_count){
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
  puts("");
}

Statement parse_statement(Token tokens[], int *token_count){
  if(tokens[*token_count].token_type == KEYWORD && tokens[*token_count].keyword_type == KEYWORD_return){
    (*token_count)++;
    if(tokens[*token_count].token_type == LITERAL){
      int return_value = atoi(tokens[(*token_count)++].name);
      assert(tokens[(*token_count)++].token_type == SEMICOLON);
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
  assert(tokens[*token_count].token_type != KEYWORD || tokens[*token_count].keyword_type != KEYWORD_int);
  (*token_count)++;
  
  /* main */
  assert(tokens[*token_count].token_type == IDENTIFIER);
  char *name = tokens[*token_count].name;
  (*token_count)++;

  /* () */
  assert(tokens[(*token_count)++].token_type == OPEN_PARENTHESIS);
  assert(tokens[(*token_count)++].token_type == CLOSE_PARENTHESIS);

  /* { */
  assert(tokens[(*token_count)++].token_type == OPEN_BRACE);

  Statement statement = parse_statement(tokens, token_count);

  assert(tokens[(*token_count)++].token_type == CLOSE_BRACE);

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
