#pragma once

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Array {
  void *data;
  size_t used;
  size_t size;
  size_t elem_size;
} Array;

Array initArray(size_t initSize, size_t elem_size);
void appendArray(Array *a, void *elem);
void *getArray(Array *a, size_t index);

typedef void (*FreeFn)(void *elem);
void freeArray(Array *a, FreeFn fn);
