#pragma once

#include "array.h"
#include "tokenizer.h"

typedef struct NodeTermIntLit {
  Token *lit;
} NodeTermIntLit;

typedef struct NodeTermIdent {
  Token *ident;
} NodeTermIdent;

typedef enum NodeTermType {
  TERM_INT_LIT,
  TERM_IDENT,
} NodeTermType;

typedef struct NodeTerm {
  NodeTermType type;
  union {
    NodeTermIntLit *int_lit;
    NodeTermIdent *ident;
  };
} NodeTerm;

typedef enum NodeExprType {
  EXPR_TERM,
  EXPR_BIN_OP,
} NodeExprType;

typedef struct NodeExpr {
  NodeExprType type;
  union {
    NodeTerm *term;
    struct NodeExprBinOp *bin_op;
  };
} NodeExpr;

typedef struct BinExprAdd {
  NodeExpr *left;
  NodeExpr *right;
} BinExprAdd;

typedef struct BinExprMul {
  NodeExpr *left;
  NodeExpr *right;
} BinExprMul;

typedef enum BinExprType {
  BIN_ADD_EXPR,
  BIN_MUL_EXPR,
} BinExprType;

typedef struct NodeExprBinOp {
  BinExprType type;
  union {
    BinExprAdd *add;
    BinExprMul *mul;
  };
} NodeExprBinOp;

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
