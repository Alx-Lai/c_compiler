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


int lex(char code[]){
  int token_counter = 0, code_counter = 0, line_counter = 0, word_counter = 0;
  while(code[code_counter]){
    switch(code[code_counter]){
      case '{':
      case '}':
      case '(':
      case ')':
      case ';':
      case '-':
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

Expression *parse_expression(Token tokens[], int *token_count){
  Expression *exp = (Expression *)malloc(sizeof(Expression));
  if(is_punctuation(tokens[*token_count], '-') ||
     is_punctuation(tokens[*token_count], '~') ||
     is_punctuation(tokens[*token_count], '!')){
    int type;
    if(is_punctuation(tokens[*token_count], '-')) type = EXP_Unary_Arithmetic_Negation;
    if(is_punctuation(tokens[*token_count], '~')) type = EXP_Unary_Bitwise_Complement;
    if(is_punctuation(tokens[*token_count], '!')) type = EXP_Unary_Logical_Negation;
    (*token_count)++;
    *exp = (Expression){
      .type = type,
      .exp = parse_expression(tokens, token_count),
    };
  }else if(tokens[*token_count].type == LITERAL){
    *exp = (Expression){
      .type = EXP_Constant,
      .val = atoi((char*) tokens[*token_count].data),
    };
    (*token_count)++;
  }else{
    *exp = (Expression){
      .type = EXP_Unknown,
    };
  }
  // printf("c%d -%d ~%d !%d \n", EXP_Constant, EXP_Unary_Arithmetic_Negation, EXP_Unary_Bitwise_Complement, EXP_Unary_Logical_Negation);
  // printf("exp %d \n", exp->type);
  return exp;
}

Statement parse_statement(Token tokens[], int *token_count){
  if(is_keyword(tokens[(*token_count)++], KEYWORD_return)){
    Statement ret = (Statement){
      .type = STAT_return,
      .return_value = parse_expression(tokens, token_count),
    };
    assert(is_punctuation(tokens[(*token_count)++], ';'));
    return ret;
  }
  return (Statement){
    .type = STAT_unknown,
  };
}

Function parse_function(Token tokens[], int *token_count){
  /* int */
  assert(is_keyword(tokens[(*token_count)++], KEYWORD_int));
  
  /* main */
  assert(tokens[*token_count].type == IDENTIFIER);
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
}

void output_ret_expression(Expression *exp){
  if(exp->type == EXP_Constant){
    sprintf(buf, "mov $%d, %%rax\n", exp->val);
  }else if(exp->type){
    output_ret_expression(exp->exp);
    if(exp->type == EXP_Unary_Arithmetic_Negation)
      sprintf(buf, "neg %%rax\n");
    else if(exp->type == EXP_Unary_Bitwise_Complement)
      sprintf(buf, "not %%rax\n");
    else if(exp->type == EXP_Unary_Logical_Negation)
      sprintf(buf, "cmp $0, %%rax\n" "mov $0, %%rax\n" "sete %%al\n");
  }else{
    assert(false);
  }
  strcat(output, buf);
}

void output_statement(Statement stat){
  if(stat.type == STAT_return){
    output_ret_expression(stat.return_value);
    strcat(output, "ret\n");
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
  int fd = open(output_fn, O_WRONLY | O_CREAT | O_TRUNC, 0644);
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
  Program prog = parse_program(tokens, token_count);
  output_program(prog, "a.s");
}
