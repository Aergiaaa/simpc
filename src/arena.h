#pragma once

#include <stdio.h>
#include <stdlib.h>

typedef struct ArenaAllocator {
  size_t size;
  void *buffer;
  void *offset;
} ArenaAllocator;

static inline ArenaAllocator *initArena(size_t bytes) {
  ArenaAllocator *a = (ArenaAllocator *)malloc(sizeof(ArenaAllocator));
  a->size = bytes;
  a->buffer = malloc(bytes);
  a->offset = a->buffer;
  return a;
}

static inline void *alloc(ArenaAllocator *a, size_t size) {
  size_t used = (char *)a->offset - (char *)a->buffer;
  if (used + size > a->size) {
    printf("Arena out of memory\n");
    exit(EXIT_FAILURE);
  }

  void *ptr = a->offset;
  a->offset = (char *)a->offset + size;
  return ptr;
}

static inline void freeArena(ArenaAllocator *a) {
  free(a->buffer);
  free(a);
}
