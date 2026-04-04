#include "array.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Array initArray(size_t initSize, size_t elem_size) {
  Array a;

  a.data = malloc(initSize * elem_size);
  if (a.data == NULL) {
    printf("Memory allocation failed\n");
    exit(EXIT_FAILURE);
  }

  a.used = 0;
  a.size = initSize;
  a.elem_size = elem_size;

  return a;
}

void appendArray(Array *a, void *elem) {
  if (a->used == a->size) {
    a->size *= 2;
    a->data = realloc(a->data, a->size * a->elem_size);
    if (a->data == NULL) {
      printf("Memory reallocation failed\n");
      exit(EXIT_FAILURE);
    }
  }

  memcpy((char *)a->data + a->used * a->elem_size, elem, a->elem_size);
  a->used++;
}

void freeArray(Array *a, FreeFn fn) {
  if (fn != NULL) {
    for (size_t i = 0; i < a->used; i++) {
      fn((char *)a->data + i * a->elem_size); // call destructor on each element
    }
  }

  free(a->data);
  a->data = NULL;
  a->used = 0;
  a->size = 0;
  a->elem_size = 0;
}

void *getArray(Array *a, size_t index) {
  return (char *)a->data + index * a->elem_size;
};
