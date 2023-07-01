#include <stdlib.h>

#include "mycc.h"
#include "util.h"

AST *parse_unary_expression(TokenVector *tokens) {
  AST *ret = new_AST();
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
    next_token(tokens);
    ret = parse_assignment_or_expression(tokens);
    fail_ifn(is_punctuation(next_token(tokens), ')'));
  } else if (peek_token(tokens).type == IDENTIFIER) {
    /* pure id e.g. ""a"" || (a = 2) or function call */
    char *name = (char *)next_token(tokens).data;
    if (is_punctuation(peek_token(tokens), '(')) {
      /* function call */
      next_token(tokens);
      ASTVector *call_parameters = init_AST_vector();
      while (!is_punctuation(peek_token(tokens), ')')) {
        AST *parameter = parse_assignment_or_expression(tokens);
        push_back_AST(call_parameters, parameter);
        if (is_punctuation(peek_token(tokens), ',')) {
          next_token(tokens);
        } else {
          break;
        }
      }
      fail_ifn(is_punctuation(next_token(tokens), ')'));
      *ret = (AST){
          .ast_type = AST_function_call,
          .call_function = name,
          .call_parameters = call_parameters,
      };
    } else {
      *ret = (AST){
          .ast_type = AST_variable,
          .var_name = name,
      };
    }
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
    AST *ret = new_AST();
    if (is_binary_op(peek_token(tokens))) {
      int pred_back = get_precedence(peek_token(tokens));
      if (pred_front > pred_back) {
        char type2 = (char)next_token(tokens).data;
        AST *new_right = new_AST();
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
        AST *sub_root = new_AST();
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
    AST *ret = new_AST();
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
  AST *ret = new_AST();
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
      AST *rvalue = new_AST(), *var = new_AST();
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
  AST *ret = new_AST();
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
  } else if (is_keyword(peek_token(tokens), KEYWORD_for)) {
    /* for loop */
    /* for */
    next_token(tokens);
    /* ( */
    fail_ifn(is_punctuation(next_token(tokens), '('));
    AST *for_init = NULL;
    if (is_punctuation(peek_token(tokens), ';')) {
      /* none */
      fail_ifn(is_punctuation(next_token(tokens), ';'));
    } else if (is_keyword(peek_token(tokens), KEYWORD_int)) {
      /* declare */
      for_init = parse_declaration(tokens);
    } else {
      for_init = parse_assignment_or_expression(tokens);
      fail_ifn(is_punctuation(next_token(tokens), ';'));
    }
    AST *for_control = NULL;
    if (is_punctuation(peek_token(tokens), ';')) {
      fail_ifn(is_punctuation(next_token(tokens), ';'));
    } else {
      for_control = parse_assignment_or_expression(tokens);
      fail_ifn(is_punctuation(next_token(tokens), ';'));
    }
    AST *for_post = NULL;
    if (!is_punctuation(peek_token(tokens), ')')) {
      /* not empty */
      for_post = parse_assignment_or_expression(tokens);
    }
    fail_ifn(is_punctuation(next_token(tokens), ')'));
    AST *for_body = parse_statement(tokens);

    *ret = (AST){
        .ast_type = AST_for,
        .for_init = for_init,
        .for_control = for_control,
        .for_post = for_post,
        .for_body = for_body,
    };
  } else if (is_keyword(peek_token(tokens), KEYWORD_while)) {
    /* while loop */
    /* while */
    next_token(tokens);
    fail_ifn(is_punctuation(next_token(tokens), '('));
    AST *while_control = parse_assignment_or_expression(tokens);
    fail_ifn(is_punctuation(next_token(tokens), ')'));
    AST *while_body = parse_statement(tokens);

    *ret = (AST){
        .ast_type = AST_while,
        .while_control = while_control,
        .while_body = while_body,
    };
  } else if (is_keyword(peek_token(tokens), KEYWORD_do)) {
    /* do while loop */
    next_token(tokens);
    AST *do_while_body = parse_statement(tokens);
    fail_ifn(is_keyword(next_token(tokens), KEYWORD_while));
    fail_ifn(is_punctuation(next_token(tokens), '('));
    AST *do_while_control = parse_assignment_or_expression(tokens);
    fail_ifn(is_punctuation(next_token(tokens), ')'));
    fail_ifn(is_punctuation(next_token(tokens), ';'));

    *ret = (AST){
        .ast_type = AST_do_while,
        .do_while_body = do_while_body,
        .do_while_control = do_while_control,
    };
  } else if (is_keyword(peek_token(tokens), KEYWORD_break)) {
    /* break */
    next_token(tokens);
    fail_ifn(is_punctuation(next_token(tokens), ';'));
    *ret = (AST){
        .ast_type = AST_break,
    };
  } else if (is_keyword(peek_token(tokens), KEYWORD_continue)) {
    /* continue */
    next_token(tokens);
    fail_ifn(is_punctuation(next_token(tokens), ';'));
    *ret = (AST){
        .ast_type = AST_continue,
    };
  } else if (is_punctuation(peek_token(tokens), ';')) {
    /* NULL expression */
    next_token(tokens);
    *ret = (AST){
        .ast_type = AST_NULL,
    };
  } else {
    /* expression */
    ret = parse_assignment_or_expression(tokens);
    fail_ifn(is_punctuation(next_token(tokens), ';'));
  }
  return ret;
}

AST *parse_declaration(TokenVector *tokens) {
  fail_ifn(is_keyword(next_token(tokens), KEYWORD_int));
  fail_ifn(peek_token(tokens).type == IDENTIFIER);
  char *name = (char *)next_token(tokens).data;
  AST *init_exp = NULL;
  AST *ret = new_AST();

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
  return ret;
}

AST *parse_statement_or_declaration(TokenVector *tokens) {
  AST *ret;
  if (is_keyword(peek_token(tokens), KEYWORD_int)) {
    /* declare [ and assignment ] */
    ret = parse_declaration(tokens);
  } else {
    /* statement */
    ret = parse_statement(tokens);
  }
  return ret;
}

AST *parse_function(TokenVector *tokens) {
  AST *ret = new_AST();
  /* int */
  fail_ifn(is_keyword(peek_token(tokens), KEYWORD_int));
  int return_type = (enum KeywordType)next_token(tokens).data;

  /* function name */
  fail_ifn(peek_token(tokens).type == IDENTIFIER);
  char *func_name = (char *)next_token(tokens).data;

  /* ( */
  fail_ifn(is_punctuation(next_token(tokens), '('));
  VariableVector *parameters = init_variable_vector();
  /* parameters*/
  if (is_keyword(peek_token(tokens), KEYWORD_int)) {
    next_token(tokens);
    char *para_name = (char *)next_token(tokens).data;
    push_back_variable(parameters, (Variable){
                                       .name = para_name,
                                   });
    while (is_punctuation(peek_token(tokens), ',')) {
      next_token(tokens);
      fail_ifn(is_keyword(next_token(tokens), KEYWORD_int));
      para_name = (char *)next_token(tokens).data;
      push_back_variable(parameters, (Variable){
                                         .name = para_name,
                                     });
    }
  }
  /* ) */
  fail_ifn(is_punctuation(next_token(tokens), ')'));

  if (is_punctuation(peek_token(tokens), ';')) {
    /* function declaration */
    next_token(tokens);
    *ret = (AST){
        .ast_type = AST_function,
        .type = return_type,
        .func_name = func_name,
        .parameters = parameters,
        .body = NULL,
    };
    // body == NULL -> declaration
    // body == {AST_NULL, ...} -> just a empty body
  } else {
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
        .func_name = func_name,
        .parameters = parameters,
        .body = body,
    };
  }
  return ret;
}

void validate(ASTVector *vec, AST *func) {
  if (func->body == NULL) {
    /* declaration */
    for (int i = 0; i < vec->size; i++) {
      if (!strcmp(vec->arr[i]->func_name, func->func_name)) {
        fail_if(vec->arr[i]->parameters->size != func->parameters->size);
      }
    }
  } else {
    /* definition */
    for (int i = 0; i < vec->size; i++) {
      if (!strcmp(vec->arr[i]->func_name, func->func_name)) {
        // defined
        fail_if(vec->arr[i]->body != NULL);
        // different declaration
        fail_if(vec->arr[i]->parameters->size != func->parameters->size);
      }
    }
  }
}

ASTVector *parse_ast(TokenVector *tokens) {
  seek_token(0);
  ASTVector *vec = init_AST_vector();
  while (getpos_token() != tokens->size) {
    AST *ret = parse_function(tokens);
    if (ret->ast_type == AST_function) validate(vec, ret);
    push_back_AST(vec, ret);
  }
  return vec;
}