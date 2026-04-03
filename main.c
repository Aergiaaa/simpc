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

  fseek(f, 0, SEEK_END);
  size_t fsize = ftell(f);
  rewind(f);

  char *src = malloc(fsize + 1);
  fread(src, 1, fsize, f);
  src[fsize] = '\0';
  fclose(f);

  Tokenizer tokenizer = initTokenizer(src);

  Array tokenlist;
  initArray(&tokenlist, 32, sizeof(Token));

  tokenize(&tokenizer, &tokenlist);

  char *str = tokens_to_asm(&tokenlist);
  freeArray(&tokenlist, freeToken);

  FILE *fres = fopen("res.asm", "w");
  if (fres == NULL) {
    printf("Cannot write to file\n");
    return EXIT_FAILURE;
  }
  fprintf(fres, "%s", str);

  fclose(fres);
  free(str);

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

  return EXIT_SUCCESS;
}
