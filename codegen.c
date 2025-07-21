#include <stdio.h>
#include "9cc.h"

void gen_comment(const char *s, bool end) {
  printf("# ");
  if (end) {
    printf("end: ");
  }
  printf("%s\n", s);
}

void gen_lval(Node *node) {
  if (node->kind != ND_LVAR) {
    error("代入の左辺値が変数ではありません");
  }

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void gen_return(Node *node) {
  gen_comment(__func__, false);
  gen(node->lhs);
  printf("  pop rax\n");
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  gen_comment(__func__, true);
}

void gen_if(Node *node) {
  gen_comment(__func__, false);
  gen(node->condition);
  printf("  pop rax\n");
  printf("  cmp rax, 0\n");
  if (node->alternative) {
    printf("  je .Lelse%d\n", unique_number);
  } else {
    printf("  je .Lend%d\n", unique_number);
  }
  gen(node->consequence);
  if (node->alternative) {
    printf(".Lelse%d:\n", unique_number);
    gen(node->alternative);
  }
  printf(".Lend%d:\n", unique_number);
  unique_number++;
  printf("  push rax\n");
  gen_comment(__func__, true);
}

void gen_while(Node *node) {
  gen_comment(__func__, false);
  printf(".Lbegin%d:\n", unique_number);
  gen(node->condition);
  printf("  pop rax\n");
  printf("  cmp rax, 0\n");
  printf("  je .Lend%d\n", unique_number);
  gen(node->consequence);
  printf("  jmp .Lbegin%d\n", unique_number);
  printf(".Lend%d:\n", unique_number);
  unique_number++;
  printf("  push rax\n");
  gen_comment(__func__, true);
}

void gen_num(Node *node) {
  gen_comment(__func__, false);
  printf("  push %d\n", node->val);
  gen_comment(__func__, true);
}

void gen_lvar(Node *node) {
  gen_comment(__func__, false);
  gen_lval(node);
  printf("  pop rax\n");
  printf("  mov rax, [rax]\n");
  printf("  push rax\n");
  gen_comment(__func__, true);
}

void gen_assign(Node *node) {
  gen_comment(__func__, false);
  gen_lval(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  mov [rax], rdi\n");
  printf("  push rdi\n");
  gen_comment(__func__, true);
}

void gen_for_infix(Node *node) {
  gen_comment(__func__, false);
  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");
  gen_comment(__func__, true);
}

void gen_add(Node *node) {
  gen_comment(__func__, false);
  gen_for_infix(node);
  printf("  add rax, rdi\n");
  printf("  push rax\n");
  gen_comment(__func__, true);
}

void gen_sub(Node *node) {
  gen_comment(__func__, false);
  gen_for_infix(node);
  printf("  sub rax, rdi\n");
  printf("  push rax\n");
  gen_comment(__func__, true);
}

void gen_mul(Node *node) {
  gen_comment(__func__, false);
  gen_for_infix(node);
  printf("  imul rax, rdi\n");
  printf("  push rax\n");
  gen_comment(__func__, true);
}

void gen_div(Node *node) {
  gen_comment(__func__, false);
  gen_for_infix(node);
  printf("  cqo\n");
  printf("  idiv rdi\n");
  printf("  push rax\n");
  gen_comment(__func__, true);
}

void gen_eq(Node *node) {
  gen_comment(__func__, false);
  gen_for_infix(node);
  printf("  cmp rax, rdi\n");
  printf("  sete al\n");
  printf("  movzb rax, al\n");
  printf("  push rax\n");
  gen_comment(__func__, true);
}

void gen_neq(Node *node) {
  gen_comment(__func__, false);
  gen_for_infix(node);
  printf("  cmp rax, rdi\n");
  printf("  setne al\n");
  printf("  movzb rax, al\n");
  printf("  push rax\n");
  gen_comment(__func__, true);
}

void gen_lt(Node *node) {
  gen_comment(__func__, false);
  gen_for_infix(node);
  printf("  cmp rax, rdi\n");
  printf("  setl al\n");
  printf("  movzb rax, al\n");
  printf("  push rax\n");
  gen_comment(__func__, true);
}

void gen_lte(Node *node) {
  gen_comment(__func__, false);
  gen_for_infix(node);
  printf("  cmp rax, rdi\n");
  printf("  setle al\n");
  printf("  movzb rax, al\n");
  printf("  push rax\n");
  gen_comment(__func__, true);
}

void gen_gt(Node *node) {
  gen_comment(__func__, false);
  gen_for_infix(node);
  printf("  cmp rax, rdi\n");
  printf("  setg al\n");
  printf("  movzb rax, al\n");
  printf("  push rax\n");
  gen_comment(__func__, true);
}

void gen_gte(Node *node) {
  gen_comment(__func__, false);
  gen_for_infix(node);
  printf("  cmp rax, rdi\n");
  printf("  setge al\n");
  printf("  movzb rax, al\n");
  printf("  push rax\n");
  gen_comment(__func__, true);
}

void gen(Node *node) {
  switch (node->kind) {
    case ND_RETURN:
      gen_return(node);
      break;
    case ND_IF:
      gen_if(node);
      break;
    case ND_WHILE:
      gen_while(node);
      break;
    case ND_NUM:
      gen_num(node);
      break;
    case ND_LVAR:
      gen_lvar(node);
      break;
    case ND_ASSIGN:
      gen_assign(node);
      break;
    case ND_ADD:
      gen_add(node);
      break;
    case ND_SUB:
      gen_sub(node);
      break;
    case ND_MUL:
      gen_mul(node);
      break;
    case ND_DIV:
      gen_div(node);
      break;
    case ND_EQ:
      gen_eq(node);
      break;
    case ND_NEQ:
      gen_neq(node);
      break;
    case ND_LT:
      gen_lt(node);
      break;
    case ND_LTE:
      gen_lte(node);
      break;
    case ND_GT:
      gen_gt(node);
      break;
    case ND_GTE:
      gen_gte(node);
      break;
    }
}
