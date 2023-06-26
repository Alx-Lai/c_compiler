#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "mycc.h"
#include "util.h"
#define MAX_CODE 0x1000
#define DEBUG
char code[MAX_CODE];

int main(int argc, char *argv[]) {
  assert(argc >= 2);
  int fd = open(argv[1], O_RDONLY);
  read(fd, code, MAX_CODE);
  close(fd);
  TokenVector *tokens = init_token_vector();
  lex(tokens, code);
#ifdef DEBUG
  print_lex(tokens);
#endif

  AST *ast = parse_ast(tokens);
#ifdef DEBUG
  print_ast(ast);
#endif
  /* output assembly */
  codegen(ast);
}
