#pragma once

#include "array.h"
#include "tokenizer.h"

typedef struct NodeTermIntLit {
  Token *lit;
} NodeTermIntLit;

typedef struct NodeTermIdent {
  Token *ident;
} NodeTermIdent;

typedef struct NodeTermParen {
  struct NodeExpr *expr;
} NodeTermParen;

typedef enum NodeTermType {
  TERM_INT_LIT,
  TERM_IDENT,
  TERM_PAREN,
} NodeTermType;

typedef struct NodeTerm {
  NodeTermType type;
  union {
    NodeTermIntLit *int_lit;
    NodeTermIdent *ident;
    NodeTermParen *paren;
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

typedef struct BinExprSub {
  NodeExpr *left;
  NodeExpr *right;
} BinExprSub;

typedef struct BinExprMul {
  NodeExpr *left;
  NodeExpr *right;
} BinExprMul;

typedef struct BinExprDiv {
  NodeExpr *left;
  NodeExpr *right;
} BinExprDiv;

typedef enum BinExprType {
  BIN_ADD_EXPR,
  BIN_SUB_EXPR,
  BIN_MUL_EXPR,
  BIN_DIV_EXPR
} BinExprType;

typedef struct NodeExprBinOp {
  BinExprType type;
  union {
    BinExprAdd *add;
    BinExprSub *sub;
    BinExprMul *mul;
    BinExprDiv *div;
  };
} NodeExprBinOp;

typedef struct NodeStmtExit {
  NodeExpr *expr;
} NodeStmtExit;

typedef struct NodeStmtLet {
  Token *ident;
  NodeExpr *expr;
} NodeStmtLet;

typedef struct NodeScope {
  Array stmt;
} NodeScope;

typedef struct NodeStmtIf {
  NodeExpr *expr;
  NodeScope *scope;
} NodeStmtIf;

typedef enum NodeStmtType {
  STMT_EXIT,
  STMT_LET,
  STMT_SCOPE,
  STMT_IF,
} NodeStmtType;

typedef struct NodeStmt {
  NodeStmtType type;
  union {
    NodeStmtExit *exit;
    NodeStmtLet *let;
    NodeScope *scope;
    NodeStmtIf *if_stmt;
  };
} NodeStmt;

typedef struct NodeProg {
  Array stmt;
} NodeProg;
