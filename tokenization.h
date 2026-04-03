#pragma once
#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "array.h"

#define BUF_SIZE 1024

typedef enum {
  EXIT,
  RETURN,
  INT_LIT,
  SEMICOL,

} TokenType;

typedef struct Token {
  TokenType type;
  const char *str;
} Token;

void freeToken(void *elem);
char *tokens_to_asm(const Array *a);

typedef struct Tokenizer {
  const char *str;
  size_t len;
  int curr_index;
} Tokenizer;

static inline Tokenizer initTokenizer(const char *str) {
  assert(str != NULL);
  return (Tokenizer){.str = str, .len = strlen(str), .curr_index = 0};
};

void tokenize(Tokenizer *t, Array *a);

// returning current value without removing it
static inline char peek(Tokenizer *t) {
  if (t->curr_index >= (int)t->len)
    return '\0';

  return t->str[t->curr_index];
};

static inline char peek_ahead(Tokenizer *t, int ahead) {
  if (t->curr_index + ahead >= (int)t->len)
    return '\0';
  return t->str[t->curr_index + ahead];
}

// returning current value and removing it from string
static inline char consume(Tokenizer *t) { return t->str[t->curr_index++]; }
