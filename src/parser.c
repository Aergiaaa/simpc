#include <stdbool.h>
#include <stdlib.h>

#include "arena.h"
#include "array.h"
#include "node.h"
#include "parser.h"
#include "tokenizer.h"

NodeExpr *parse_expr(Parser *p, int min_prec);

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
  case LPAREN:;
    {
      consume_token(p);
      NodeTermParen *paren = alloc(p->allocator, sizeof(NodeTermParen));
      NodeExpr *expr = parse_expr(p, 0);
      if (expr == NULL) {
        printf("expected expression");
        exit(EXIT_FAILURE);
      }

      try_consume_token_with_err(p, RPAREN, "Expected ')'\n");
      NodeTerm *term = alloc(p->allocator, sizeof(NodeTerm));

      paren->expr = expr;
      term->type = TERM_PAREN;
      term->paren = paren;
      return term;
    }
  default:
    return NULL;
  }
}

NodeExpr *parse_expr(Parser *p, int min_prec) {

  NodeTerm *term_left = parse_term(p);
  if (term_left == NULL) {
    return NULL;
  }

  NodeExpr *expr_left_root = alloc(p->allocator, sizeof(NodeExpr));
  expr_left_root->type = EXPR_TERM;
  expr_left_root->term = term_left;

  while (true) {
    Token *curr = peek_token(p);
    if (curr == NULL)
      break;

    int prec = bin_prec(curr->type);
    if (!is_bin_op(curr->type) || prec < min_prec) {
      break;
    }
    Token *op = consume_token(p);

    int next_min_prec = prec + 1;
    NodeExpr *expr_right = parse_expr(p, next_min_prec);
    if (expr_right == NULL) {
      printf("unable to parse expr");
      exit(EXIT_FAILURE);
    }

    NodeExpr *expr_left = alloc(p->allocator, sizeof(NodeExpr));
    expr_left->type = expr_left_root->type;
    switch (expr_left_root->type) {
    case EXPR_BIN_OP:
      expr_left->bin_op = expr_left_root->bin_op;
      break;
    case EXPR_TERM:
      expr_left->term = expr_left_root->term;
      break;
    }

    NodeExprBinOp *bin_op = alloc(p->allocator, sizeof(NodeExprBinOp));
    switch (op->type) {
    case ADD:;
      {
        BinExprAdd *add = alloc(p->allocator, sizeof(BinExprAdd));

        add->left = expr_left;
        add->right = expr_right;

        bin_op->type = BIN_ADD_EXPR;
        bin_op->add = add;
        break;
      }
    case SUB:;
      {
        BinExprSub *sub = alloc(p->allocator, sizeof(BinExprSub));
        sub->left = expr_left;
        sub->right = expr_right;

        bin_op->type = BIN_SUB_EXPR;
        bin_op->sub = sub;
        break;
      }
    case MUL:;
      {
        BinExprMul *mul = alloc(p->allocator, sizeof(BinExprMul));
        mul->left = expr_left;
        mul->right = expr_right;

        bin_op->type = BIN_MUL_EXPR;
        bin_op->mul = mul;
        break;
      }
    case DIV:;
      {
        BinExprDiv *div = alloc(p->allocator, sizeof(BinExprDiv));
        div->left = expr_left;
        div->right = expr_right;

        bin_op->type = BIN_DIV_EXPR;
        bin_op->div = div;
        break;
      }
    default:
      break;
    }

    expr_left_root->type = EXPR_BIN_OP;
    expr_left_root->bin_op = bin_op;
  }

  return expr_left_root;
};

NodeStmt *parse_stmt(Parser *p) {
  NodeStmt *stmt = alloc(p->allocator, sizeof(NodeStmt));

  while (peek_token(p) != NULL) {
    if (peek_token(p)->type == EXIT && peek_token_offset(p, 1) != NULL &&
        peek_token_offset(p, 1)->type == LPAREN) {
      consume_token(p); // consume the "exit"
      consume_token(p); // consume the "("

      NodeStmtExit *exit_stmt = alloc(p->allocator, sizeof(NodeStmtExit));

      NodeExpr *expr = parse_expr(p, 0);
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

      NodeExpr *expr = parse_expr(p, 0); // consume 'expr'
      if (expr != NULL) {
        let->expr = expr;
      } else {
        printf("invalid expression");
        exit(EXIT_FAILURE);
      }

      try_consume_token_with_err(p, SEMICOL, "expected ';'\n");

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
