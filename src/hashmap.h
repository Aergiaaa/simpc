#pragma once
#include <stdlib.h>
#include <string.h>

#define HASHMAP_CAP 1024

typedef struct HashEntry {
  char *key;     // variable name
  int stack_pos; // position on the stack
  int used;      // 1 = slot occupied, 0 = empty
} HashEntry;

typedef struct HashMap {
  HashEntry entries[HASHMAP_CAP];
  size_t count;
} HashMap;

static inline HashMap initHashMap() {
  HashMap m;
  memset(&m, 0, sizeof(HashMap)); // zero everything, used=0 for all
  return m;
}

// djb2 hash
static inline size_t hash_key(const char *key) {
  size_t hash = 5381;
  int c;
  while ((c = (unsigned char)*key++))
    hash = ((hash << 5) + hash) + c;
  return hash % HASHMAP_CAP;
}

static inline void hashmap_insert(HashMap *m, const char *key, int stack_pos) {
  size_t i = hash_key(key);

  // linear probing for collision
  while (m->entries[i].used && strcmp(m->entries[i].key, key) != 0)
    i = (i + 1) % HASHMAP_CAP;

  if (!m->entries[i].used) {
    m->entries[i].key = strdup(key);
    m->entries[i].used = 1;
    m->count++;
  }

  m->entries[i].stack_pos = stack_pos;
}

static inline HashEntry *hashmap_get(HashMap *m, const char *key) {
  size_t i = hash_key(key);

  while (m->entries[i].used) {
    if (strcmp(m->entries[i].key, key) == 0)
      return &m->entries[i];
    i = (i + 1) % HASHMAP_CAP;
  }

  return NULL; // not found
}

static inline void freeHashMap(HashMap *m) {
  for (size_t i = 0; i < HASHMAP_CAP; i++) {
    if (m->entries[i].used)
      free(m->entries[i].key);
  }
}
