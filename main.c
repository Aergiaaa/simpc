#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1024

typedef enum {
  RETURN,
  INT_LIT,
  SEMICOL,

} TokenType;

typedef struct Token {
  TokenType type;
  char *val;
} Token;

typedef struct Array {
  Token *tokens;
  size_t used;
  size_t size;
} Array;

void initArray(Array *a, size_t initSize) {
  a->tokens = malloc(initSize * sizeof(Token));
  if (a->tokens == NULL) {
    printf("Memory allocation failed\n");
    exit(EXIT_FAILURE);
  }

  a->used = 0;
  a->size = initSize;
}

void freeArray(Array *a) {
  for (size_t i = 0; i < a->used; i++) {
    if (a->tokens[i].type == INT_LIT) {
      free(a->tokens[i].val);
    }
  }

  free(a->tokens);
  a->tokens = NULL;
  a->used = 0;
  a->size = 0;
}

void appendToken(Array *a, Token *t) {
  if (a->used == a->size) {
    a->size *= 2;
    a->tokens = realloc(a->tokens, a->size * sizeof(Token));
    if (a->tokens == NULL) {
      printf("Memory reallocation failed\n");
      exit(EXIT_FAILURE);
    }
  }

  a->tokens[a->used++] = *t;
}

void tokenize(Array *a, char buf[BUF_SIZE]) {
  char c[BUF_SIZE];
  int len = 0;

  for (int i = 0; buf[i] != '\0'; i++) {
    // if buffer == '[A-Za-z]'
    if (isalpha(buf[i])) {
      c[len++] = buf[i];
      i++;

      while (isalnum(buf[i])) {
        c[len++] = buf[i];
        i++;
      }
      c[len] = '\0';
      i--;

      if (strcmp(c, "return") == 0) {
        Token t = {.type = RETURN, .val = "return"};
        appendToken(a, &t);
        len = 0;
        c[0] = '\0';
        continue;
      } else {
        printf("Unknown WORD");
        exit(EXIT_FAILURE);
      }
    }

    // if buffer == '[0-9]'
    else if (isdigit(buf[i])) {
      c[len++] = buf[i];
      i++;

      while (isdigit(buf[i])) {
        c[len++] = buf[i];
        i++;
      }
      c[len] = '\0';
      i--;

      Token t = {.type = INT_LIT, .val = strdup(c)};
      appendToken(a, &t);
      len = 0;
      c[0] = '\0';
      continue;
    }

    else if (buf[i] == ';') {
      Token t = {.type = SEMICOL, .val = ";"};
      appendToken(a, &t);
      continue;
    }

    // if buffer == ' '
    else if (isspace(buf[i])) {
      continue;
    }

    else {
      printf("Unknown WORD");
      exit(EXIT_FAILURE);
    }
  }
}

char *tokens_to_asm(const Array *a) {
  char *result = malloc(BUF_SIZE * 10);
  result[0] = '\0';
  strcat(result, "global _start\n_start:\n");

  for (int i = 0; i < a->used; i++) {
    Token t = a->tokens[i];
    if (t.type == RETURN) {
      if (i + 1 < a->used && a->tokens[i + 1].type == INT_LIT) {
        if (i + 2 < a->used && a->tokens[i + 2].type == SEMICOL) {
          strcat(result, "		mov rax, 60\n");

          char tmp[BUF_SIZE];
          sprintf(tmp, "		mov rdi, %s\n", a->tokens[i + 1].val);
          strcat(result, tmp);

          strcat(result, "		syscall\n");
        }
      }
    }
  }
  return result;
}

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
  printf("%s", str);

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
