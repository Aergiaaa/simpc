#pragma once

#include "hashmap.h"

#define MAX_SCOPE_DEPTH 64

typedef struct ScopeStack {
  HashMap maps[MAX_SCOPE_DEPTH];
  size_t depth;
  size_t scope_base[MAX_SCOPE_DEPTH];
} ScopeStack;

static inline ScopeStack initScopeStack() {
  ScopeStack s;
  s.depth = 0;
  s.maps[0] = initHashMap();
  return s;
}

static inline void freeScopeStack(ScopeStack *s) {
  for (size_t i = 0; i <= s->depth; i++)
    freeHashMap(&s->maps[i]);
}

// called when entering scope
static inline void scope_push(ScopeStack *s, size_t curr_stack_size) {
  s->depth++;
  s->maps[s->depth] = initHashMap();
  s->scope_base[s->depth] = curr_stack_size;
}

// called when exitting scope
static inline size_t scope_pop(ScopeStack *s, size_t curr_stack_size) {
  freeHashMap(&s->maps[s->depth]);
  size_t vars_to_pop = curr_stack_size - s->scope_base[s->depth];
  s->depth--;
  return vars_to_pop;
}

static inline void scope_insert(ScopeStack *s, const char *key, int stack_pos) {
  hashmap_insert(&s->maps[s->depth], key, stack_pos);
}

static inline HashEntry *scope_get(ScopeStack *s, const char *key) {
  for (int i = s->depth; i >= 0; i--) {
    HashEntry *entry = hashmap_get(&s->maps[i], key);
    if (entry != NULL)
      return entry;
  }

  return NULL;
}
