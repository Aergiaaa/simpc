#include "generation.h"
#include "array.h"
#include "hashmap.h"
#include "node.h"

#include <stdio.h>
#include <stdlib.h>

#define QWORD 8

void push(Generator *g, char *reg) {
  char tmp[BUF_SIZE];
  sprintf(tmp, "\tpush %s\n", reg);
  strcat(g->result, tmp);
  g->stack_size++;
}

void pop(Generator *g, char *reg) {
  char tmp[BUF_SIZE];
  sprintf(tmp, "\tpop %s\n", reg);
  strcat(g->result, tmp);
  g->stack_size--;
}

void gen_expr(Generator *g, NodeExpr *expr) {
  char buf[BUF_SIZE];
  switch (expr->type) {
  case EXPR_INT_LIT:
    sprintf(buf, "\tmov rax, %s\n", expr->int_lit->lit->str);
    strcat(g->result, buf);
    push(g, "rax");
    break;
  case EXPR_IDENT:;
    {
      HashEntry *entry = hashmap_get(&g->vars, expr->ident->ident->str);
      if (entry == NULL) {
        printf("undefined variable: %s\n", expr->ident->ident->str);
        exit(EXIT_FAILURE);
      }

      size_t offset = (g->stack_size - entry->stack_pos - 1) * QWORD;
      sprintf(buf, "QWORD [rsp + %zu]\n", offset);
      push(g, buf);
      break;
    }
  }
}

void gen_stmt(Generator *g, NodeStmt *stmt) {
  switch (stmt->type) {
  case STMT_EXIT:
    gen_expr(g, stmt->exit->expr);
    strcat(g->result, "\tmov rax, 60\n"); // syscall exit
    pop(g, "rdi");                        // load the statement for exit code
    strcat(g->result, "\tsyscall\n");
    break;
  case STMT_LET:
    if (hashmap_get(&g->vars, stmt->let->ident->str) != NULL) {
      printf("identifier already used: %s", stmt->let->ident->str);
      exit(EXIT_FAILURE);
    }

    gen_expr(g, stmt->let->expr);
    hashmap_insert(&g->vars, stmt->let->ident->str, g->stack_size - 1);

    break;
  }
}

char *generate(Generator *g) {
  g->result[0] = '\0';
  strcat(g->result, "global _start\n_start:\n");

  for (int i = 0; i < g->root->stmt.used; i++) {
    NodeStmt *stmt = (NodeStmt *)getArray(&g->root->stmt, i);
    gen_stmt(g, stmt);
  }

  // edge case if there is no exit
  strcat(g->result, "\tmov rax, 60\n");
  strcat(g->result, "\tmov rdi, 0\n");
  strcat(g->result, "\tsyscall\n");

  return g->result;
};
