#include <stdlib.h>

#include "mycc.h"
#include "util.h"

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
  } else if (peek_token(tokens).type == LITERAL) {  // literal
    *ret = (AST){
        .ast_type = AST_literal,
        .type = KEYWORD_int,
        .val = atoi((char *)next_token(tokens).data),
    };
  } else if (is_punctuation(peek_token(tokens), '(')) {
    free(ret);
    next_token(tokens);
    ret = parse_assignment_or_expression(tokens);
    fail_ifn(is_punctuation(next_token(tokens), ')'));
  } else if (peek_token(tokens).type == IDENTIFIER) {
    /* pure id e.g. ""a"" || (a = 2) */
    char *name = (char *)next_token(tokens).data;
    *ret = (AST){
        .ast_type = AST_variable,
        .var_name = name,
    };
  } else {
    errf("type:%d %c id:%d\n", peek_token(tokens).type,
         (char)peek_token(tokens).data, getpos_token());
    fail();
  }
  return ret;
}

AST *parse_expression(TokenVector *tokens) {
  AST *left = parse_unary_expression(tokens), *right;
  if (is_binary_op(peek_token(tokens))) {
    /* binary op */
    /* binary parse from left to right  */
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

AST *parse_conditional_expression(TokenVector *tokens) {
  AST *condition = parse_expression(tokens);
  if (is_punctuation(peek_token(tokens), '?')) {
    /* ternary operator */
    next_token(tokens);
    AST *if_body = parse_assignment_or_expression(tokens);
    fail_ifn(is_punctuation(next_token(tokens), ':'));
    AST *else_body = parse_conditional_expression(tokens);
    AST *ret = (AST *)malloc(sizeof(AST));
    *ret = (AST){
        .ast_type = AST_ternary,
        .condition = condition,
        .if_body = if_body,
        .else_body = else_body,
    };
    return ret;
  } else {
    return condition;
  }
}

AST *parse_assignment_or_expression(TokenVector *tokens) {
  AST *ret = (AST *)malloc(sizeof(AST));
  bool assignment = false;
  if (peek_token(tokens).type == IDENTIFIER) {
    next_token(tokens);
    if (is_assignment(peek_token(tokens))) assignment = true;
    back_token();
  }
  if (assignment) {
    /* assignment */
    char *name = (char *)next_token(tokens).data;
    fail_ifn(is_assignment(peek_token(tokens)));
    char type = (char)next_token(tokens).data;
    if (type == '=') {
      /* = */
      *ret = (AST){
          .type = KEYWORD_int,
          .ast_type = AST_assign,
          .assign_var_name = name,
          .assign_ast = parse_assignment_or_expression(tokens),
      };
    } else {
      /* += series */
      AST *rvalue = (AST *)malloc(sizeof(AST)),
          *var = (AST *)malloc(sizeof(AST));
      *var = (AST){
          .ast_type = AST_variable,
          .var_name = name,
      };
      char type2 = assign_to_origin(type);
      *rvalue = (AST){
          .ast_type = AST_binary_op,
          .type = type2,
          .left = var,
          .right = parse_assignment_or_expression(tokens),
      };
      *ret = (AST){
          .type = KEYWORD_int,
          .ast_type = AST_assign,
          .assign_var_name = name,
          .assign_ast = rvalue,
      };
    }
  } else {
    /* expression \ {assignment} */
    ret = parse_conditional_expression(tokens);
  }
  return ret;
}

AST *parse_statement(TokenVector *tokens) {
  AST *ret = (AST *)malloc(sizeof(AST));  // TODO: prevent memory leak
  if (is_keyword(peek_token(tokens), KEYWORD_return)) {
    /* return value */
    next_token(tokens);
    *ret = (AST){
        .ast_type = AST_return,
        .return_value = parse_assignment_or_expression(tokens),
    };
    fail_ifn(is_punctuation(next_token(tokens), ';'));
  } else if (is_keyword(peek_token(tokens), KEYWORD_if)) {
    /* if */
    next_token(tokens);
    fail_ifn(is_punctuation(next_token(tokens), '('));  // (
    AST *condition = parse_assignment_or_expression(tokens);
    fail_ifn(is_punctuation(next_token(tokens), ')'));  // )
    /* now only one statement */
    AST *if_body = parse_statement(tokens);
    AST *else_body = NULL;
    if (is_keyword(peek_token(tokens), KEYWORD_else)) {
      /* else */
      next_token(tokens);
      else_body = parse_statement(tokens);
    }
    *ret = (AST){
        .ast_type = AST_if,
        .condition = condition,
        .if_body = if_body,
        .else_body = else_body,
    };
  } else if (is_punctuation(peek_token(tokens), '{')) {
    /* compound statements */
    /* { */
    next_token(tokens);
    ASTVector *vec = init_AST_vector();
    while (!is_punctuation(peek_token(tokens), '}')) {
      push_back_AST(vec, parse_statement_or_declaration(tokens));
    }
    /* } */
    next_token(tokens);
    *ret = (AST){
        .ast_type = AST_compound,
        .statements = vec,
    };
  } else {
    /* expression */
    ret = parse_assignment_or_expression(tokens);
    fail_ifn(is_punctuation(next_token(tokens), ';'));
  }
  return ret;
}

AST *parse_statement_or_declaration(TokenVector *tokens) {
  AST *ret;
  if (is_keyword(peek_token(tokens), KEYWORD_int)) {
    /* declare [ and assignment ] */
    next_token(tokens);
    fail_ifn(peek_token(tokens).type == IDENTIFIER);
    char *name = (char *)next_token(tokens).data;
    AST *init_exp = NULL;
    ret = (AST *)malloc(sizeof(AST));

    if (!is_punctuation(peek_token(tokens), ';')) {
      // haven't end -> assignment
      fail_ifn(is_punctuation(next_token(tokens), '='));
      init_exp = parse_assignment_or_expression(tokens);
    }
    *ret = (AST){
        .type = KEYWORD_int,
        .ast_type = AST_declare,
        .decl_name = name,
        .decl_init = init_exp,
    };
    fail_ifn(is_punctuation(next_token(tokens), ';'));
  } else {
    /* statement */
    ret = parse_statement(tokens);
  }
  return ret;
}

AST *parse_function(TokenVector *tokens) {
  AST *ret = (AST *)malloc(sizeof(AST));
  /* int */
  fail_ifn(is_keyword(peek_token(tokens), KEYWORD_int));
  int return_type = (enum KeywordType)next_token(tokens).data;

  /* function name */
  fail_ifn(peek_token(tokens).type == IDENTIFIER);
  char *name = (char *)next_token(tokens).data;

  /* () */
  fail_ifn(is_punctuation(next_token(tokens), '('));
  fail_ifn(is_punctuation(next_token(tokens), ')'));

  /* { */
  fail_ifn(is_punctuation(next_token(tokens), '{'));

  ASTVector *body = init_AST_vector();
  while (!is_punctuation(peek_token(tokens), '}')) {
    AST *statement = parse_statement_or_declaration(tokens);
    push_back_AST(body, statement);
  }

  /* } */
  fail_ifn(is_punctuation(next_token(tokens), '}'));

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
  fail_ifn(getpos_token() == tokens->size);
  return ret;
}