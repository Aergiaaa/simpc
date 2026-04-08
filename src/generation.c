#include "array.h"
#include "hashmap.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "generation.h"
#include "node.h"
#include "tokenizer.h"

#define QWORD 8

void gen_expr(Generator *g, NodeExpr *expr);

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

void gen_term(Generator *g, NodeTerm *term) {
  char buf[BUF_SIZE];
  switch (term->type) {
  case TERM_INT_LIT:
    sprintf(buf, "\tmov rax, %s\n", term->int_lit->lit->str);
    strcat(g->result, buf);
    push(g, "rax");
    break;
  case TERM_IDENT:;
    {
      HashEntry *entry = hashmap_get(&g->vars, term->ident->ident->str);
      if (entry == NULL) {
        printf("undefined variable: %s\n", term->ident->ident->str);
        exit(EXIT_FAILURE);
      }

      size_t offset = (g->stack_size - entry->stack_pos - 1) * QWORD;
      sprintf(buf, "QWORD [rsp + %zu]\n", offset);
      push(g, buf);
      break;
    }
  case TERM_PAREN:;
    gen_expr(g, term->paren->expr);
  }
}

void gen_bin_op(Generator *g, NodeExprBinOp *bin) {
  switch (bin->type) {
  case BIN_ADD_EXPR:
    gen_expr(g, bin->add->left);
    gen_expr(g, bin->add->right);

    pop(g, "rbx");
    pop(g, "rax");
    strcat(g->result, "\tadd rax,rbx\n");
    push(g, "rax");
    break;

  case BIN_SUB_EXPR:
    gen_expr(g, bin->sub->left);
    gen_expr(g, bin->sub->right);

    pop(g, "rbx");
    pop(g, "rax");
    strcat(g->result, "\tsub rax,rbx\n");
    push(g, "rax");
    break;

  case BIN_MUL_EXPR:
    // implement multipication
    gen_expr(g, bin->mul->left);
    gen_expr(g, bin->mul->right);

    pop(g, "rbx");
    pop(g, "rax");
    strcat(g->result, "\timul rax,rbx\n");
    push(g, "rax");
    break;

  case BIN_DIV_EXPR:
    gen_expr(g, bin->div->left);
    gen_expr(g, bin->div->right);

    pop(g, "rbx");
    pop(g, "rax");
    strcat(g->result, "\txor rdx,rdx\n");
    strcat(g->result, "\tdiv rbx\n");
    push(g, "rax");
    break;

  default:
    break;
  }
}

void gen_expr(Generator *g, NodeExpr *expr) {
  switch (expr->type) {
  case EXPR_TERM:
    gen_term(g, expr->term);
    break;
  case EXPR_BIN_OP:
    gen_bin_op(g, expr->bin_op);
    break;
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
