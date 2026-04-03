#pragma once

#include "tokenizer.h"

typedef struct NodeExpr {
  Token *lit;
} NodeExpr;

typedef struct NodeExit {
  NodeExpr *expr;
} NodeExit;
