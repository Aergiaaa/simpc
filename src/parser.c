#include <stdbool.h>
#include <stdlib.h>

#include "arena.h"
#include "array.h"
#include "node.h"
#include "parser.h"
#include "tokenizer.h"

NodeExpr *parse_expr(Parser *p, int min_prec);
NodeStmt *parse_stmt(Parser *p);
NodeTerm *parse_term(Parser *p);
NodeScope *parse_scope(Parser *p);
NodeIfPred *parse_if_pred(Parser *p);
NodeExprBinOp *parse_bin_op(Parser *p, Token *op, NodeExpr *left, int prec);

bool is_stmt_exit(Parser *p);
bool is_stmt_let(Parser *p);
bool is_stmt_scope(Parser *p);
bool is_stmt_assign(Parser *p);
NodeStmt *parse_stmt_exit(Parser *p, NodeStmt *stmt);
NodeStmt *parse_stmt_let(Parser *p, NodeStmt *stmt);
NodeStmt *parse_stmt_scope(Parser *p, NodeStmt *stmt);
NodeStmt *parse_stmt_assign(Parser *p, NodeStmt *stmt);
NodeStmt *parse_stmt_if(Parser *p, NodeStmt *stmt);

void error_expected(Parser *p, const char *msg);

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

NodeExpr *parse_expr(Parser *p, int min_prec) {

  NodeTerm *term_left = parse_term(p);
  if (term_left == NULL) {
    return NULL;
  }

  NodeExpr *left = alloc(p->allocator, sizeof(NodeExpr));
  left->type = EXPR_TERM;
  left->term = term_left;

  while (true) {
    Token *op = peek_token(p);
    if (op == NULL || !is_bin_op(op->type) || bin_prec(op->type) < min_prec)
      break;
    consume_token(p);

    NodeExpr *expr = alloc(p->allocator, sizeof(NodeExpr));
    expr->type = EXPR_BIN_OP;
    expr->bin_op = parse_bin_op(p, op, left, bin_prec(op->type));

    left = expr;
  }

  return left;
};

NodeStmt *parse_stmt(Parser *p) {
  NodeStmt *stmt = alloc(p->allocator, sizeof(NodeStmt));

  while (peek_token(p) != NULL) {
    if (is_stmt_exit(p)) {
      return parse_stmt_exit(p, stmt);
    }
    if (is_stmt_let(p)) {
      return parse_stmt_let(p, stmt);
    }
    if (try_consume_token(p, IF)) {
      return parse_stmt_if(p, stmt);
    }
    if (is_stmt_assign(p)) {
      return parse_stmt_assign(p, stmt);
    }
    if (is_stmt_scope(p)) {
      return parse_stmt_scope(p, stmt);
    }

    error_expected(p, "statement");
  }
  return stmt;
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
  case LPAREN:;
    {
      NodeTermParen *paren = alloc(p->allocator, sizeof(NodeTermParen));
      NodeExpr *expr = parse_expr(p, 0);
      if (expr == NULL) {
        error_expected(p, "expression");
      }

      try_consume_token_with_err(p, RPAREN);
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

NodeScope *parse_scope(Parser *p) {
  if (try_consume_token(p, LBRACE) == NULL)
    return NULL;

  NodeScope *scope = alloc(p->allocator, sizeof(NodeScope));
  scope->stmt = initArray(8, sizeof(NodeStmt));

  while (peek_token(p) != NULL && peek_token(p)->type != RBRACE) {
    NodeStmt *inner_stmt = parse_stmt(p);
    if (inner_stmt == NULL) {
      printf("invalid statement in scope\n");
      exit(EXIT_FAILURE);
    }

    appendArray(&scope->stmt, inner_stmt);
  }

  try_consume_token_with_err(p, RBRACE);

  return scope;
}

NodeIfPred *parse_if_pred(Parser *p) {
  if (try_consume_token(p, ELIF)) {
    try_consume_token_with_err(p, LPAREN);

    NodeIfPredElif *pred_elif = alloc(p->allocator, sizeof(NodeIfPredElif));

    NodeExpr *expr = parse_expr(p, 0);
    if (expr == NULL) {
      error_expected(p, "expression");
    }
    pred_elif->expr = expr;

    try_consume_token_with_err(p, RPAREN);

    NodeScope *scope = parse_scope(p);
    if (scope == NULL) {
      error_expected(p, "scope");
    }
    pred_elif->scope = scope;

    pred_elif->pred = parse_if_pred(p);

    NodeIfPred *pred = alloc(p->allocator, sizeof(NodeIfPred));
    pred->type = IF_PRED_ELIF;
    pred->pred_elif = pred_elif;
    return pred;
  }

  if (try_consume_token(p, ELSE)) {
    NodeIfPredElse *pred_else = alloc(p->allocator, sizeof(NodeIfPredElse));

    NodeScope *scope = parse_scope(p);
    if (scope == NULL) {
      error_expected(p, "scope");
    }
    pred_else->scope = scope;

    NodeIfPred *pred = alloc(p->allocator, sizeof(NodeIfPred));
    pred->type = IF_PRED_ELSE;
    pred->pred_else = pred_else;
    return pred;
  }

  return NULL;
}

NodeExprBinOp *parse_bin_op(Parser *p, Token *op, NodeExpr *left, int prec) {
  NodeExpr *right = parse_expr(p, prec + 1);
  if (right == NULL) {
    error_expected(p, "expression");
  }

  NodeExprBinOp *bin_op = alloc(p->allocator, sizeof(NodeExprBinOp));
  switch (op->type) {
  case PLUS:;
    BinExprAdd *add = alloc(p->allocator, sizeof(BinExprAdd));

    add->left = left;
    add->right = right;

    bin_op->type = BIN_ADD_EXPR;
    bin_op->add = add;
    break;
  case DASH:;
    BinExprSub *sub = alloc(p->allocator, sizeof(BinExprSub));
    sub->left = left;
    sub->right = right;

    bin_op->type = BIN_SUB_EXPR;
    bin_op->sub = sub;
    break;
  case STAR:;
    BinExprMul *mul = alloc(p->allocator, sizeof(BinExprMul));
    mul->left = left;
    mul->right = right;

    bin_op->type = BIN_MUL_EXPR;
    bin_op->mul = mul;
    break;
  case FSLASH:;
    BinExprDiv *div = alloc(p->allocator, sizeof(BinExprDiv));
    div->left = left;
    div->right = right;

    bin_op->type = BIN_DIV_EXPR;
    bin_op->div = div;
    break;
  default:
    printf("unknown operator\n");
    exit(EXIT_FAILURE);
  }

  return bin_op;
}

NodeStmt *parse_stmt_exit(Parser *p, NodeStmt *stmt) {
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

  try_consume_token_with_err(p, RPAREN);
  try_consume_token_with_err(p, SEMICOL);

  stmt->type = STMT_EXIT;
  stmt->exit = exit_stmt;

  return stmt;
}

NodeStmt *parse_stmt_let(Parser *p, NodeStmt *stmt) {
  consume_token(p); // consume "let"

  NodeStmtLet *let = alloc(p->allocator, sizeof(NodeStmtLet));
  let->ident = consume_token(p); // consume "ident"

  consume_token(p); // consume "="

  NodeExpr *expr = parse_expr(p, 0); // consume "expr"
  if (expr != NULL) {
    let->expr = expr;
  } else {
    printf("invalid expression");
    exit(EXIT_FAILURE);
  }

  try_consume_token_with_err(p, SEMICOL);

  stmt->type = STMT_LET;
  stmt->let = let;
  return stmt;
}

NodeStmt *parse_stmt_scope(Parser *p, NodeStmt *stmt) {
  NodeScope *scope = parse_scope(p);
  if (scope == NULL) {
    printf("invalid scope");
    exit(EXIT_FAILURE);
  }

  stmt->type = STMT_SCOPE;
  stmt->scope = scope;
  return stmt;
}

NodeStmt *parse_stmt_assign(Parser *p, NodeStmt *stmt) {
  NodeStmtAssign *assign = alloc(p->allocator, sizeof(NodeStmtAssign));
  assign->ident = consume_token(p);

  consume_token(p);

  NodeExpr *expr = parse_expr(p, 0);
  if (expr == NULL) {
    error_expected(p, "expression");
  }
  assign->expr = expr;

  try_consume_token_with_err(p, SEMICOL);

  stmt->type = STMT_ASSIGN;
  stmt->assign = assign;
  return stmt;
}

NodeStmt *parse_stmt_if(Parser *p, NodeStmt *stmt) {
  try_consume_token_with_err(p, LPAREN);

  NodeStmtIf *if_stmt = alloc(p->allocator, sizeof(NodeStmtIf));

  NodeExpr *expr = parse_expr(p, 0);
  if (expr == NULL) {
    printf("invalid expression");
    exit(EXIT_FAILURE);
  }
  if_stmt->expr = expr;

  try_consume_token_with_err(p, RPAREN);
  NodeScope *scope = parse_scope(p);
  if (scope == NULL) {
    error_expected(p, "scope");
  }
  if_stmt->scope = scope;

  if_stmt->pred = parse_if_pred(p);

  stmt->type = STMT_IF;
  stmt->if_stmt = if_stmt;
  return stmt;
}

void error_expected(Parser *p, const char *msg) {
  Token *tok = peek_token_offset(p, -1);
  int line = tok->line + ((tok != NULL) ? +0 : -1);
  printf("[parse error] expected `%s` on line %d\n", msg, line);
  exit(EXIT_FAILURE);
}

bool is_stmt_exit(Parser *p) {
  return peek_token(p)->type == EXIT && peek_token_offset(p, 1) != NULL &&
         peek_token_offset(p, 1)->type == LPAREN;
}

bool is_stmt_let(Parser *p) {
  return peek_token(p)->type == LET && peek_token_offset(p, 1) != NULL &&
         peek_token_offset(p, 1)->type == IDENT &&
         peek_token_offset(p, 2) != NULL &&
         peek_token_offset(p, 2)->type == EQUAL;
}

bool is_stmt_assign(Parser *p) {
  return peek_token(p)->type == IDENT && peek_token_offset(p, 1) != NULL &&
         peek_token_offset(p, 1)->type == EQUAL;
}

bool is_stmt_scope(Parser *p) { return peek_token(p)->type == LBRACE; }
