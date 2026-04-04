#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "tokenizer.h"

void tokenize(Tokenizer *t, Array *a) {
  char buf[BUF_SIZE];
  int len = 0;

  while (peek_char(t) != '\0') {

    // if buffer is a word / [A-Za-Z]
    if (isalpha(peek_char(t))) {
      buf[len++] = consume_char(t);
      while (peek_char(t) != '\0' && isalnum(peek_char(t))) {
        buf[len++] = consume_char(t);
      }
      buf[len] = '\0';

      if (strcmp(buf, "exit") == 0) {
        appendArray(a, &(Token){.type = EXIT, .str = "exit"});
      } else if (strcmp(buf, "let") == 0) {
        appendArray(a, &(Token){.type = LET, .str = "let"});
      } else {
        appendArray(a, &(Token){.type = IDENT, .str = strdup(buf)});
      }
      len = 0;
      buf[0] = '\0';
      continue;
    }

    // if buffer is a number
    else if (isdigit(peek_char(t))) {
      buf[len++] = consume_char(t);
      while (peek_char(t) != '\0' && isdigit(peek_char(t))) {
        buf[len++] = consume_char(t);
      }
      buf[len] = '\0';
      appendArray(a, &(Token){.type = INT_LIT, .str = strdup(buf)});

      len = 0;
      buf[0] = '\0';

      continue;
    }

    else if (peek_char(t) == '(') {
      consume_char(t);
      appendArray(a, &(Token){.type = LPAREN, .str = "("});
      continue;
    } else if (peek_char(t) == ')') {
      consume_char(t);
      appendArray(a, &(Token){.type = RPAREN, .str = ")"});
      continue;
    }

    // if buf is semicolon
    else if (peek_char(t) == ';') {
      consume_char(t);
      appendArray(a, &(Token){.type = SEMICOL, .str = ";"});
      continue;
    }

    else if (peek_char(t) == '=') {
      consume_char(t);
      appendArray(a, &(Token){.type = EQUAL, .str = "="});
      continue;
    }

    else if (isspace(peek_char(t))) {
      consume_char(t);
      continue;
    }

    else {
      // appendArray(a, &(Token){.type = IDENT, .str = strdup(buf)});
      // len = 0;
      // buf[0] = '\0';
      // consume_char(t);
      printf("Unknown char: %c\n", peek_char(t));
      exit(EXIT_FAILURE);
    }
  }

  t->curr_index = 0;
}

void freeToken(void *elem) {
  Token *t = (Token *)elem;
  if (t->type == INT_LIT || t->type == IDENT)
    free((char *)t->str);
}
