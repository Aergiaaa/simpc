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
        Token tok = {.type = EXIT, .str = "exit"};
        appendArray(a, &tok);

        len = 0;
        buf[0] = '\0';

        continue;
      } else {
        printf("Unknown WORD");
        exit(EXIT_FAILURE);
      }
    }

    // if buffer is a number
    else if (isdigit(peek_char(t))) {
      buf[len++] = consume_char(t);
      while (peek_char(t) != '\0' && isdigit(peek_char(t))) {
        buf[len++] = consume_char(t);
      }
      buf[len] = '\0';
      Token tok = {.type = INT_LIT, .str = strdup(buf)};
      appendArray(a, &tok);

      len = 0;
      buf[0] = '\0';

      continue;
    }

    // if buf is semicolon
    else if (peek_char(t) == ';') {
      consume_char(t);

      Token tok = {.type = SEMICOL, .str = ";"};
      appendArray(a, &tok);
      continue;
    }

    else if (isspace(peek_char(t))) {
      consume_char(t);
      continue;
    }

    else {
      printf("Unknown WORD");
      exit(EXIT_FAILURE);
    }
  }

  t->curr_index = 0;
}

void freeToken(void *elem) {
  Token *t = (Token *)elem;
  if (t->type == INT_LIT)
    free((char *)t->str);
}
