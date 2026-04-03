#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tokenization.h"

void tokenize(Array *a, char buf[BUF_SIZE]) {
  char c[BUF_SIZE];
  int len = 0;

  for (int i = 0; buf[i] != '\0'; i++) {
    // if buffer == '[A-Za-z]'
    if (isalpha(buf[i])) {
      c[len++] = buf[i];
      i++;

      while (isalnum(buf[i])) {
        c[len++] = buf[i];
        i++;
      }
      c[len] = '\0';
      i--;

      if (strcmp(c, "exit") == 0) {
        Token t = {.type = EXIT, .val = "exit"};
        appendToken(a, &t);
        len = 0;
        c[0] = '\0';
        continue;
      } else {
        printf("Unknown WORD");
        exit(EXIT_FAILURE);
      }
    }

    // if buffer == '[0-9]'
    else if (isdigit(buf[i])) {
      c[len++] = buf[i];
      i++;

      while (isdigit(buf[i])) {
        c[len++] = buf[i];
        i++;
      }
      c[len] = '\0';
      i--;

      Token t = {.type = INT_LIT, .val = strdup(c)};
      appendToken(a, &t);
      len = 0;
      c[0] = '\0';
      continue;
    }

    else if (buf[i] == ';') {
      Token t = {.type = SEMICOL, .val = ";"};
      appendToken(a, &t);
      continue;
    }

    // if buffer == ' '
    else if (isspace(buf[i])) {
      continue;
    }

    else {
      printf("Unknown WORD");
      exit(EXIT_FAILURE);
    }
  }
}

char *tokens_to_asm(const Array *a) {
  char *result = malloc(BUF_SIZE * 10);
  result[0] = '\0';
  strcat(result, "global _start\n_start:\n");

  for (int i = 0; i < a->used; i++) {
    Token t = a->tokens[i];
    if (t.type == EXIT) {
      if (i + 1 < a->used && a->tokens[i + 1].type == INT_LIT) {
        if (i + 2 < a->used && a->tokens[i + 2].type == SEMICOL) {
          strcat(result, "		mov rax, 60\n");

          char tmp[BUF_SIZE];
          sprintf(tmp, "		mov rdi, %s\n", a->tokens[i + 1].val);
          strcat(result, tmp);

          strcat(result, "		syscall\n");
        }
      }
    }
  }
  return result;
}
