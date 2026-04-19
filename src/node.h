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

typedef struct NodeIfPredElif {
  NodeExpr *expr;
  NodeScope *scope;
  struct NodeIfPred *pred;
} NodeIfPredElif;

typedef struct NodeIfPredElse {
  NodeScope *scope;
} NodeIfPredElse;

typedef enum NodeIfPredType {
  IF_PRED_ELIF,
  IF_PRED_ELSE,
} NodeIfPredType;

typedef struct NodeIfPred {
  NodeIfPredType type;
  union {
    NodeIfPredElif *pred_elif;
    NodeIfPredElse *pred_else;
  };
} NodeIfPred;

typedef struct NodeStmtIf {
  NodeExpr *expr;
  NodeScope *scope;
  NodeIfPred *pred;
} NodeStmtIf;

typedef struct NodeStmtAssign {
  Token *ident;
  NodeExpr *expr;
} NodeStmtAssign;

typedef enum NodeStmtType {
  STMT_EXIT,
  STMT_LET,
  STMT_SCOPE,
  STMT_IF,
  STMT_ASSIGN,
} NodeStmtType;

typedef struct NodeStmt {
  NodeStmtType type;
  union {
    NodeStmtExit *exit;
    NodeStmtLet *let;
    NodeScope *scope;
    NodeStmtIf *if_stmt;
    NodeStmtAssign *assign;
  };
} NodeStmt;

typedef struct NodeProg {
  Array stmt;
} NodeProg;
