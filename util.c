#include "util.h"

#include <stdlib.h>

/* vector tools */
TokenVector *init_token_vector() {
  TokenVector *ret = (TokenVector *)malloc(sizeof(TokenVector));
  Token *arr = (Token *)malloc(2 * sizeof(Token));
  *ret = (TokenVector){
      .size = 0,
      .capacity = 2,
      .arr = arr,
  };
  return ret;
};

void push_back_token(TokenVector *vec, Token t) {
  if (vec->size == vec->capacity) {
    vec->capacity *= 2;
    Token *newarr = (Token *)realloc(vec->arr, vec->capacity * sizeof(Token));
    fail_ifn(newarr);
    vec->arr = newarr;
  }
  vec->arr[vec->size++] = t;
}

VariableVector *init_variable_vector() {
  VariableVector *ret = (VariableVector *)malloc(sizeof(VariableVector));
  Variable *arr = (Variable *)malloc(2 * sizeof(Variable));
  *ret = (VariableVector){
      .size = 0,
      .capacity = 2,
      .arr = arr,
  };
  return ret;
}

void push_back_variable(VariableVector *vec, Variable v) {
  if (vec->size == vec->capacity) {
    vec->capacity *= 2;
    Variable *newarr =
        (Variable *)realloc(vec->arr, vec->capacity * sizeof(Variable));
    fail_ifn(newarr);
    vec->arr = newarr;
  }
  vec->arr[vec->size++] = v;
}

void pop_back_variable(VariableVector *vec) {
  fail_if(vec->size <= 0);
  --vec->size;
}

ASTVector *init_AST_vector() {
  ASTVector *ret = (ASTVector *)malloc(sizeof(ASTVector));
  AST **arr = (AST **)malloc(2 * sizeof(AST *));
  *ret = (ASTVector){
      .size = 0,
      .capacity = 2,
      .arr = arr,
  };
  return ret;
}

void push_back_AST(ASTVector *vec, AST *ast) {
  if (vec->size == vec->capacity) {
    vec->capacity *= 2;
    AST **newarr = (AST **)realloc(vec->arr, vec->capacity * sizeof(AST *));
    fail_ifn(newarr);
    vec->arr = newarr;
  }
  vec->arr[vec->size++] = ast;
}

IntVector *init_int_vector() {
  IntVector *ret = (IntVector *)malloc(sizeof(IntVector));
  int *arr = (int *)malloc(2 * sizeof(int));
  *ret = (IntVector){
      .size = 0,
      .capacity = 2,
      .arr = arr,
  };
  return ret;
}

void push_back_int(IntVector *vec, int num) {
  if (vec->size == vec->capacity) {
    vec->capacity *= 2;
    int *newarr = (int *)realloc(vec->arr, vec->capacity * sizeof(int));
    fail_ifn(newarr);
    vec->arr = newarr;
  }
  vec->arr[vec->size++] = num;
}

void pop_back_int(IntVector *vec) {
  fail_if(vec->size <= 0);
  --vec->size;
}

/* parser tools */
static int token_pointer;
void seek_token(int x) { token_pointer = x; }
Token peek_token(TokenVector *vec) { return vec->arr[token_pointer]; }
Token next_token(TokenVector *vec) { return vec->arr[token_pointer++]; }
void back_token() { --token_pointer; }
int getpos_token() { return token_pointer; }
int get_variable_offset(VariableVector *variables, char *name) {
  for (int i = variables->size - 1; i >= 0; i--) {
    if (!strcmp(name, variables->arr[i].name)) return variables->arr[i].offset;
  }
  return -1;
}

/* util */
char assign_to_origin(char ch) {
  switch (ch) {
    case PUNCTUATION_add_equal:
      return '+';
    case PUNCTUATION_sub_equal:
      return '-';
    case PUNCTUATION_div_equal:
      return '/';
    case PUNCTUATION_mul_equal:
      return '*';
    case PUNCTUATION_mod_equal:
      return '%';
    case PUNCTUATION_shift_left_equal:
      return PUNCTUATION_bitwise_shift_left;
    case PUNCTUATION_shift_right_equal:
      return PUNCTUATION_bitwise_shift_right;
    case PUNCTUATION_bitwise_and_equal:
      return '&';
    case PUNCTUATION_bitwise_or_equal:
      return '|';
    case PUNCTUATION_bitwise_xor_equal:
      return '^';
    default:
      break;
  }
  return -1;
}

Token init_token(enum TokenType type, uintptr_t data) {
  return (Token){
      .type = type,
      .data = data,
  };
}

bool is_punctuation(Token token, char ch) {
  return token.type == PUNCTUATION && token.data == (uintptr_t)ch;
}

bool is_assignment(Token token) {
  char ch = (char)token.data;
  return token.type == PUNCTUATION &&
         (ch == '=' || ch == PUNCTUATION_add_equal ||
          ch == PUNCTUATION_sub_equal || ch == PUNCTUATION_div_equal ||
          ch == PUNCTUATION_mul_equal || ch == PUNCTUATION_mod_equal ||
          ch == PUNCTUATION_shift_left_equal ||
          ch == PUNCTUATION_shift_right_equal ||
          ch == PUNCTUATION_bitwise_and_equal ||
          ch == PUNCTUATION_bitwise_or_equal ||
          ch == PUNCTUATION_bitwise_xor_equal);
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
  static char binary_op_set[] = {
      '+',
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
      PUNCTUATION_bitwise_shift_right,
      PUNCTUATION_add_equal,
      PUNCTUATION_sub_equal,
      PUNCTUATION_div_equal,
      PUNCTUATION_mul_equal,
      PUNCTUATION_mod_equal,
      PUNCTUATION_shift_left_equal,
      PUNCTUATION_shift_right_equal,
      PUNCTUATION_bitwise_and_equal,
      PUNCTUATION_bitwise_or_equal,
      PUNCTUATION_bitwise_xor_equal,
  };
  int len = 28;
  for (int i = 0; i < len; i++)
    if (type == binary_op_set[i]) return true;
  return false;
}

int get_precedence(Token token) {
  if (token.type != PUNCTUATION) return 0;
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

char *new_string(char *str) {
  int l = strlen(str);
  char *ret = (char *)malloc((l + 1) * sizeof(char));
  strcpy(ret, str);
  return ret;
}

/* for memory safe */
static ASTVector *AST_vec;
void init_AST() { AST_vec = init_AST_vector(); }

AST *new_AST() {
  AST *ret = (AST *)malloc(sizeof(AST));
  push_back_AST(AST_vec, ret);
  return ret;
}

void free_AST() {
  for (int i = 0; i < AST_vec->size; i++) free(AST_vec->arr[i]);
  free(AST_vec->arr);
  free(AST_vec);
}

/* debug related */
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

void print_ast(AST *ast) {
  switch (ast->ast_type) {
    case AST_literal:
      printf("%ld", ast->val);
      break;
    case AST_function:
      fail_ifn(ast->type == KEYWORD_int);
      printf("int %s(", ast->func_name);
      for (int i = 0; i < ast->parameters->size; i++) {
        printf("int %s", ast->parameters->arr[i].name);
        if (i != ast->parameters->size - 1) printf(", ");
      }
      printf(")");
      if (ast->body == NULL) {
        printf(";\n");
      } else {
        printf("{\n");
        for (int i = 0; i < ast->body->size; i++) {
          print_ast(ast->body->arr[i]);
        }
        printf("}\n");
      }
      break;
    case AST_declare:
      fail_ifn(ast->type == KEYWORD_int);
      printf("int %s", ast->decl_name);
      if (ast->decl_init) {
        printf(" = ");
        print_ast(ast->decl_init);
      }
      printf(";\n");
      break;
    case AST_return:
      printf("return ");
      print_ast(ast->return_value);
      printf(";\n");
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
      fail_ifn(ast->type == KEYWORD_int);
      printf("%s = ", ast->assign_var_name);
      print_ast(ast->assign_ast);
      break;
    case AST_variable:
      printf("%s", ast->var_name);
      break;
    case AST_if:
      printf("if (");
      print_ast(ast->condition);
      printf(")\n");
      print_ast(ast->if_body);
      printf("\n");
      if (ast->else_body) {
        printf(" else \n");
        print_ast(ast->else_body);
      }
      printf("\n");
      break;
    case AST_ternary:
      print_ast(ast->condition);
      printf(" ? ");
      print_ast(ast->if_body);
      printf(" : ");
      print_ast(ast->else_body);
      break;
    case AST_compound:
      printf("{\n");
      for (int i = 0; i < ast->statements->size; i++) {
        print_ast(ast->statements->arr[i]);
      }
      printf("\n}\n");
      break;
    case AST_for:
      printf("for(");
      if (ast->for_init) print_ast(ast->for_init);
      printf(";");
      if (ast->for_control) print_ast(ast->for_control);
      printf(";");
      if (ast->for_post) print_ast(ast->for_post);
      printf(")\n");
      print_ast(ast->for_body);
      break;
    case AST_while:
      printf("while(");
      print_ast(ast->while_control);
      printf(")");
      print_ast(ast->while_body);
      break;
    case AST_do_while:
      printf("do\n");
      print_ast(ast->do_while_body);
      printf("while(");
      print_ast(ast->do_while_control);
      printf(");");
      break;
    case AST_break:
      puts("break;");
      break;
    case AST_continue:
      puts("continue;");
      break;
    case AST_NULL:
      break;
    case AST_function_call:
      printf("%s(", ast->call_function);
      for (int i = 0; i < ast->call_parameters->size; i++) {
        print_ast(ast->call_parameters->arr[i]);
        if (i != ast->call_parameters->size - 1) printf(", ");
      }
      printf(")");
      break;
    default:
      fail();
      break;
  }
}
