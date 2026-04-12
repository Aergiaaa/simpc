#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "generation.h"
#include "node.h"
#include "scope.h"
#include "tokenizer.h"

#define QWORD 8

char *create_label(Generator *g);
void gen_expr(Generator *g, NodeExpr *expr);
void gen_stmt(Generator *g, NodeStmt *stmt);
void gen_scope(Generator *g, NodeScope *scope);
void gen_term(Generator *g, NodeTerm *term);
void gen_bin_op(Generator *g, NodeExprBinOp *bin);
void push(Generator *g, char *reg);
void pop(Generator *g, char *reg);

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
  case STMT_IF:
    gen_expr(g, stmt->if_stmt->expr);
    pop(g, "rax");

    char *label = create_label(g);

    strcat(g->result, "\ttest rax,rax\n");

    char buf[BUF_SIZE];
    sprintf(buf, "\tjz %s\n", label);
    strcat(g->result, buf);

    gen_scope(g, stmt->if_stmt->scope);

    sprintf(buf, "%s:\n", label);
    strcat(g->result, buf);

    break;
  case STMT_EXIT:
    gen_expr(g, stmt->exit->expr);
    strcat(g->result, "\tmov rax, 60\n"); // syscall exit
    pop(g, "rdi");                        // load the statement for exit code
    strcat(g->result, "\tsyscall\n");
    break;
  case STMT_LET:
    if (scope_get(&g->scope, stmt->let->ident->str) != NULL) {
      printf("identifier already used: %s", stmt->let->ident->str);
      exit(EXIT_FAILURE);
    }

    gen_expr(g, stmt->let->expr);
    scope_insert(&g->scope, stmt->let->ident->str, g->stack_size - 1);

    break;
  case STMT_SCOPE:
    gen_scope(g, stmt->scope);
    break;
  }
}

void gen_scope(Generator *g, NodeScope *scope) {
  scope_push(&g->scope, g->stack_size);

  for (int i = 0; i < scope->stmt.used; i++) {
    NodeStmt *inner_stmt = (NodeStmt *)getArray(&scope->stmt, i);
    gen_stmt(g, inner_stmt);
  }

  size_t vars_to_pop = scope_pop(&g->scope, g->stack_size);
  if (vars_to_pop > 0) {
    char buf[BUF_SIZE];
    sprintf(buf, "\tadd rsp, %zu\n", vars_to_pop * QWORD);
    strcat(g->result, buf);
    g->stack_size -= vars_to_pop;
  }
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
      HashEntry *entry = scope_get(&g->scope, term->ident->ident->str);
      if (entry == NULL) {
        printf("undefined variable: %s\n", term->ident->ident->str);
        exit(EXIT_FAILURE);
      }

      size_t offset = (g->stack_size - entry->stack_pos - 1) * QWORD;
      sprintf(buf, "QWORD [rsp + %zu]", offset);
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

char *create_label(Generator *g) {
  static char buf[BUF_SIZE];
  sprintf(buf, "loc%d", g->label_count++);
  return buf;
};
