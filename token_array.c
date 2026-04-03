#include <stdio.h>
#include <stdlib.h>

#include "tokenization.h"

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
