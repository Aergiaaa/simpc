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
  LPAREN,
  RPAREN,
  LET,
  IDENT,
  EQUAL,
} TokenType;

typedef struct Token {
  TokenType type;
  const char *str;
} Token;

void freeToken(void *elem);

typedef struct Tokenizer {
  const char *str;
  size_t len;
  int curr_index;
} Tokenizer;

void tokenize(Tokenizer *t, Array *tokenArray);

static inline Tokenizer initTokenizer(const char *str) {
  assert(str != NULL);
  return (Tokenizer){
      .str = str,
      .len = strlen(str),
      .curr_index = 0,
  };
};

// returning current value without removing it
static inline char peek_char(Tokenizer *t) {
  if (t->curr_index >= (int)t->len)
    return '\0';

  return t->str[t->curr_index];
};

// returning current value and removing it from string
static inline char consume_char(Tokenizer *t) {
  if (t->curr_index >= t->len)
    return '\0';
  ;
  return t->str[t->curr_index++];
}
