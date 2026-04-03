#pragma once

#include "array.h"
#include "node.h"
#include "tokenizer.h"

typedef struct Parser {
  const Array *tokens;
  size_t len;
  size_t curr_index;
} Parser;

NodeExit *parse(Parser *p);

static inline Parser initParser(Array *tokenArray) {
  return (Parser){
      .tokens = tokenArray, .len = tokenArray->used, .curr_index = 0};
};

// returning current value without removing it
static inline Token *peek_token(Parser *p) {
  if (p->curr_index >= (size_t)p->len)
    return NULL;

  return (Token *)getArray((Array *)p->tokens, p->curr_index);
};

// returning current value and removing it from string
static inline Token *consume_token(Parser *p) {
  if (p->curr_index >= p->len)
    return NULL;
  return (Token *)getArray((Array *)p->tokens, p->curr_index++);
}
