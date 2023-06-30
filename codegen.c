
#include <stdio.h>

#include "mycc.h"
#include "util.h"

static FILE *output_f;
#define outf(...) fprintf(output_f, __VA_ARGS__)

static void output_ast(VariableVector *cur_scope,
                       VariableVector *function_scope, AST *ast,
                       IntVector *Context) {
  /* TODO: refactor or clean or refactor the long code*/
  int offset, counter;
  if (ast->ast_type == AST_function) {
    VariableVector *new_scope = init_variable_vector();
    IntVector *new_Context = init_int_vector();
    outf(
        ".globl %s\n"
        "%s:\n",
        ast->func_name, ast->func_name);

    // prologue
    outf(
        "push %%rbp\n"
        "mov %%rsp, %%rbp\n"
        "mov $0, %%rax\n");
    for (int i = 0; i < ast->body->size; i++)
      output_ast(new_scope, function_scope, ast->body->arr[i], new_Context);

    // epilogue
    outf(
        "mov %%rbp, %%rsp\n"
        "pop %%rbp\n"
        "ret\n");
  } else if (ast->ast_type == AST_return) {
    // put to rax and then return at function epilogue
    output_ast(cur_scope, function_scope, ast->return_value, Context);
    /* epilogue */
    outf(
        "mov %%rbp, %%rsp\n"
        "pop %%rbp\n"
        "ret\n");
  } else if (ast->ast_type == AST_literal) {
    outf("mov $%ld, %%rax\n", ast->val);
  } else if (ast->ast_type == AST_unary_op) {
    output_ast(cur_scope, function_scope, ast->exp, Context);
    if (ast->type == '-') {
      outf("neg %%rax\n");
    } else if (ast->type == '!') {
      outf("not %%rax\n");
    } else if (ast->type == '~') {
      outf(
          "cmp $0, %%rax\n"
          "mov $0, %%rax\n"
          "sete %%al\n");
    }
  } else if (ast->ast_type == AST_binary_op) {
    // cannot push 32 bit reg in x64 machine
    switch (ast->type) {
      case '+':
        output_ast(cur_scope, function_scope, ast->left, Context);
        outf("push %%rax\n");
        output_ast(cur_scope, function_scope, ast->right, Context);
        outf(
            "pop %%rcx\n"
            "add %%rcx, %%rax\n");
        break;
      case '-':
        /* sub rcx, rax ; rax = rax - rcx */
        output_ast(cur_scope, function_scope, ast->left, Context);
        outf("push %%rax\n");
        output_ast(cur_scope, function_scope, ast->right, Context);
        outf(
            "push %%rax\n"
            "pop %%rcx\n"
            "pop %%rax\n"
            "sub %%rcx, %%rax\n");
        break;
      case '*':
        /* imul rbx ; rdx:rax = rbx*rax */
        output_ast(cur_scope, function_scope, ast->left, Context);
        outf("push %%rax\n");
        output_ast(cur_scope, function_scope, ast->right, Context);
        // don't care about overflow
        outf(
            "pop %%rbx\n"
            "imul %%rbx\n");
        break;
      case '/':
        /* idiv rbx ; rax = rdx:rax / rbx, rdx = rdx:rax % rax  */
        output_ast(cur_scope, function_scope, ast->left, Context);
        outf("push %%rax\n");
        output_ast(cur_scope, function_scope, ast->right, Context);
        outf(
            "push %%rax\n"        // denominator
            "xor %%rdx, %%rdx\n"  // clean rdx
            "pop %%rbx\n"
            "pop %%rax\n"  // don't care about overflow now
            "idiv %%rbx\n");
        break;
      case PUNCTUATION_logical_and:
        counter = get_label_counter();
        output_ast(cur_scope, function_scope, ast->left, Context);
        outf(
            "cmp $0, %%rax\n"
            "jne _clause%d\n"
            "jmp _end%d\n"
            "_clause%d:\n",
            counter, counter, counter);
        output_ast(cur_scope, function_scope, ast->right, Context);
        outf(
            "cmp $0, %%rax\n"
            "mov $0, %%rax\n"
            "setne %%al\n"
            "_end%d:\n",
            counter);
        break;
      case PUNCTUATION_logical_or:
        counter = get_label_counter();
        output_ast(cur_scope, function_scope, ast->left, Context);
        outf(
            "cmp $0, %%rax\n"
            "je _clause%d\n"
            "mov $1, %%rax\n"
            "jmp _end%d\n"
            "_clause%d:\n",
            counter, counter, counter);
        output_ast(cur_scope, function_scope, ast->right, Context);
        outf(
            "cmp $0, %%rax\n"
            "mov $0, %%rax\n"
            "setne %%al\n"
            "_end%d:\n",
            counter);
        break;
      case PUNCTUATION_equal:
        output_ast(cur_scope, function_scope, ast->left, Context);
        outf("push %%rax\n");
        output_ast(cur_scope, function_scope, ast->right, Context);
        outf(
            "pop %%rcx\n"
            "cmp %%rax, %%rcx\n"
            "mov $0, %%rax\n"  // use xor clean flags
            "sete %%al\n");
        break;
      case PUNCTUATION_not_equal:
        output_ast(cur_scope, function_scope, ast->left, Context);
        outf("push %%rax\n");
        output_ast(cur_scope, function_scope, ast->right, Context);
        outf(
            "pop %%rcx\n"
            "cmp %%rax, %%rcx\n"
            "mov $0, %%rax\n"
            "setne %%al\n");
        break;
      case '<':
        output_ast(cur_scope, function_scope, ast->left, Context);
        outf("push %%rax\n");
        output_ast(cur_scope, function_scope, ast->right, Context);
        outf(
            "pop %%rcx\n"
            "cmp %%rax, %%rcx\n"
            "mov $0, %%rax\n"
            "setl %%al\n");
        break;
      case '>':
        output_ast(cur_scope, function_scope, ast->left, Context);
        outf("push %%rax\n");
        output_ast(cur_scope, function_scope, ast->right, Context);
        outf(
            "pop %%rcx\n"
            "cmp %%rcx, %%rax\n"
            "mov $0, %%rax\n"
            "setl %%al\n");
        break;
      case PUNCTUATION_less_equal:
        output_ast(cur_scope, function_scope, ast->left, Context);
        outf("push %%rax\n");
        output_ast(cur_scope, function_scope, ast->right, Context);
        outf(
            "pop %%rcx\n"
            "cmp %%rcx, %%rax\n"
            "mov $0, %%rax\n"
            "setge %%al\n");
        break;
      case PUNCTUATION_greater_equal:
        output_ast(cur_scope, function_scope, ast->left, Context);
        outf("push %%rax\n");
        output_ast(cur_scope, function_scope, ast->right, Context);
        outf(
            "pop %%rcx\n"
            "cmp %%rax, %%rcx\n"
            "mov $0, %%rax\n"
            "setge %%al\n");
        break;
      case '%':
        /* idiv rbx ; rax = rdx:rax / rbx, rdx = rdx:rax % rax  */
        output_ast(cur_scope, function_scope, ast->left, Context);
        outf("push %%rax\n");
        output_ast(cur_scope, function_scope, ast->right, Context);
        outf(
            "push %%rax\n"        // denominator
            "xor %%rdx, %%rdx\n"  // clean rdx
            "pop %%rbx\n"
            "pop %%rax\n"  // don't care about overflow now
            "idiv %%rbx\n"
            "mov %%rdx, %%rax\n");
        break;
      case '&':
        output_ast(cur_scope, function_scope, ast->left, Context);
        outf("push %%rax\n");
        output_ast(cur_scope, function_scope, ast->right, Context);
        outf(
            "pop %%rcx\n"
            "and %%rcx, %%rax\n");
        break;
      case '|':
        output_ast(cur_scope, function_scope, ast->left, Context);
        outf("push %%rax\n");
        output_ast(cur_scope, function_scope, ast->right, Context);
        outf(
            "pop %%rcx\n"
            "or %%rcx, %%rax\n");
        break;
      case '^':
        output_ast(cur_scope, function_scope, ast->left, Context);
        outf("push %%rax\n");
        output_ast(cur_scope, function_scope, ast->right, Context);
        outf(
            "pop %%rcx\n"
            "xor %%rcx, %%rax\n");
        break;
      case PUNCTUATION_bitwise_shift_left:
        output_ast(cur_scope, function_scope, ast->left, Context);
        outf("push %%rax\n");
        output_ast(cur_scope, function_scope, ast->right, Context);
        outf(
            "xor %%rcx, %%rcx\n"
            "mov %%al, %%cl\n"
            "pop %%rax\n"  // don't care about overflow
            "shl %%cl, %%rax\n");
        break;
      case PUNCTUATION_bitwise_shift_right:
        output_ast(cur_scope, function_scope, ast->left, Context);
        outf("push %%rax\n");
        output_ast(cur_scope, function_scope, ast->right, Context);
        outf(
            "mov %%rax, %%rcx\n"
            "pop %%rax\n"  // don't care about overflow
            "shr %%cl, %%rax\n");
        break;
      default:
        fail();
        break;
    }
  } else if (ast->ast_type == AST_declare) {
    if (get_variable_offset(cur_scope, ast->decl_name) != -1) {
      // redefinition
      fail();
    }
    if (ast->decl_init) {
      output_ast(cur_scope, function_scope, ast->decl_init, Context);
    } else {
      // no initial value set to zero
      outf("mov $0, %%rax\n");
    }
    outf("push %%rax\n");
    int stack_offset;
    if (cur_scope->size == 0)
      stack_offset = -8;
    else
      stack_offset = cur_scope->arr[cur_scope->size - 1].offset - 8;
    push_back_variable(cur_scope, (Variable){
                                      .name = ast->decl_name,
                                      .offset = stack_offset,
                                  });
    if (function_scope->size == 0)
      stack_offset = -8;
    else
      stack_offset = function_scope->arr[function_scope->size - 1].offset - 8;
    push_back_variable(function_scope, (Variable){
                                           .name = ast->decl_name,
                                           .offset = stack_offset,
                                       });

  } else if (ast->ast_type == AST_assign) {
    output_ast(cur_scope, function_scope, ast->assign_ast, Context);
    offset = get_variable_offset(function_scope, ast->assign_var_name);
    fail_if(offset == -1);
    outf("mov %%rax, %d(%%rbp)\n", offset);
  } else if (ast->ast_type == AST_variable) {
    offset = get_variable_offset(function_scope, ast->assign_var_name);
    fail_if(offset == -1);
    outf("mov %d(%%rbp), %%rax\n", offset);
  } else if (ast->ast_type == AST_if) {
    counter = get_label_counter();
    output_ast(cur_scope, function_scope, ast->condition, Context);
    outf(
        "cmp $0, %%rax\n"
        "je _else_%d\n",
        counter);
    output_ast(cur_scope, function_scope, ast->if_body, Context);
    outf(
        "jmp _post_conditional_%d\n"
        "_else_%d:\n",
        counter, counter);
    /* no else -> no output */
    if (ast->else_body)
      output_ast(cur_scope, function_scope, ast->else_body, Context);
    outf("_post_conditional_%d:\n", counter);
  } else if (ast->ast_type == AST_ternary) {
    counter = get_label_counter();
    output_ast(cur_scope, function_scope, ast->condition, Context);
    outf(
        "cmp $0, %%rax\n"
        "je _exp3_%d\n",
        counter);
    output_ast(cur_scope, function_scope, ast->if_body, Context);
    outf(
        "jmp _post_ternary_%d\n"
        "_exp3_%d:\n",
        counter, counter);
    output_ast(cur_scope, function_scope, ast->else_body, Context);
    outf("_post_ternary_%d:\n", counter);
  } else if (ast->ast_type == AST_compound) {
    /* */
    cur_scope = init_variable_vector();
    for (int i = 0; i < ast->statements->size; i++)
      output_ast(cur_scope, function_scope, ast->statements->arr[i], Context);

    /* clean variables */
    for (int i = 0; i < cur_scope->size; i++) {
      pop_back_variable(function_scope);
    }
    outf("add $%ld, %%rsp\n", 8 * cur_scope->size);
    /* TODO: free scope */

  } else if (ast->ast_type == AST_while) {
    counter = get_label_counter();
    outf(
        "_while_start_%d:\n"
        "_continue_label_%d:\n",
        counter, counter);
    output_ast(cur_scope, function_scope, ast->while_control, Context);
    outf(
        "cmp $0, %%rax\n"
        "je _end_%d\n",
        counter);
    push_back_int(Context, counter);
    output_ast(cur_scope, function_scope, ast->while_body, Context);
    outf(
        "jmp _while_start_%d\n"
        "_end_%d:\n",
        counter, counter);
    pop_back_int(Context);
  } else if (ast->ast_type == AST_do_while) {
    counter = get_label_counter();
    outf("_do_while_start_%d:\n", counter);
    push_back_int(Context, counter);
    output_ast(cur_scope, function_scope, ast->do_while_body, Context);
    outf("_continue_label_%d:\n", counter);
    output_ast(cur_scope, function_scope, ast->do_while_control, Context);
    outf(
        "cmp $0, %%rax\n"
        "jne _do_while_start_%d\n"
        "_end_%d:",
        counter, counter);
    pop_back_int(Context);
  } else if (ast->ast_type == AST_for) {
    counter = get_label_counter();
    VariableVector *for_scope = init_variable_vector();
    if (ast->for_init)
      output_ast(for_scope, function_scope, ast->for_init, Context);
    outf("_for_cond_%d:\n", counter);
    // if empty then keep the loop
    if (ast->for_control)
      output_ast(for_scope, function_scope, ast->for_control, Context);
    else
      outf("mov $1, %%rax\n");
    outf(
        "cmp $0, %%rax\n"
        "je _end_%d\n",
        counter);
    push_back_int(Context, counter);
    output_ast(for_scope, function_scope, ast->for_body, Context);
    outf("_continue_label_%d:\n", counter);
    if (ast->for_post)
      output_ast(for_scope, function_scope, ast->for_post, Context);
    outf(
        "jmp _for_cond_%d\n"
        "_end_%d:\n",
        counter, counter);

    for (int i = 0; i < for_scope->size; i++) {
      pop_back_variable(function_scope);
    }
    pop_back_int(Context);
    /* roll back for scope variables */
    outf("add $%ld, %%rsp\n", 8 * for_scope->size);
  } else if (ast->ast_type == AST_break) {
    fail_if(Context->size <= 0);
    outf("jmp _end_%d\n", Context->arr[Context->size - 1]);
  } else if (ast->ast_type == AST_continue) {
    fail_if(Context->size <= 0);
    outf("jmp _continue_label_%d\n", Context->arr[Context->size - 1]);
  } else if (ast->ast_type == AST_NULL) {
    outf("nop\n");
  } else {
    errf("type:%d", ast->ast_type);
    fail();
  }
}

void codegen(ASTVector *vec) {
  output_f = fopen("a.s", "w");
  for (int i = 0; i < vec->size; i++) {
    VariableVector *function_scope = init_variable_vector();
    output_ast(NULL, function_scope, vec->arr[i], NULL);
    outf("\n");
  }
  fclose(output_f);
}