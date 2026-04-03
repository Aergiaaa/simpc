#pragma once

#include "node.h"
typedef struct Generator {
  NodeExit *root;
} Generator;

static inline Generator initGenerator(NodeExit *root) {
  return (Generator){.root = root};
};

char *generate(Generator *g);
