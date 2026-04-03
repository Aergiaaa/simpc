#include "generation.h"

char *generate(Generator *g) {
  char *result = malloc(BUF_SIZE * 10);
  result[0] = '\0';
  strcat(result, "global _start\n_start:\n");

  strcat(result, "		mov rax, 60\n");

  char tmp[BUF_SIZE];
  sprintf(tmp, "		mov rdi, %s\n", g->root->expr->lit->str);
  strcat(result, tmp);

  strcat(result, "		syscall\n");

  return result;
};
