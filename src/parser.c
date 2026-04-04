#include <stdlib.h>

#include "array.h"
#include "node.h"
#include "parser.h"
#include "tokenizer.h"

NodeExpr *parse_expr(Parser *p) {
  NodeExpr *expr = malloc(sizeof(NodeExpr));

  Token *t = peek_token(p);
  if (t != NULL && t->type == INT_LIT) {
    NodeExprIntLit *int_lit = malloc(sizeof(NodeExprIntLit));
    int_lit->lit = consume_token(p);

    expr->type = EXPR_INT_LIT;
    expr->int_lit = int_lit;

    return expr;
  }

  else if (t != NULL && t->type == IDENT) {
    NodeExprIdent *ident = malloc(sizeof(NodeExprIdent));
    ident->ident = consume_token(p);

    expr->type = EXPR_IDENT;
    expr->ident = ident;

    return expr;
  }

  return NULL;
};

NodeStmt *parse_stmt(Parser *p) {
  NodeStmt *stmt = malloc(sizeof(NodeStmt));

  while (peek_token(p) != NULL) {
    if (peek_token(p)->type == EXIT && peek_token_offset(p, 1) != NULL &&
        peek_token_offset(p, 1)->type == LPAREN) {
      consume_token(p); // consume the "exit"
      consume_token(p); // consume the "("

      NodeStmtExit *exit_stmt = malloc(sizeof(NodeStmtExit));

      NodeExpr *expr = parse_expr(p);
      if (expr != NULL) {
        exit_stmt->expr = expr;
      } else {
        printf("invalid expression\n");
        exit(EXIT_FAILURE);
      }

      Token *rparen = peek_token(p);
      if (rparen != NULL && rparen->type == RPAREN) {
        consume_token(p);
      } else {
        printf("expecting close parenthesis\n");
        exit(EXIT_FAILURE);
      }

      Token *semicol = peek_token(p);
      if (semicol != NULL && semicol->type == SEMICOL) {
        consume_token(p);
      } else {
        printf("expecting ';'\n");
        exit(EXIT_FAILURE);
      }

      stmt->type = STMT_EXIT;
      stmt->exit = exit_stmt;

      return stmt;
    } else if (peek_token(p) != NULL && peek_token(p)->type == LET &&
               peek_token_offset(p, 1) != NULL &&
               peek_token_offset(p, 1)->type == IDENT &&
               peek_token_offset(p, 2) != NULL &&
               peek_token_offset(p, 2)->type == EQUAL) {

      consume_token(p); // consume 'let'

      NodeStmtLet *let = malloc(sizeof(NodeStmtLet));
      let->ident = consume_token(p); // consume 'ident'

      consume_token(p); // consume '='

      NodeExpr *expr = parse_expr(p); // consume 'expr'
      if (expr != NULL) {
        let->expr = expr;
      } else {
        printf("invalid expression");
        exit(EXIT_FAILURE);
      }

      if (peek_token(p) != NULL && peek_token(p)->type == SEMICOL) {
        consume_token(p);
      } else {
        printf("expected ';'");
        exit(EXIT_FAILURE);
      }

      stmt->type = STMT_LET;
      stmt->let = let;
      return stmt;
    } else {
      printf("unexpected statement\n");
      exit(EXIT_FAILURE);
    }
  };
  return stmt;
}

NodeProg *parse(Parser *p) {
  NodeProg *prog = malloc(sizeof(NodeProg));
  prog->stmt = initArray(8, sizeof(NodeStmt));

  while (peek_token(p) != NULL) {
    NodeStmt *stmt = parse_stmt(p);
    if (stmt != NULL) {
      appendArray(&prog->stmt, stmt);
    } else {
      printf("invalid statement\n");
      exit(EXIT_FAILURE);
    }
  };

  return prog;
};
