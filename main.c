#include <stdio.h>
#include <stdlib.h>

#include "generation.h"
#include "parser.h"

int main(int argc, char **argv) {
  int exit_code = EXIT_SUCCESS;

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

  Array tokenlist;
  initArray(&tokenlist, 32, sizeof(Token));

  Tokenizer tokenizer = initTokenizer(src);

  tokenize(&tokenizer, &tokenlist);

  Parser parser = initParser(&tokenlist);

  NodeExit *tree = parse(&parser);

  if (tree == NULL) {
    printf("no exit statement found");
    exit(EXIT_FAILURE);
  }

  Generator generator = initGenerator(tree);
  char *str = generate(&generator);

  FILE *fres = fopen("res.asm", "w");
  if (fres == NULL) {
    printf("Cannot write to file\n");
    exit_code = EXIT_FAILURE;
    goto freeup;
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

  // free-ing stuff
freeup:
  freeArray(&tokenlist, freeToken);
  free(tree->expr);
  free(tree);
  free(str);
  free(src);

  return exit_code;
}
