#pragma once
#include <assert.h>
#include <string.h>

#include "array.h"

#define BUF_SIZE 1024

typedef enum TokenType {
  // binary expr
  // already in precedence
  PLUS = 1,
  DASH,
  STAR,
  FSLASH,

  // builtin
  EXIT,
  RETURN,

  // datatype
  INT_LIT,

  SEMICOL,
  LPAREN,
  RPAREN,
  LBRACE,
  RBRACE,

  // variable allocation
  LET,
  IDENT,
  EQUAL,

  // if else
  IF,
  ELIF,
  ELSE,

} TokenType;

bool is_bin_op(TokenType type);
int bin_prec(TokenType type);

typedef struct Token {
  int line;
  int col;
  TokenType type;
  const char *str;
  bool need_free;
} Token;

void freeBuiltinMap();
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

static inline char peek_char_offset(Tokenizer *t, int offset) {
  if (t->curr_index + offset >= (int)t->len)
    return '\0';

  return t->str[t->curr_index + offset];
};

// returning current value without removing it
static inline char peek_char(Tokenizer *t) { return peek_char_offset(t, 0); };

// returning current value and removing it from string
static inline char consume_char(Tokenizer *t) {
  if (t->curr_index >= t->len)
    return '\0';
  ;
  return t->str[t->curr_index++];
}
