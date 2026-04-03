#include <stdio.h>
#include <stdlib.h>

#include "tokenization.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Incorrect usage, Correct usage is...\n");
    printf("simp <file.sm>\n");
    return EXIT_FAILURE;
  }

  FILE *f = fopen(argv[1], "r");
  if (f == NULL) {
    printf("Cannot Read the file\n");
    return EXIT_FAILURE;
  }

  Array tokenlist;
  initArray(&tokenlist, 32);

  char buf[BUF_SIZE];
  while (fgets(buf, BUF_SIZE, f) != NULL) {
    tokenize(&tokenlist, buf);
  }
  char *str = tokens_to_asm(&tokenlist);

  FILE *fres = fopen("res.asm", "w");
  if (fres == NULL) {
    printf("Cannot write to file\n");
    return EXIT_FAILURE;
  }
  fprintf(fres, "%s", str);
  fclose(fres);

  int ret = system("nasm -felf64 res.asm -o res.o");
  if (ret != 0) {
    printf("nasm failed\n");
    return EXIT_FAILURE;
  }

  ret = system("ld res.o -o res");
  if (ret != 0) {
    printf("ld failed\n");
    return EXIT_FAILURE;
  }
  // free all
  freeArray(&tokenlist);
  fclose(f);
  free(str);

  return EXIT_SUCCESS;
}
