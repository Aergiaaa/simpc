#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "hashmap.h"
#include "tokenizer.h"

void token_singular_char(Tokenizer *t, Array *a, char c);

static HashMap builtin_map;
static bool builtin_map_init = false;
static void init_builtin_map() {
  builtin_map = initHashMap();
  hashmap_insert(&builtin_map, "exit", EXIT);
  hashmap_insert(&builtin_map, "let", LET);
  hashmap_insert(&builtin_map, "if", IF);
  hashmap_insert(&builtin_map, "else", ELSE);
  hashmap_insert(&builtin_map, "return", RETURN);
}

void tokenize(Tokenizer *t, Array *a) {
  if (!builtin_map_init) {
    init_builtin_map();
    builtin_map_init = true;
  }

  char buf[BUF_SIZE];
  int len = 0;

  while (peek_char(t) != '\0') {
    char c = peek_char(t);

    if (isalpha(c)) {
      buf[len++] = consume_char(t);
      while (peek_char(t) != '\0' && isalnum(peek_char(t))) {
        buf[len++] = consume_char(t);
      }
      buf[len] = '\0';

      HashEntry *builtin = hashmap_get(&builtin_map, buf);
      if (builtin != NULL) {
        appendArray(a, &(Token){
                           .type = (TokenType)builtin->stack_pos,
                           .str = builtin->key,
                           .need_free = false,
                       });
      } else {
        appendArray(
            a, &(Token){.type = IDENT, .str = strdup(buf), .need_free = true});
      }

      len = 0;
      buf[0] = '\0';
      continue;
    }

    // if buffer is a number
    else if (isdigit(c)) {
      buf[len++] = consume_char(t);
      while (peek_char(t) != '\0' && isdigit(peek_char(t))) {
        buf[len++] = consume_char(t);
      }
      buf[len] = '\0';
      appendArray(
          a, &(Token){.type = INT_LIT, .str = strdup(buf), .need_free = true});

      len = 0;
      buf[0] = '\0';
      continue;
    }

    token_singular_char(t, a, c);
  }

  t->curr_index = 0;
}

bool is_bin_op(TokenType type) {
  switch (type) {
  case PLUS:
  case DASH:
  case STAR:
  case FSLASH:
    return true;
  default:
    return false;
  }
}

int bin_prec(TokenType type) {
  switch (type) {
  case PLUS:
  case DASH:
    return 1;
  case STAR:
  case FSLASH:
    return 2;
  default:
    return 0;
  }
}

void token_singular_char(Tokenizer *t, Array *a, char c) {
  if (isspace(c)) {
    consume_char(t);
    return;
  }

  switch (c) {
  case '(':
    consume_char(t);
    appendArray(a, &(Token){.type = LPAREN, .str = "(", .need_free = false});
    break;
  case ')':
    consume_char(t);
    appendArray(a, &(Token){.type = RPAREN, .str = ")", .need_free = false});
    break;
  case '{':
    consume_char(t);
    appendArray(a, &(Token){.type = LBRACE, .str = "{", .need_free = false});
    break;
  case '}':
    consume_char(t);
    appendArray(a, &(Token){.type = RBRACE, .str = "}", .need_free = false});
    break;
  case '=':
    consume_char(t);
    appendArray(a, &(Token){.type = EQUAL, .str = "=", .need_free = false});
    break;
  case '+':
    consume_char(t);
    appendArray(a, &(Token){.type = PLUS, .str = "+", .need_free = false});
    break;
  case '-':
    consume_char(t);
    appendArray(a, &(Token){.type = DASH, .str = "-", .need_free = false});
    break;
  case '*':
    consume_char(t);
    appendArray(a, &(Token){.type = STAR, .str = "*", .need_free = false});
    break;
  case '/':
    consume_char(t);
    appendArray(a, &(Token){.type = FSLASH, .str = "/", .need_free = false});
    break;
  case ';':
    consume_char(t);
    appendArray(a, &(Token){.type = SEMICOL, .str = ";", .need_free = false});
    break;
  default:
    printf("Unknown char: %c\n", peek_char(t));
    exit(EXIT_FAILURE);
  }
}

void freeBuiltinMap() {
  freeHashMap(&builtin_map);
  builtin_map_init = false;
}

void freeToken(void *elem) {
  Token *t = (Token *)elem;
  if (t->need_free)
    free((char *)t->str);
}
