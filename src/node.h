#pragma once

#include "tokenizer.h"

typedef struct NodeExprIntLit {
  Token *lit;
} NodeExprIntLit;

typedef struct NodeExprIdent {
  Token *ident;
} NodeExprIdent;

typedef enum NodeExprType {
  EXPR_INT_LIT,
  EXPR_IDENT,
} NodeExprType;

typedef struct NodeExpr {
  NodeExprType type;
  union {
    NodeExprIdent *ident;
    NodeExprIntLit *int_lit;
  };
} NodeExpr;

typedef struct NodeStmtExit {
  NodeExpr *expr;
} NodeStmtExit;

typedef struct NodeStmtLet {
  Token *ident;
  NodeExpr *expr;
} NodeStmtLet;

typedef enum NodeStmtType {
  STMT_EXIT,
  STMT_LET,
} NodeStmtType;

typedef struct NodeStmt {
  NodeStmtType type;
  union {
    NodeStmtExit *exit;
    NodeStmtLet *let;
  };
} NodeStmt;

typedef struct NodeProg {
  Array stmt;
} NodeProg;

static inline void freeNodeExpr(NodeExpr *expr) {
  if (expr->type == EXPR_INT_LIT)
    free(expr->int_lit);
  else if (expr->type == EXPR_IDENT)
    free(expr->ident);
  free(expr);
}

static inline void freeNodeStmtExit(NodeStmtExit *node) {
  freeNodeExpr(node->expr);
  free(node);
}
