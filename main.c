#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "mycc.h"
#include "util.h"
#define MAX_CODE 0x1000
#define DEBUG
char code[MAX_CODE];

int main(int argc, char *argv[]) {
  fail_ifn(argc >= 2);
  int fd = open(argv[1], O_RDONLY);
  read(fd, code, MAX_CODE);
  close(fd);
  TokenVector *tokens = init_token_vector();
  lex(tokens, code);
#ifdef DEBUG
  print_lex(tokens);
#endif
  init_AST();
  ASTVector *ast_vec = parse_ast(tokens);
#ifdef DEBUG
  for (int i = 0; i < ast_vec->size; i++) {
    print_ast(ast_vec->arr[i]);
  }
#endif
  /* output assembly */
  codegen(ast_vec);

  /* clean memory */
  free_AST();
}
