#include <stdlib.h>

#include "node.h"
#include "parser.h"
#include "tokenizer.h"

NodeExpr *parse_expr(Parser *p) {
  Token *t = peek_token(p);
  if (t != NULL && t->type == INT_LIT) {
    NodeExpr *expr = malloc(sizeof(NodeExpr));
    expr->lit = consume_token(p);
    return expr;
  }

  return NULL;
};

NodeExit *parse(Parser *p) {
  NodeExit *exit_node = malloc(sizeof(NodeExit));

  while (peek_token(p) != NULL) {
    if (peek_token(p)->type == EXIT) {
      consume_token(p);

      NodeExpr *expr = parse_expr(p);
      if (expr != NULL) {
        exit_node->expr = expr;
      } else {
        printf("invalid expression");
        exit(EXIT_FAILURE);
      }

      Token *semicol = peek_token(p);
      if (semicol != NULL && semicol->type == SEMICOL) {
        consume_token(p);
      } else {
        printf("invalid expression");
        exit(EXIT_FAILURE);
      }
    } else {
      printf("unexpected token");
      exit(EXIT_FAILURE);
    }
  };
  p->curr_index = 0;

  return exit_node;
};
