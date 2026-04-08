#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "tokenizer.h"

bool is_bin_op(TokenType type) {
  switch (type) {
  case ADD:
  case SUB:
  case MUL:
  case DIV:
    return true;
  default:
    return false;
  }
}

int bin_prec(TokenType type) {
  switch (type) {
  case ADD:
  case SUB:
    return 1;
  case MUL:
  case DIV:
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
  case '=':
    consume_char(t);
    appendArray(a, &(Token){.type = EQUAL, .str = "=", .need_free = false});
    break;
  case '+':
    consume_char(t);
    appendArray(a, &(Token){.type = ADD, .str = "+", .need_free = false});
    break;
  case '-':
    consume_char(t);
    appendArray(a, &(Token){.type = SUB, .str = "-", .need_free = false});
    break;
  case '*':
    consume_char(t);
    appendArray(a, &(Token){.type = MUL, .str = "*", .need_free = false});
    break;
  case '/':
    consume_char(t);
    appendArray(a, &(Token){.type = DIV, .str = "/", .need_free = false});
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

void tokenize(Tokenizer *t, Array *a) {
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

      if (strcmp(buf, "exit") == 0) {
        appendArray(a,
                    &(Token){.type = EXIT, .str = "exit", .need_free = false});
      } else if (strcmp(buf, "let") == 0) {
        appendArray(a, &(Token){.type = LET, .str = "let", .need_free = false});
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

void freeToken(void *elem) {
  Token *t = (Token *)elem;
  if (t->need_free)
    free((char *)t->str);
}
