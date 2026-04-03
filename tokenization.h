#pragma once
#include <stddef.h>

#define BUF_SIZE 1024

typedef enum {
  EXIT,
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

void initArray(Array *a, size_t initSize);
void freeArray(Array *a);
void appendToken(Array *a, Token *t);

void tokenize(Array *a, char buf[BUF_SIZE]);
char *tokens_to_asm(const Array *a);
