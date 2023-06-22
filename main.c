#include "typing.h"
#include "util.h"
#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MAX_CODE 0x1000
#define BUF_SIZE 0x200

char code[MAX_CODE];
char buf[BUF_SIZE];
VariableVector *variables;
FILE *output_f;


int get_variable_offset(char *name) {
  for (int i = 0; i < variables->size; i++) {
    if (!strcmp(name, variables->arr[i].name))
      return variables->arr[i].offset;
  }
  return -1;
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
  case '=':
  case PUNCTUATION_add_equal:
  case PUNCTUATION_sub_equal:
  case PUNCTUATION_div_equal:
  case PUNCTUATION_mul_equal:
  case PUNCTUATION_mod_equal:
  case PUNCTUATION_shift_left_equal:
  case PUNCTUATION_shift_right_equal:
  case PUNCTUATION_bitwise_and_equal:
  case PUNCTUATION_bitwise_or_equal:
  case PUNCTUATION_bitwise_xor_equal:
    return 14;
  default:
    return 0;
  }
}

void lex(TokenVector *tokens, char code[]) {
  int code_counter = 0, word_counter = 0;
  while (code[code_counter]) {
    switch (code[code_counter]) {
    case '{':
    case '}':
    case '(':
    case ')':
    case ';':
    case '~':
      push_back_token(tokens, init_punctuation(code[code_counter]));
      code_counter++;
      break;
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '^':
      if (code[code_counter + 1] == '=') {
        if (code[code_counter] == '+')
          push_back_token(tokens, init_punctuation(PUNCTUATION_add_equal));
        else if (code[code_counter] == '-')
          push_back_token(tokens, init_punctuation(PUNCTUATION_sub_equal));
        else if (code[code_counter] == '*')
          push_back_token(tokens, init_punctuation(PUNCTUATION_mul_equal));
        else if (code[code_counter] == '/')
          push_back_token(tokens, init_punctuation(PUNCTUATION_div_equal));
        else if (code[code_counter] == '%')
          push_back_token(tokens, init_punctuation(PUNCTUATION_mod_equal));
        else if (code[code_counter] == '^')
          push_back_token(tokens,
                          init_punctuation(PUNCTUATION_bitwise_xor_equal));
        code_counter += 2;
      } else {
        push_back_token(tokens, init_punctuation(code[code_counter]));
        code_counter++;
      }
      break;
    case '!':
      if (code[code_counter + 1] == '=') {
        push_back_token(tokens, init_punctuation(PUNCTUATION_not_equal));
        code_counter++;
      } else {
        push_back_token(tokens, init_punctuation(code[code_counter]));
      }
      code_counter++;
      break;
    case '>':
    case '<':
      if (code[code_counter] == code[code_counter + 1] &&
          code[code_counter + 2] == '=') {
        /* >>= <<= */
        if (code[code_counter] == '>') {
          push_back_token(tokens,
                          init_punctuation(PUNCTUATION_shift_right_equal));
        } else if (code[code_counter] == '<') {
          push_back_token(tokens,
                          init_punctuation(PUNCTUATION_shift_left_equal));
        }
        code_counter += 3;
      } else if (code[code_counter] == code[code_counter + 1]) {
        /* >> << */
        if (code[code_counter] == '>') {
          push_back_token(tokens,
                          init_punctuation(PUNCTUATION_bitwise_shift_right));
        } else if (code[code_counter] == '<') {
          push_back_token(tokens,
                          init_punctuation(PUNCTUATION_bitwise_shift_left));
        }
        code_counter += 2;
      } else if (code[code_counter + 1] == '=') {
        /* >= <= */
        if (code[code_counter] == '>') {
          push_back_token(tokens, init_punctuation(PUNCTUATION_greater_equal));
        } else if (code[code_counter] == '<') {
          push_back_token(tokens, init_punctuation(PUNCTUATION_less_equal));
        }
        code_counter += 2;
      } else {
        /* > < */
        push_back_token(tokens, init_punctuation(code[code_counter]));
        code_counter++;
      }
      break;
    case '=':
    case '&':
    case '|':
      if (code[code_counter + 1] == code[code_counter]) {
        if (code[code_counter] == '&')
          push_back_token(tokens, init_punctuation(PUNCTUATION_logical_and));
        else if (code[code_counter] == '|')
          push_back_token(tokens, init_punctuation(PUNCTUATION_logical_or));
        else if (code[code_counter] == '=')
          push_back_token(tokens, init_punctuation(PUNCTUATION_equal));
        code_counter += 2;
      } else if (code[code_counter + 1] == '=') {
        /* &= |= */
        if (code[code_counter] == '&') {
          push_back_token(tokens,
                          init_punctuation(PUNCTUATION_bitwise_and_equal));
        } else if (code[code_counter] == '|') {
          push_back_token(tokens,
                          init_punctuation(PUNCTUATION_bitwise_or_equal));
        }
        code_counter += 2;
      } else {
        push_back_token(tokens, init_punctuation(code[code_counter]));
        code_counter++;
      }
      break;
    case '\n':
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
          push_back_token(tokens, init_keyword(keyword_type));
        } else {
          push_back_token(tokens, init_identifier(name));
        }
      } else if (isdigit(code[code_counter])) {
        do {
          buf[word_counter++] = code[code_counter++];
        } while (isdigit(code[code_counter]));
        buf[word_counter] = '\0';
        char *name = (char *)malloc((word_counter + 1) * sizeof(char));
        strcpy(name, buf);
        push_back_token(tokens, init_token(LITERAL, (uintptr_t)name));
      }
    }
  }
}

void print_lex(TokenVector *tokens) {
  for (int i = 0; i < tokens->size; i++) {
    printf("%03d:", i);
    if (tokens->arr[i].type == PUNCTUATION)
      printf("PUNCTUATION%c ", (char)tokens->arr[i].data);
    if (tokens->arr[i].type == KEYWORD)
      printf("KEYWORD(%d) ", (int)tokens->arr[i].data);
    if (tokens->arr[i].type == IDENTIFIER)
      printf("IDENTIFIER(%s) ", (char *)tokens->arr[i].data);
    if (tokens->arr[i].type == LITERAL)
      printf("LITERAL(%s) ", (char *)tokens->arr[i].data);
    puts("");
  }
}
AST *parse_expression(TokenVector *tokens);

AST *parse_unary_expression(TokenVector *tokens) {
  AST *ret = (AST *)malloc(sizeof(AST));
  /* unary*/
  if (is_punctuation(peek_token(tokens), '-') ||
      is_punctuation(peek_token(tokens), '~') ||
      is_punctuation(peek_token(tokens), '!')) {
    int type = (char)next_token(tokens).data;
    *ret = (AST){
        .ast_type = AST_unary_op,
        .type = type,
        .exp = parse_unary_expression(tokens),
    };
    return ret;
  } else if (peek_token(tokens).type == LITERAL) { // literal
    *ret = (AST){
        .ast_type = AST_literal,
        .type = KEYWORD_int,
        .val = atoi((char *)next_token(tokens).data),
    };
  } else if (is_punctuation(peek_token(tokens), '(')) {
    free(ret);
    next_token(tokens);
    ret = parse_expression(tokens);
    assert(is_punctuation(next_token(tokens), ')'));
  } else if (peek_token(tokens).type == IDENTIFIER) {
    char *name = (char *)next_token(tokens).data;
    if (is_assignment(peek_token(tokens))) {
      if (is_punctuation(peek_token(tokens), '=')) {
        next_token(tokens);
        *ret = (AST){
            .ast_type = AST_assign,
            .type = KEYWORD_int,
            .assign_var_name = name,
            .assign_ast = parse_expression(tokens),
        };
      } else {
        /* b = (a += 2) => b = (a = a + 2) */
        char type = (char)next_token(tokens).data;
        type = assign_to_origin(type);
        AST *sub_ast = (AST *)malloc(sizeof(AST)), *var = (AST *)malloc(sizeof(AST));
        *var = (AST){
          .ast_type = AST_variable,
          .var_name = name,
        };
        *sub_ast = (AST){
          .ast_type = AST_binary_op,
          .type = type,
          .left = var,
          .right = parse_expression(tokens),
        };
        *ret = (AST){
          .ast_type = AST_assign,
          .type = KEYWORD_int,
          .assign_var_name = name,
          .assign_ast = sub_ast,
        };
      }
    } else {
      *ret = (AST){
          .ast_type = AST_variable,
          .type = KEYWORD_int,
          .var_name = name,
      };
    }
  } else {
    fail(__LINE__);
  }
  return ret;
}

AST *parse_expression(TokenVector *tokens) {
  AST *left = parse_unary_expression(tokens), *right;
  /* binary op */
  /* binary parse from left to right  */
  if (is_binary_op(peek_token(tokens))) {
    int pred_front = get_precedence(peek_token(tokens));
    char type = (char)next_token(tokens).data;
    right = parse_unary_expression(tokens);
    // peek next op precedence
    /**
     * a - b * c      *  a - b - c
     *    -           *      -
     *   / \          *     / \
     *  a   *         *    -   c
     *     / \        *   / \
     *    b   c       *  a   b
     */
    AST *ret = (AST *)malloc(sizeof(AST));
    if (is_binary_op(peek_token(tokens))) {
      int pred_back = get_precedence(peek_token(tokens));
      if (pred_front > pred_back) {
        char type2 = (char)next_token(tokens).data;
        AST *new_right = (AST *)malloc(sizeof(AST));
        *new_right = (AST){
            .ast_type = AST_binary_op,
            .type = type2,
            .left = right,
            .right = parse_expression(tokens),
        };
        *ret = (AST){
            .ast_type = AST_binary_op,
            .type = type,
            .left = left,
            .right = new_right,
        };
      } else {
        char type2 = (char)next_token(tokens).data;
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
            .right = parse_expression(tokens),
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

AST *parse_statement(TokenVector *tokens) {
  AST *ret = (AST *)malloc(sizeof(AST)); // TODO: prevent memory leak
  if (is_keyword(peek_token(tokens), KEYWORD_return)) {
    /* return value */
    next_token(tokens);
    *ret = (AST){
        .ast_type = AST_return,
        .return_value = parse_expression(tokens),
    };
    assert(is_punctuation(next_token(tokens), ';'));
    return ret;
  } else if (is_keyword(peek_token(tokens), KEYWORD_int)) {
    /* declare and assignment (optional)*/
    next_token(tokens);
    assert(peek_token(tokens).type == IDENTIFIER);
    char *name = (char *)next_token(tokens).data;
    AST *init_exp = NULL;

    if (!is_punctuation(peek_token(tokens), ';')) {
      // haven't end -> assignment
      assert(is_punctuation(next_token(tokens), '='));
      init_exp = parse_expression(tokens);
    }
    *ret = (AST){
        .type = KEYWORD_int,
        .ast_type = AST_declare,
        .decl_name = name,
        .decl_init = init_exp,
    };
    assert(is_punctuation(next_token(tokens), ';'));
    return ret;
  } else if (peek_token(tokens).type == IDENTIFIER) {
    /* assignment */
    char *name = (char *)next_token(tokens).data;
    assert(is_assignment(peek_token(tokens)));
    char type = (char)next_token(tokens).data;
    if(type == '='){
      *ret = (AST){
        .type = KEYWORD_int,
        .ast_type = AST_assign,
        .assign_var_name = name,
        .assign_ast = parse_expression(tokens),
      };
    } else {
      AST *rvalue = (AST *)malloc(sizeof(AST)), *var = (AST *)malloc(sizeof(AST));
      *var = (AST){
        .ast_type = AST_variable,
        .var_name = name,
      };
      char type2 = assign_to_origin(type);
      *rvalue = (AST){
        .ast_type = AST_binary_op,
        .type = type2,
        .left = var,
        .right = parse_expression(tokens),
      };
      *ret = (AST){
          .type = KEYWORD_int,
          .ast_type = AST_assign,
          .assign_var_name = name,
          .assign_ast = rvalue,
      };
    }
    assert(is_punctuation(next_token(tokens), ';'));
    return ret;
  } else {
    /* else = useless expression */
    ret = parse_expression(tokens);
    assert(is_punctuation(next_token(tokens), ';'));
    return ret;
  }
  return NULL;
}

AST *parse_function(TokenVector *tokens) {
  AST *ret = (AST *)malloc(sizeof(AST));
  /* int */
  assert(is_keyword(peek_token(tokens), KEYWORD_int));
  int return_type = (enum KeywordType)next_token(tokens).data;

  /* main */
  assert(peek_token(tokens).type == IDENTIFIER);
  char *name = (char *)next_token(tokens).data;

  /* () */
  assert(is_punctuation(next_token(tokens), '('));
  assert(is_punctuation(next_token(tokens), ')'));

  /* { */
  assert(is_punctuation(next_token(tokens), '{'));

  ASTVector *body = init_AST_vector();
  while (!is_punctuation(peek_token(tokens), '}')) {
    AST *statement = parse_statement(tokens);
    push_back_AST(body, statement);
  }

  /* } */
  assert(is_punctuation(next_token(tokens), '}'));

  *ret = (AST){
      .ast_type = AST_function,
      .type = return_type,
      .func_name = name,
      .body = body,
  };
  return ret;
}

AST *parse_ast(TokenVector *tokens) {
  seek_token(0);
  AST *ret = parse_function(tokens);
  assert(getpos_token() == tokens->size);
  return ret;
}

#define outf(...) fprintf(output_f, __VA_ARGS__)

void output_ast(AST *ast) {
  static int stack_offset;
  int offset;
  if (ast->ast_type == AST_function) {
    outf(".globl %s\n"
         "%s:\n",
         ast->func_name, ast->func_name);

    // prologue
    outf("push %%rbp\n"
         "mov %%rsp, %%rbp\n"
         "mov $0, %%rax\n");

    stack_offset = -8;
    for (int i = 0; i < ast->body->size; i++)
      output_ast(ast->body->arr[i]);

    // epilogue
    outf("mov %%rbp, %%rsp\n"
         "pop %%rbp\n"
         "ret\n");
  } else if (ast->ast_type == AST_return) {
    output_ast(
        ast->return_value); // put to rax and then return at function epilogue
  } else if (ast->ast_type == AST_literal) {
    outf("mov $%ld, %%rax\n", ast->val);
  } else if (ast->ast_type == AST_unary_op) {
    output_ast(ast->exp);
    if (ast->type == '-') {
      outf("neg %%rax\n");
    } else if (ast->type == '!') {
      outf("not %%rax\n");
    } else if (ast->type == '~') {
      outf("cmp $0, %%rax\n"
           "mov $0, %%rax\n"
           "sete %%al\n");
    }
  } else if (ast->ast_type == AST_binary_op) {
    // cannot push 32 bit reg in x64 machine
    int counter;
    switch (ast->type) {
    case '+':
      output_ast(ast->left);
      outf("push %%rax\n");
      output_ast(ast->right);
      outf("pop %%rcx\n"
           "add %%rcx, %%rax\n");
      break;
    case '-':
      /* sub rcx, rax ; rax = rax - rcx */
      output_ast(ast->left);
      outf("push %%rax\n");
      output_ast(ast->right);
      outf("push %%rax\n"
           "pop %%rcx\n"
           "pop %%rax\n"
           "sub %%rcx, %%rax\n");
      break;
    case '*':
      /* imul rbx ; rdx:rax = rbx*rax */
      output_ast(ast->left);
      outf("push %%rax\n");
      output_ast(ast->right);
      // don't care about overflow
      outf("pop %%rbx\n"
           "imul %%rbx\n");
      break;
    case '/':
      /* idiv rbx ; rax = rdx:rax / rbx, rdx = rdx:rax % rax  */
      output_ast(ast->left); // TODO: refactor
      outf("push %%rax\n");
      output_ast(ast->right);
      outf("push %%rax\n"       // denominator
           "xor %%rdx, %%rdx\n" // clean rdx
           "pop %%rbx\n"
           "pop %%rax\n" // don't care about overflow now
           "idiv %%rbx\n");
      break;
    case PUNCTUATION_logical_and:
      counter = get_label_counter();
      output_ast(ast->left);
      outf("cmp $0, %%rax\n"
           "jne _clause%d\n"
           "jmp _end%d\n"
           "_clause%d:\n",
           counter, counter, counter);
      output_ast(ast->right);
      outf("cmp $0, %%rax\n"
           "mov $0, %%rax\n"
           "setne %%al\n"
           "_end%d:\n",
           counter);
      break;
    case PUNCTUATION_logical_or:
      counter = get_label_counter();
      output_ast(ast->left);
      outf("cmp $0, %%rax\n"
           "je _clause%d\n"
           "mov $1, %%rax\n"
           "jmp _end%d\n"
           "_clause%d:\n",
           counter, counter, counter);
      output_ast(ast->right);
      outf("cmp $0, %%rax\n"
           "mov $0, %%rax\n"
           "setne %%al\n"
           "_end%d:\n",
           counter);
      break;
    case PUNCTUATION_equal:
      output_ast(ast->left);
      outf("push %%rax\n");
      output_ast(ast->right);
      outf("pop %%rcx\n"
           "cmp %%rax, %%rcx\n"
           "mov $0, %%rax\n" // use xor clean flags
           "sete %%al\n");
      break;
    case PUNCTUATION_not_equal:
      output_ast(ast->left);
      outf("push %%rax\n");
      output_ast(ast->right);
      outf("pop %%rcx\n"
           "cmp %%rax, %%rcx\n"
           "mov $0, %%rax\n"
           "setne %%al\n");
      break;
    case '<':
      output_ast(ast->left);
      outf("push %%rax\n");
      output_ast(ast->right);
      outf("pop %%rcx\n"
           "cmp %%rax, %%rcx\n"
           "mov $0, %%rax\n"
           "setl %%al\n");
      break;
    case '>':
      output_ast(ast->left);
      outf("push %%rax\n");
      output_ast(ast->right);
      outf("pop %%rcx\n"
           "cmp %%rcx, %%rax\n"
           "mov $0, %%rax\n"
           "setl %%al\n");
      break;
    case PUNCTUATION_less_equal:
      output_ast(ast->left);
      outf("push %%rax\n");
      output_ast(ast->right);
      outf("pop %%rcx\n"
           "cmp %%rcx, %%rax\n"
           "mov $0, %%rax\n"
           "setge %%al\n");
      break;
    case PUNCTUATION_greater_equal:
      output_ast(ast->left);
      outf("push %%rax\n");
      output_ast(ast->right);
      outf("pop %%rcx\n"
           "cmp %%rax, %%rcx\n"
           "mov $0, %%rax\n"
           "setge %%al\n");
      break;
    case '%':
      /* idiv rbx ; rax = rdx:rax / rbx, rdx = rdx:rax % rax  */
      output_ast(ast->left); // TODO: refactor
      outf("push %%rax\n");
      output_ast(ast->right);
      outf("push %%rax\n"       // denominator
           "xor %%rdx, %%rdx\n" // clean rdx
           "pop %%rbx\n"
           "pop %%rax\n" // don't care about overflow now
           "idiv %%rbx\n"
           "mov %%rdx, %%rax\n");
      break;
    case '&':
      output_ast(ast->left);
      outf("push %%rax\n");
      output_ast(ast->right);
      outf("pop %%rcx\n"
           "and %%rcx, %%rax\n");
      break;
    case '|':
      output_ast(ast->left);
      outf("push %%rax\n");
      output_ast(ast->right);
      outf("pop %%rcx\n"
           "or %%rcx, %%rax\n");
      break;
    case '^':
      output_ast(ast->left);
      outf("push %%rax\n");
      output_ast(ast->right);
      outf("pop %%rcx\n"
           "xor %%rcx, %%rax\n");
      break;
    case PUNCTUATION_bitwise_shift_left:
      output_ast(ast->left); // TODO: refactor
      outf("push %%rax\n");
      output_ast(ast->right);
      outf("xor %%rcx, %%rcx\n"
           "mov %%al, %%cl\n"
           "pop %%rax\n" // don't care about overflow
           "shl %%cl, %%rax\n");
      break;
    case PUNCTUATION_bitwise_shift_right:
      output_ast(ast->left); // TODO: refactor
      outf("push %%rax\n");
      output_ast(ast->right);
      outf("mov %%rax, %%rcx\n"
           "pop %%rax\n" // don't care about overflow
           "shr %%cl, %%rax\n");
      break;
    default:
      fail(__LINE__);
      break;
    }
  } else if (ast->ast_type == AST_declare) {
    if (get_variable_offset(ast->decl_name) != -1) {
      // redefinition
      fail(__LINE__);
    }
    if (ast->decl_init) {
      output_ast(ast->decl_init);
    } else {
      // no initial value set to zero
      outf("mov $0, %%rax\n");
    }
    outf("push %%rax\n");
    push_back_variable(variables, (Variable){
                                      .name = ast->decl_name,
                                      .offset = stack_offset,
                                  });
    stack_offset -= 8;
  } else if (ast->ast_type == AST_assign) {
    output_ast(ast->assign_ast);
    offset = get_variable_offset(ast->assign_var_name);
    assert(offset != -1);
    outf("mov %%rax, %d(%%rbp)\n", offset);
  } else if (ast->ast_type == AST_variable) {
    offset = get_variable_offset(ast->var_name);
    assert(offset != -1);
    outf("mov %d(%%rbp), %%rax\n", offset);
  } else {
    fprintf(stderr, "type:%d", ast->ast_type);
    fail(__LINE__);
  }
}

void print_ast(AST *ast) {
  switch (ast->ast_type) {
  case AST_literal:
    printf("%ld", ast->val);
    break;
  case AST_function:
    assert(ast->type == KEYWORD_int);
    printf("int %s() {\n", ast->func_name);
    for (int i = 0; i < ast->body->size; i++) {
      print_ast(ast->body->arr[i]);
      printf(";\n");
    }
    printf("}\n");
    break;
  case AST_declare:
    assert(ast->type == KEYWORD_int);
    printf("int %s", ast->decl_name);
    if (ast->decl_init) {
      printf(" = ");
      print_ast(ast->decl_init);
    }
    break;
  case AST_return:
    printf("return ");
    print_ast(ast->return_value);
    break;
  case AST_unary_op:
    switch (ast->type) {
    case PUNCTUATION_logical_and:
      printf(" && ");
      break;
    case PUNCTUATION_logical_or:
      printf(" || ");
      break;
    case PUNCTUATION_equal:
      printf(" == ");
      break;
    case PUNCTUATION_not_equal:
      printf(" != ");
      break;
    case PUNCTUATION_less_equal:
      printf(" <= ");
      break;
    case PUNCTUATION_greater_equal:
      printf(" >= ");
      break;
    case PUNCTUATION_bitwise_shift_left:
      printf(" << ");
      break;
    case PUNCTUATION_bitwise_shift_right:
      printf(" >> ");
      break;
    default:
      printf(" %c ", ast->type);
      break;
    }
    print_ast(ast->exp);
    break;
  case AST_binary_op:
    print_ast(ast->left);
    printf(" %c ", ast->type);
    print_ast(ast->right);
    break;
  case AST_assign:
    assert(ast->type == KEYWORD_int);
    printf("%s = ", ast->assign_var_name);
    print_ast(ast->assign_ast);
    break;
  case AST_variable:
    printf("%s", ast->var_name);
    break;
  default:
    fail(__LINE__);
    break;
  }
}

void output_program(AST *ast) {
  output_f = fopen("a.s", "w");
  output_ast(ast);
  fclose(output_f);
}

int main(int argc, char *argv[]) {
  assert(argc >= 2);
  int fd = open(argv[1], O_RDONLY);
  read(fd, code, MAX_CODE);
  close(fd);

  TokenVector *tokens = init_token_vector();
  variables = init_variable_vector();

  lex(tokens, code);
  print_lex(tokens);

  AST *ast = parse_ast(tokens);
  print_ast(ast);

  output_program(ast);
}
