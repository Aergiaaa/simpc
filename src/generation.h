#pragma once

#include "hashmap.h"
#include "node.h"

typedef struct Generator {
  NodeProg *root;
  char *result;
  HashMap vars;
  size_t stack_size;
} Generator;

static inline Generator initGenerator(NodeProg *root) {
  return (Generator){
      .root = root,
      .result = (char *)malloc(BUF_SIZE * 10),
      .vars = initHashMap(),
      .stack_size = 0,
  };
};

char *generate(Generator *g);
