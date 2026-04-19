#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "hashmap.h"
#include "tokenizer.h"

void token_singular_char(Tokenizer *t, Array *a, char c, int line_count);
void try_remove_comment(Tokenizer *t, char next_char);

static HashMap builtin_map;
static bool builtin_map_init = false;
static void init_builtin_map() {
  builtin_map = initHashMap();
  hashmap_insert(&builtin_map, "exit", EXIT);
  hashmap_insert(&builtin_map, "let", LET);
  hashmap_insert(&builtin_map, "if", IF);
  hashmap_insert(&builtin_map, "elif", ELIF);
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

  int line_count = 1;
  while (peek_char(t) != '\0') {
    char c = peek_char(t);

    if (isalpha(c)) {
      buf[len++] = consume_char(t);

      while (isalnum(peek_char(t))) {
        buf[len++] = consume_char(t);
      }
      buf[len] = '\0';

      HashEntry *builtin = hashmap_get(&builtin_map, buf);
      if (builtin != NULL) {
        appendArray(a, &(Token){
                           .type = (TokenType)builtin->stack_pos,
                           .line = line_count,
                           .str = builtin->key,
                           .need_free = false,
                       });
      } else {
        appendArray(a, &(Token){
                           .type = IDENT,
                           .line = line_count,
                           .str = strdup(buf),
                           .need_free = true,
                       });
      }

      len = 0;
      buf[0] = '\0';
      continue;
    }

    // if buffer is a number
    else if (isdigit(c)) {
      buf[len++] = consume_char(t);
      while (isdigit(peek_char(t))) {
        buf[len++] = consume_char(t);
      }
      buf[len] = '\0';
      appendArray(a, &(Token){
                         .type = INT_LIT,
                         .line = line_count,
                         .str = strdup(buf),
                         .need_free = true,
                     });

      len = 0;
      buf[0] = '\0';
      continue;
    }

    if (peek_char(t) == '/') {
      char next_char = peek_char_offset(t, 1);
      if (next_char == '/' || next_char == '*') {
        try_remove_comment(t, next_char);
        continue;
      }
    }

    if (peek_char(t) == '\n') {
      consume_char(t);
      line_count++;
      continue;
    }

    token_singular_char(t, a, c, line_count);
  }

  t->curr_index = 0;
}

void token_singular_char(Tokenizer *t, Array *a, char c, int line_count) {
  if (isspace(c)) {
    consume_char(t);
    return;
  }

  switch (c) {
  case '(':
    consume_char(t);
    appendArray(a, &(Token){
                       .line = line_count,
                       .type = LPAREN,
                       .str = "(",
                       .need_free = false,
                   });
    break;
  case ')':
    consume_char(t);
    appendArray(a, &(Token){
                       .line = line_count,
                       .type = RPAREN,
                       .str = ")",
                       .need_free = false,
                   });
    break;
  case '{':
    consume_char(t);
    appendArray(a, &(Token){
                       .line = line_count,
                       .type = LBRACE,
                       .str = "{",
                       .need_free = false,
                   });
    break;
  case '}':
    consume_char(t);
    appendArray(a, &(Token){
                       .line = line_count,
                       .type = RBRACE,
                       .str = "}",
                       .need_free = false,
                   });
    break;
  case '=':
    consume_char(t);
    appendArray(a, &(Token){
                       .line = line_count,
                       .type = EQUAL,
                       .str = "=",
                       .need_free = false,
                   });
    break;
  case '+':
    consume_char(t);
    appendArray(a, &(Token){
                       .line = line_count,
                       .type = PLUS,
                       .str = "+",
                       .need_free = false,
                   });
    break;
  case '-':
    consume_char(t);
    appendArray(a, &(Token){
                       .line = line_count,
                       .type = DASH,
                       .str = "-",
                       .need_free = false,
                   });
    break;
  case '*':
    consume_char(t);
    appendArray(a, &(Token){
                       .line = line_count,
                       .type = STAR,
                       .str = "*",
                       .need_free = false,
                   });
    break;
  case '/':
    consume_char(t);
    appendArray(a, &(Token){
                       .line = line_count,
                       .type = FSLASH,
                       .str = "/",
                       .need_free = false,
                   });
    break;
  case ';':
    consume_char(t);
    appendArray(a, &(Token){
                       .line = line_count,
                       .type = SEMICOL,
                       .str = ";",
                       .need_free = false,
                   });
    break;
  default:
    printf("invalid token: %c\n", peek_char(t));
    exit(EXIT_FAILURE);
  }
}

void try_remove_comment(Tokenizer *t, char next_char) {

  if (next_char == '/') {
    consume_char(t);
    consume_char(t);

    while (peek_char(t) != '\0' && peek_char(t) != '\n') {
      consume_char(t);
    }
    consume_char(t);

    return;
  }

  if (next_char == '*') {
    consume_char(t); // '/'
    consume_char(t); // '*'

    int depth = 1;
    while (depth > 0) {
      if (peek_char(t) == '\0') {
        printf("unterminated block comment\n");
        exit(EXIT_FAILURE);
      }
      if (peek_char(t) == '/' && peek_char_offset(t, 1) == '*') {
        consume_char(t);
        consume_char(t);
        depth++;
      } else if (peek_char(t) == '*' && peek_char_offset(t, 1) == '/') {
        consume_char(t);
        consume_char(t);
        depth--;
      } else {
        consume_char(t);
      }
    }
    return;
  }

  return;
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

void freeBuiltinMap() {
  freeHashMap(&builtin_map);
  builtin_map_init = false;
}

void freeToken(void *elem) {
  Token *t = (Token *)elem;
  if (t->need_free)
    free((char *)t->str);
}
