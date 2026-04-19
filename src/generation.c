#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "generation.h"
#include "hashmap.h"
#include "node.h"
#include "scope.h"
#include "tokenizer.h"

#define QWORD 8

void gen_expr(Generator *g, NodeExpr *expr);
void gen_stmt(Generator *g, NodeStmt *stmt);
void gen_scope(Generator *g, NodeScope *scope);
void gen_term(Generator *g, NodeTerm *term);
void gen_bin_op(Generator *g, NodeExprBinOp *bin);
void gen_if_pred(Generator *g, NodeIfPred *pred, char *end_label);
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
  strcat(g->result, "\t; exit fallback\n");
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
  char buf[BUF_SIZE];

  switch (stmt->type) {
  case STMT_IF:;
    sprintf(buf, "\t; if loc%zu\n", g->label_count);
    strcat(g->result, buf);

    char end_label[32];
    sprintf(end_label, "loc%zu", g->label_count++);

    gen_expr(g, stmt->if_stmt->expr);
    pop(g, "rax");

    char next_label[32];
    sprintf(next_label, "loc%zu", g->label_count++);

    strcat(g->result, "\ttest rax,rax\n");

    sprintf(buf, "\tjz %s\n", next_label);
    strcat(g->result, buf);

    gen_scope(g, stmt->if_stmt->scope);

    sprintf(buf, "\tjmp %s\n", end_label);
    strcat(g->result, buf);

    sprintf(buf, "%s:\n", next_label);
    strcat(g->result, buf);

    if (stmt->if_stmt->pred != NULL)
      gen_if_pred(g, stmt->if_stmt->pred, end_label);

    sprintf(buf, "%s:\n", end_label);
    strcat(g->result, buf);
    break;
  case STMT_ASSIGN:;
    strcat(g->result, "\t; assignment\n");

    HashEntry *entry = scope_get(&g->scope, stmt->assign->ident->str);
    if (entry == NULL) {
      printf("there is no identifier in stack: %s", stmt->let->ident->str);
      exit(EXIT_FAILURE);
    };

    gen_expr(g, stmt->assign->expr);
    pop(g, "rax");

    char buf[BUF_SIZE];
    size_t offset = (g->stack_size - entry->stack_pos - 1) * QWORD;

    sprintf(buf, "\tmov QWORD [rsp + %zu], rax\n", offset);
    strcat(g->result, buf);
    break;
  case STMT_EXIT:
    strcat(g->result, "\t; exit call\n");

    gen_expr(g, stmt->exit->expr);
    strcat(g->result, "\tmov rax, 60\n"); // syscall exit
    pop(g, "rdi");                        // load the statement for exit code
    strcat(g->result, "\tsyscall\n");
    break;
  case STMT_LET:
    strcat(g->result, "\t; let assignment\n");

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
  strcat(g->result, "\t; scope start\n");

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

  strcat(g->result, "\t; scope end\n");
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

void gen_if_pred(Generator *g, NodeIfPred *pred, char *end_label) {
  char buf[BUF_SIZE];

  switch (pred->type) {
  case IF_PRED_ELIF:;
    char next_label[32];
    sprintf(next_label, "loc%zu", g->label_count++);

    gen_expr(g, pred->pred_elif->expr);
    pop(g, "rax");

    strcat(g->result, "\ttest rax,rax\n");
    sprintf(buf, "\tjz %s\n", next_label);
    strcat(g->result, buf);

    gen_scope(g, pred->pred_elif->scope);

    sprintf(buf, "\tjmp %s\n", end_label); // skip
    strcat(g->result, buf);

    sprintf(buf, "%s:\n", next_label);
    strcat(g->result, buf);

    if (pred->pred_elif->pred != NULL)
      gen_if_pred(g, pred->pred_elif->pred, end_label);

    break;
  case IF_PRED_ELSE:
    gen_scope(g, pred->pred_else->scope);
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
