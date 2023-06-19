#include "typing.h"
#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MAX_CODE 0x1000
#define BUF_SIZE 0x200

Token tokens[MAX_CODE];
char code[MAX_CODE];
char output[MAX_CODE] = {0};
char buf[BUF_SIZE];

Token init_token(enum TokenType type, uintptr_t data) {
  return (Token){
      .type = type,
      .data = data,
  };
}

Token init_punctuation(char ch) {
  return init_token(PUNCTUATION, (uintptr_t)ch);
}

Token init_keyword(enum KeywordType type) {
  return init_token(KEYWORD, (uintptr_t)type);
}

Token init_identifier(char *str) {
  return init_token(IDENTIFIER, (uintptr_t)str);
}

bool is_punctuation(Token token, char ch) {
  return token.type == PUNCTUATION && token.data == (uintptr_t)ch;
}

bool is_keyword(Token token, enum KeywordType type) {
  return token.type == KEYWORD && token.data == (uintptr_t)type;
}

int get_label_counter() {
  static int counter = 0;
  return counter++;
}

bool is_binary_op(Token token) {
  char type = (char)token.data;
  static char binary_op_set[] = {'+',
                                 '-',
                                 '*',
                                 '/',
                                 PUNCTUATION_logical_and,
                                 PUNCTUATION_logical_or,
                                 PUNCTUATION_equal,
                                 PUNCTUATION_not_equal,
                                 '<',
                                 PUNCTUATION_less_equal,
                                 '>',
                                 PUNCTUATION_greater_equal,
                                 '%',
                                 '&',
                                 '|',
                                 '^',
                                 PUNCTUATION_bitwise_shift_left,
                                 PUNCTUATION_bitwise_shift_right};
  int len = 18;
  for (int i = 0; i < len; i++)
    if (type == binary_op_set[i])
      return true;
  return false;
}

void fail(int line_number) {
  fprintf(stderr, "fail at line %d\n", line_number);
  assert(0);
}

enum KeywordType parse_keyword(char *word) {
  if (!strcmp(word, "return"))
    return KEYWORD_return;
  if (!strcmp(word, "int"))
    return KEYWORD_int;
  return KEYWORD_unknown;
};

int get_precedence(Token token) {
  if (token.type != PUNCTUATION)
    return 0;
  char type = (char)token.data;
  /* not deal with unary op */
  switch (type) {
  case '*':
  case '/':
  case '%':
    return 3;
  case '+':
  case '-':
    return 4;
  case PUNCTUATION_bitwise_shift_left:
  case PUNCTUATION_bitwise_shift_right:
    return 5;
  case '<':
  case '>':
  case PUNCTUATION_less_equal:
  case PUNCTUATION_greater_equal:
    return 6;
  case PUNCTUATION_equal:
  case PUNCTUATION_not_equal:
    return 7;
  case '&':
    return 8;
  case '^':
    return 9;
  case '|':
    return 10;
  case PUNCTUATION_logical_and:
    return 11;
  case PUNCTUATION_logical_or:
    return 12;
  default:
    return 0;
  }
}

int lex(char code[]) {
  int token_counter = 0, code_counter = 0, line_counter = 0, word_counter = 0;
  while (code[code_counter]) {
    switch (code[code_counter]) {
    case '{':
    case '}':
    case '(':
    case ')':
    case ';':
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '~':
    case '^':
      tokens[token_counter++] = init_punctuation(code[code_counter]);
      code_counter++;
      break;
    case '!':
      if (code[code_counter + 1] == '=') {
        tokens[token_counter++] = init_punctuation(PUNCTUATION_not_equal);
        code_counter++;
      } else {
        tokens[token_counter++] = init_punctuation(code[code_counter]);
      }
      code_counter++;
      break;
    case '>':
    case '<':
      if (code[code_counter + 1] == '=') {
        if (code[code_counter] == '>')
          tokens[token_counter++] = init_punctuation(PUNCTUATION_greater_equal);
        else if (code[code_counter] == '<')
          tokens[token_counter++] = init_punctuation(PUNCTUATION_less_equal);
        code_counter++;
      } else if (code[code_counter] == code[code_counter + 1]) {
        if (code[code_counter] == '>')
          tokens[token_counter++] =
              init_punctuation(PUNCTUATION_bitwise_shift_right);
        else if (code[code_counter] == '<')
          tokens[token_counter++] =
              init_punctuation(PUNCTUATION_bitwise_shift_left);
        code_counter++;
      } else {
        tokens[token_counter++] = init_punctuation(code[code_counter]);
      }
      code_counter++;
      break;
    case '=':
      if (code[code_counter + 1] == '=') {
        tokens[token_counter++] = init_punctuation(PUNCTUATION_equal);
        code_counter += 2;
      } else {
        fail(__LINE__); // assign is not implemented
      }
      break;
    case '&':
    case '|':
      if (code[code_counter + 1] == code[code_counter]) {
        if (code[code_counter] == '&')
          tokens[token_counter++] = init_punctuation(PUNCTUATION_logical_and);
        else if (code[code_counter] == '|')
          tokens[token_counter++] = init_punctuation(PUNCTUATION_logical_or);
        code_counter++;
      } else {
        tokens[token_counter++] = init_punctuation(code[code_counter]);
      }
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
      if (isalpha(code[code_counter])) {
        do {
          buf[word_counter++] = code[code_counter++]; // todo: vector
        } while (isalnum(code[code_counter]));
        buf[word_counter] = '\0';
        char *name = (char *)malloc((word_counter + 1) * sizeof(char));
        strcpy(name, buf);
        int keyword_type = parse_keyword(buf);
        if (keyword_type != KEYWORD_unknown) {
          tokens[token_counter++] = init_keyword(keyword_type);
        } else {
          tokens[token_counter++] = init_identifier(name);
        }
      } else if (isdigit(code[code_counter])) {
        do {
          buf[word_counter++] = code[code_counter++];
        } while (isdigit(code[code_counter]));
        buf[word_counter] = '\0';
        char *name = (char *)malloc((word_counter + 1) * sizeof(char));
        strcpy(name, buf);
        tokens[token_counter++] = init_token(LITERAL, (uintptr_t)name);
      }
    }
  }
  return token_counter;
}

void print_lex(Token tokens[], int token_count) {
  for (int i = 0; i < token_count; i++) {
    printf("%03d:", i);
    if (tokens[i].type == PUNCTUATION)
      printf("PUNCTUATION%c ", (char)tokens[i].data);
    if (tokens[i].type == KEYWORD)
      printf("KEYWORD(%d) ", (int)tokens[i].data);
    if (tokens[i].type == IDENTIFIER)
      printf("IDENTIFIER(%s) ", (char *)tokens[i].data);
    if (tokens[i].type == LITERAL)
      printf("LITERAL(%s) ", (char *)tokens[i].data);
    puts("");
  }
}
AST *parse_expression(Token tokens[], int *counter);

AST *parse_unary_expression(Token tokens[], int *counter) {
  AST *ret = (AST *)malloc(sizeof(AST));
  /* unary*/
  if (is_punctuation(tokens[(*counter)], '-') ||
      is_punctuation(tokens[(*counter)], '~') ||
      is_punctuation(tokens[(*counter)], '!')) {
    int type = (char)tokens[(*counter)].data;
    (*counter)++;
    *ret = (AST){
        .ast_type = AST_unary_op,
        .type = type,
        .exp = parse_unary_expression(tokens, counter),
    };
    return ret;
  } else if (tokens[(*counter)].type == LITERAL) { // literal
    *ret = (AST){
        .ast_type = AST_literal,
        .type = KEYWORD_int,
        .val = atoi((char *)tokens[(*counter)].data),
    };
    (*counter)++;
  } else if (is_punctuation(tokens[(*counter)], '(')) {
    free(ret);
    (*counter)++;
    ret = parse_expression(tokens, counter);
    assert(is_punctuation(tokens[(*counter)++], ')'));
  } else {
    fail(__LINE__);
  }
  return ret;
}

AST *parse_expression(Token tokens[], int *counter) {
  AST *left = parse_unary_expression(tokens, counter), *right;
  /* binary op */
  /* binary parse from left to right  */
  if (is_binary_op(tokens[(*counter)])) {
    int pred_front = get_precedence(tokens[(*counter)]);
    char type = (char)tokens[(*counter)].data;
    (*counter)++;
    right = parse_unary_expression(tokens, counter);
    // peek next op precedence
    /**
     * a - b * c
     *    -
     *   / \
     *  a   *
     *     / \
     *    b   c
     *
     * a - b - c
     *      -
     *     / \
     *    -   c
     *   / \
     *  a   b
     *
     */
    AST *ret = (AST *)malloc(sizeof(AST));
    if (is_binary_op(tokens[(*counter)])) {
      int pred_back = get_precedence(tokens[(*counter)]);
      if (pred_front > pred_back) {
        char type2 = (char)tokens[(*counter)].data;
        (*counter)++;
        AST *new_right = (AST *)malloc(sizeof(AST));
        *new_right = (AST){
            .ast_type = AST_binary_op,
            .type = type2,
            .left = right,
            .right = parse_expression(tokens, counter),
        };
        *ret = (AST){
            .ast_type = AST_binary_op,
            .type = type,
            .left = left,
            .right = new_right,
        };
      } else {
        char type2 = (char)tokens[(*counter)].data;
        (*counter)++;
        AST *sub_root = (AST *)malloc(sizeof(AST));
        *sub_root = (AST){
            .ast_type = AST_binary_op,
            .type = type,
            .left = left,
            .right = right,
        };
        *ret = (AST){
            .ast_type = AST_binary_op,
            .type = type2,
            .left = sub_root,
            .right = parse_expression(tokens, counter),
        };
      }
    } else {
      *ret = (AST){
          .ast_type = AST_binary_op,
          .type = type,
          .left = left,
          .right = right,
      };
    }
    return ret;
  } else {
    return left;
  }
}

AST *parse_statement(Token tokens[], int *counter) {
  AST *ret = (AST *)malloc(sizeof(AST)); // TODO: prevent memory leak
  if (is_keyword(tokens[(*counter)], KEYWORD_return)) {
    (*counter)++;
    *ret = (AST){
        .ast_type = AST_return,
        .return_value = parse_expression(tokens, counter),
    };
    assert(is_punctuation(tokens[(*counter)++], ';'));
    return ret;
  }
  fail(__LINE__);
  return NULL;
}

AST *parse_function(Token tokens[], int *counter) {
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

AST *parse_ast(Token tokens[], int token_count) {
  int counter = 0;
  AST *ret = parse_function(tokens, &counter);
  assert(counter == token_count);
  return ret;
}

void output_ast(AST *ast) {
  if (ast->ast_type == AST_function) {
    sprintf(buf,
            ".globl %s\n"
            "%s:\n",
            ast->func_name, ast->func_name);
    strcat(output, buf);
    output_ast(ast->body);
  } else if (ast->ast_type == AST_return) {
    output_ast(ast->return_value);
    strcat(output, "ret\n");
  } else if (ast->ast_type == AST_literal) {
    sprintf(buf, "mov $%ld, %%rax\n", ast->val);
    strcat(output, buf);
  } else if (ast->ast_type == AST_unary_op) {
    output_ast(ast->exp);
    if (ast->type == '-') {
      strcat(output, "neg %rax\n");
    } else if (ast->type == '!') {
      strcat(output, "not %rax\n");
    } else if (ast->type == '~') {
      strcat(output, "cmp $0, %rax\n"
                     "mov $0, %rax\n"
                     "sete %al\n");
    }
  } else if (ast->ast_type == AST_binary_op) {
    // cannot push 32 bit reg in x64 machine
    int counter;
    switch (ast->type) {
    case '+':
      output_ast(ast->left);
      strcat(output, "push %rax\n");
      output_ast(ast->right);
      strcat(output, "pop %rcx\n"
                     "add %rcx, %rax\n");
      break;
    case '-':
      /* sub rcx, rax ; rax = rax - rcx */
      output_ast(ast->left);
      strcat(output, "push %rax\n");
      output_ast(ast->right);
      strcat(output, "push %rax\n"
                     "pop %rcx\n"
                     "pop %rax\n"
                     "sub %rcx, %rax\n");
      break;
    case '*':
      /* imul rbx ; rdx:rax = rbx*rax */
      output_ast(ast->left);
      strcat(output, "push %rax\n");
      output_ast(ast->right);
      strcat(output, "pop %rbx\n"
                     "imul %rbx\n"); // don't care about overflow now
      break;
    case '/':
      /* idiv rbx ; rax = rdx:rax / rbx, rdx = rdx:rax % rax  */
      output_ast(ast->left); // TODO: refactor
      strcat(output, "push %rax\n");
      output_ast(ast->right);
      strcat(output, "push %rax\n"      // denominator
                     "xor %rdx, %rdx\n" // clean rdx
                     "pop %rbx\n"
                     "pop %rax\n" // don't care about overflow now
                     "idiv %rbx\n");
      break;
    case PUNCTUATION_logical_and:
      counter = get_label_counter();
      output_ast(ast->left);
      sprintf(buf,
              "cmp $0, %%rax\n"
              "jne _clause%d\n"
              "jmp _end%d\n"
              "_clause%d:\n",
              counter, counter, counter);
      strcat(output, buf);
      output_ast(ast->right);
      sprintf(buf,
              "cmp $0, %%rax\n"
              "mov $0, %%rax\n"
              "setne %%al\n"
              "_end%d:\n",
              counter);
      strcat(output, buf);
      break;
    case PUNCTUATION_logical_or:
      counter = get_label_counter();
      output_ast(ast->left);
      sprintf(buf,
              "cmp $0, %%rax\n"
              "je _clause%d\n"
              "mov $1, %%rax\n"
              "jmp _end%d\n"
              "_clause%d:\n",
              counter, counter, counter);
      strcat(output, buf);
      output_ast(ast->right);
      sprintf(buf,
              "cmp $0, %%rax\n"
              "mov $0, %%rax\n"
              "setne %%al\n"
              "_end%d:\n",
              counter);
      strcat(output, buf);
      break;
    case PUNCTUATION_equal:
      output_ast(ast->left);
      strcat(output, "push %rax\n");
      output_ast(ast->right);
      strcat(output, "pop %rcx\n"
                     "cmp %rax, %rcx\n"
                     "mov $0, %rax\n" // use xor clean flags
                     "sete %al\n");
      break;
    case PUNCTUATION_not_equal:
      output_ast(ast->left);
      strcat(output, "push %rax\n");
      output_ast(ast->right);
      strcat(output, "pop %rcx\n"
                     "cmp %rax, %rcx\n"
                     "mov $0, %rax\n"
                     "setne %al\n");
      break;
    case '<':
      output_ast(ast->left);
      strcat(output, "push %rax\n");
      output_ast(ast->right);
      strcat(output, "pop %rcx\n"
                     "cmp %rax, %rcx\n"
                     "mov $0, %rax\n"
                     "setl %al\n");
      break;
    case '>':
      output_ast(ast->left);
      strcat(output, "push %rax\n");
      output_ast(ast->right);
      strcat(output, "pop %rcx\n"
                     "cmp %rcx, %rax\n"
                     "mov $0, %rax\n"
                     "setl %al\n");
      break;
    case PUNCTUATION_less_equal:
      output_ast(ast->left);
      strcat(output, "push %rax\n");
      output_ast(ast->right);
      strcat(output, "pop %rcx\n"
                     "cmp %rcx, %rax\n"
                     "mov $0, %rax\n"
                     "setge %al\n");
      break;
    case PUNCTUATION_greater_equal:
      output_ast(ast->left);
      strcat(output, "push %rax\n");
      output_ast(ast->right);
      strcat(output, "pop %rcx\n"
                     "cmp %rax, %rcx\n"
                     "mov $0, %rax\n"
                     "setge %al\n");
      break;
    case '%':
      /* idiv rbx ; rax = rdx:rax / rbx, rdx = rdx:rax % rax  */
      output_ast(ast->left); // TODO: refactor
      strcat(output, "push %rax\n");
      output_ast(ast->right);
      strcat(output, "push %rax\n"      // denominator
                     "xor %rdx, %rdx\n" // clean rdx
                     "pop %rbx\n"
                     "pop %rax\n" // don't care about overflow now
                     "idiv %rbx\n"
                     "mov %rdx, %rax");
      break;
    case '&':
      output_ast(ast->left);
      strcat(output, "push %rax\n");
      output_ast(ast->right);
      strcat(output, "pop %rcx\n"
                     "and %rcx, %rax\n");
      break;
    case '|':
      output_ast(ast->left);
      strcat(output, "push %rax\n");
      output_ast(ast->right);
      strcat(output, "pop %rcx\n"
                     "or %rcx, %rax\n");
      break;
    case '^':
      output_ast(ast->left);
      strcat(output, "push %rax\n");
      output_ast(ast->right);
      strcat(output, "pop %rcx\n"
                     "xor %rcx, %rax\n");
      break;
    case PUNCTUATION_bitwise_shift_left:
      output_ast(ast->left); // TODO: refactor
      strcat(output, "push %rax\n");
      output_ast(ast->right);
      strcat(output, "xor %rcx, %rcx\n"
                     "mov %al, %cl\n"
                     "pop %rax\n"       // don't care about overflow now
                     "shl %cl, %rax\n");
      break;
    case PUNCTUATION_bitwise_shift_right:
      output_ast(ast->left); // TODO: refactor
      strcat(output, "push %rax\n");
      output_ast(ast->right);
      strcat(output, "mov %rax, %rcx\n"
                     "pop %rax\n" // don't care about overflow now
                     "shr %cl, %rax\n");
      break;
    default:
      fail(__LINE__);
      break;
    }
  } else {
    fail(__LINE__);
  }
}

void output_program(AST *ast) {
  output_ast(ast);
  int fd = open("a.s", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  write(fd, output, strlen(output));
  close(fd);
}

int main(int argc, char *argv[]) {
  assert(argc >= 2);
  int fd = open(argv[1], O_RDONLY);
  read(fd, code, MAX_CODE);
  close(fd);
  int token_count = lex(code);
  print_lex(tokens, token_count);
  AST *ast = parse_ast(tokens, token_count);
  output_program(ast);
}
