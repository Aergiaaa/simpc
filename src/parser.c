#include <stdlib.h>

#include "arena.h"
#include "array.h"
#include "node.h"
#include "parser.h"
#include "tokenizer.h"

NodeExpr *parse_expr(Parser *p);

NodeExprBinOp *parse_expr_bin(Parser *p) {
  NodeExpr *left = parse_expr(p);
  if (left == NULL)
    return NULL;

  NodeExpr *right = parse_expr(p);
  if (right == NULL) {
    printf("expected expression");
    exit(EXIT_FAILURE);
  }

  NodeExprBinOp *bin = alloc(p->allocator, sizeof(NodeExprBinOp));
  if (peek_token(p)) {
    switch (peek_token(p)->type) {
    case ADD:;
      {
        BinExprAdd *add_expr = alloc(p->allocator, sizeof(BinExprAdd));
        add_expr->left = left;
        consume_token(p);
        add_expr->right = right;
        consume_token(p);

        bin->type = BIN_ADD_EXPR;
        bin->add = add_expr;
        return bin;
      }
    case MUL:
    default:
      break;
    }
  }

  printf("unsupported binary expression");
  exit(EXIT_FAILURE);
}

NodeTerm *parse_term(Parser *p) {
  Token *t = peek_token(p);
  if (t == NULL) {
    return NULL;
  }

  switch (t->type) {
  case INT_LIT:;
    {
      NodeTermIntLit *int_lit = alloc(p->allocator, sizeof(NodeTermIntLit));
      int_lit->lit = consume_token(p);

      NodeTerm *term = alloc(p->allocator, sizeof(NodeTerm));
      term->type = TERM_INT_LIT;
      term->int_lit = int_lit;
      return term;
    }
  case IDENT:;
    {
      NodeTermIdent *ident = alloc(p->allocator, sizeof(NodeTermIdent));
      ident->ident = consume_token(p);

      NodeTerm *term = alloc(p->allocator, sizeof(NodeTerm));
      term->type = TERM_IDENT;
      term->ident = ident;
      return term;
    }
  default:
    return NULL;
  }
}

NodeExpr *parse_expr(Parser *p) {
  NodeTerm *term = parse_term(p);
  if (term != NULL) {
    NodeExpr *expr = alloc(p->allocator, sizeof(NodeExpr));

    if (peek_token(p)) {
      switch (peek_token(p)->type) {
      case ADD:;
        {
          NodeExprBinOp *bin = alloc(p->allocator, sizeof(NodeExprBinOp));
          BinExprAdd *add_expr = alloc(p->allocator, sizeof(BinExprAdd));

          NodeExpr *left = alloc(p->allocator, sizeof(NodeExpr));
          left->type = EXPR_TERM;
          left->term = term;
          add_expr->left = left;
          consume_token(p);

          NodeExpr *right = parse_expr(p);
          if (right == NULL) {
            printf("expected expression");
            exit(EXIT_FAILURE);
          }
          add_expr->right = right;

          bin->type = BIN_ADD_EXPR;
          bin->add = add_expr;

          expr->type = EXPR_BIN_OP;
          expr->bin_op = bin;
          return expr;
        }
      default:
        break;
      }
    }

    expr->type = EXPR_TERM;
    expr->term = term;
    return expr;
  }

  NodeExprBinOp *bin = parse_expr_bin(p);
  if (bin != NULL) {
    NodeExpr *expr = alloc(p->allocator, sizeof(NodeExpr));
    expr->type = EXPR_BIN_OP;
    expr->bin_op = bin;
    return expr;
  }

  return NULL;
};

NodeStmt *parse_stmt(Parser *p) {
  NodeStmt *stmt = alloc(p->allocator, sizeof(NodeStmt));

  while (peek_token(p) != NULL) {
    if (peek_token(p)->type == EXIT && peek_token_offset(p, 1) != NULL &&
        peek_token_offset(p, 1)->type == LPAREN) {
      consume_token(p); // consume the "exit"
      consume_token(p); // consume the "("

      NodeStmtExit *exit_stmt = alloc(p->allocator, sizeof(NodeStmtExit));

      NodeExpr *expr = parse_expr(p);
      if (expr != NULL) {
        exit_stmt->expr = expr;
      } else {
        printf("invalid expression\n");
        exit(EXIT_FAILURE);
      }

      try_consume_token_with_err(p, RPAREN, "expecting ')'");
      try_consume_token_with_err(p, SEMICOL, "expecting ';'");

      stmt->type = STMT_EXIT;
      stmt->exit = exit_stmt;

      return stmt;
    } else if (peek_token(p) != NULL && peek_token(p)->type == LET &&
               peek_token_offset(p, 1) != NULL &&
               peek_token_offset(p, 1)->type == IDENT &&
               peek_token_offset(p, 2) != NULL &&
               peek_token_offset(p, 2)->type == EQUAL) {

      consume_token(p); // consume 'let'

      NodeStmtLet *let = alloc(p->allocator, sizeof(NodeStmtLet));
      let->ident = consume_token(p); // consume 'ident'

      consume_token(p); // consume '='

      NodeExpr *expr = parse_expr(p); // consume 'expr'
      if (expr != NULL) {
        let->expr = expr;
      } else {
        printf("invalid expression");
        exit(EXIT_FAILURE);
      }

      try_consume_token_with_err(p, SEMICOL, "expected ';'");

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
  NodeProg *prog = alloc(p->allocator, sizeof(NodeProg));
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
