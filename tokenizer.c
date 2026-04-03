#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "tokenization.h"

void tokenize(Tokenizer *t, Array *a) {
  char buf[BUF_SIZE];
  int len = 0;

  while (peek(t) != '\0') {

    // if buffer is a word / [A-Za-Z]
    if (isalpha(peek(t))) {
      buf[len++] = consume(t);
      while (peek(t) != '\0' && isalnum(peek(t))) {
        buf[len++] = consume(t);
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
    else if (isdigit(peek(t))) {
      buf[len++] = consume(t);
      while (peek(t) != '\0' && isdigit(peek(t))) {
        buf[len++] = consume(t);
      }
      buf[len] = '\0';
      Token tok = {.type = INT_LIT, .str = strdup(buf)};
      appendArray(a, &tok);

      len = 0;
      buf[0] = '\0';

      continue;
    }

    // if buf is semicolon
    else if (peek(t) == ';') {
      consume(t);

      Token tok = {.type = SEMICOL, .str = ";"};
      appendArray(a, &tok);
      continue;
    }

    else if (isspace(peek(t))) {
      consume(t);
      continue;
    }

    else {
      printf("Unknown WORD");
      exit(EXIT_FAILURE);
    }
  }

  t->curr_index = 0;
}

char *tokens_to_asm(const Array *a) {
  char *result = malloc(BUF_SIZE * 10);
  result[0] = '\0';
  strcat(result, "global _start\n_start:\n");

  for (int i = 0; i < a->used; i++) {
    Token *t = (Token *)getArray((Array *)a, i);
    if (t->type == EXIT) {
      Token *next = (Token *)getArray((Array *)a, i + 1);
      Token *semi = (Token *)getArray((Array *)a, i + 2);

      if (i + 1 < a->used && next->type == INT_LIT) {
        if (i + 2 < a->used && semi->type == SEMICOL) {
          strcat(result, "		mov rax, 60\n");

          char tmp[BUF_SIZE];
          sprintf(tmp, "		mov rdi, %s\n", next->str);
          strcat(result, tmp);

          strcat(result, "		syscall\n");
        }
      }
    }
  }
  return result;
}

void freeToken(void *elem) {
  Token *t = (Token *)elem;
  if (t->type == INT_LIT)
    free((char *)t->str);
}
