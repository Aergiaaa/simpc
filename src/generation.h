#pragma once

#include "node.h"
#include "scope.h"

typedef struct Generator {
  NodeProg *root;
  char *result;
  ScopeStack scope;
  size_t stack_size;
  int label_count;
} Generator;

static inline Generator initGenerator(NodeProg *root) {
  return (Generator){
      .root = root,
      .result = (char *)malloc(BUF_SIZE * 10),
      .scope = initScopeStack(),
      .stack_size = 0,
      .label_count = 0,
  };
};

char *generate(Generator *g);
