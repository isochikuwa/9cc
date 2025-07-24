#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "9cc.h"

const char *pointers[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void gen_lval(Node *node) {
  if (node->kind != ND_LVAR) {
    error("代入の左辺値が変数ではありません");
  }

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

char *function_name(Node *node) {
  if (!(node->name || node->name_len)) {
    // TODO: エラーハンドリング
    return NULL;
  }

  char *function_name = calloc(1, node->name_len+1);
  strncpy(function_name, node->name, node->name_len);

  return function_name;
}

void gen_block(Node *node) {
  for (NodeList *cur = node->statements; cur; cur = cur->next) {
    gen(cur->node);
    printf("  pop rax\n");
  }
}

void gen_function(Node *node) {
  printf("%s:\n", function_name(node));
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");

  // ローカル変数用の領域を確保
  printf("  sub rsp, %d\n", node->offset);

  int i = 0;
  // arguments
  for (NodeList *cur = node->arguments; cur; cur = cur->next) {
    if (cur->node->kind != ND_LVAR) continue;
 
    // 引数をレジスタからスタックに移す
    printf("  mov [rbp-%d], %s\n", cur->node->offset, pointers[i]);
    i++;
  }

  // statement
  gen(node->lhs);

  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
}

void gen_call(Node *node) {
  int i = 0;
  for(NodeList *cur = node->arguments; cur; cur = cur->next) {
    gen(cur->node);
    // 評価した値をレジスタにいれる
    // gcc だと引数の後ろからレジスタに入れてそうだったけど一旦前からいれることとする
    printf("  pop rax\n");
    printf("  mov %s, rax\n", pointers[i]);
    i++;
  }
  printf("  call %s\n", function_name(node));
  printf("  push rax\n");
}

void gen_return(Node *node) {
  gen(node->lhs);
  printf("  pop rax\n");
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
}

void gen_if(Node *node) {
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
}

void gen_while(Node *node) {
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
}

void gen_for(Node *node) {
  if (node->initialize) {
    gen(node->initialize);
  }
  printf(".Lbegin%d:\n", unique_number);
  gen(node->condition);
  printf("  pop rax\n");
  printf("  cmp rax, 0\n");
  printf("  je .Lend%d\n", unique_number);
  gen(node->consequence);
  if (node->finalize) {
    gen(node->finalize);
  }
  printf("  jmp .Lbegin%d\n", unique_number);
  printf(".Lend%d:\n", unique_number);
  unique_number++;
  printf("  push rax\n");
}

void gen_num(Node *node) {
  printf("  push %d\n", node->val);
}

void gen_lvar(Node *node) {
  gen_lval(node);
  printf("  pop rax\n");
  printf("  mov rax, [rax]\n");
  printf("  push rax\n");
}

void gen_assign(Node *node) {
  gen_lval(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  mov [rax], rdi\n");
  printf("  push rdi\n");
}

void gen_for_infix(Node *node) {
  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");
}

void gen_add(Node *node) {
  gen_for_infix(node);
  printf("  add rax, rdi\n");
  printf("  push rax\n");
}

void gen_sub(Node *node) {
  gen_for_infix(node);
  printf("  sub rax, rdi\n");
  printf("  push rax\n");
}

void gen_mul(Node *node) {
  gen_for_infix(node);
  printf("  imul rax, rdi\n");
  printf("  push rax\n");
}

void gen_div(Node *node) {
  gen_for_infix(node);
  printf("  cqo\n");
  printf("  idiv rdi\n");
  printf("  push rax\n");
}

void gen_eq(Node *node) {
  gen_for_infix(node);
  printf("  cmp rax, rdi\n");
  printf("  sete al\n");
  printf("  movzb rax, al\n");
  printf("  push rax\n");
}

void gen_neq(Node *node) {
  gen_for_infix(node);
  printf("  cmp rax, rdi\n");
  printf("  setne al\n");
  printf("  movzb rax, al\n");
  printf("  push rax\n");
}

void gen_lt(Node *node) {
  gen_for_infix(node);
  printf("  cmp rax, rdi\n");
  printf("  setl al\n");
  printf("  movzb rax, al\n");
  printf("  push rax\n");
}

void gen_lte(Node *node) {
  gen_for_infix(node);
  printf("  cmp rax, rdi\n");
  printf("  setle al\n");
  printf("  movzb rax, al\n");
  printf("  push rax\n");
}

void gen_gt(Node *node) {
  gen_for_infix(node);
  printf("  cmp rax, rdi\n");
  printf("  setg al\n");
  printf("  movzb rax, al\n");
  printf("  push rax\n");
}

void gen_gte(Node *node) {
  gen_for_infix(node);
  printf("  cmp rax, rdi\n");
  printf("  setge al\n");
  printf("  movzb rax, al\n");
  printf("  push rax\n");
}

void gen(Node *node) {
  switch (node->kind) {
    case ND_RETURN:
      gen_return(node);
      break;
    case ND_BLOCK:
      gen_block(node);
      break;
    case ND_FUNCTION:
      gen_function(node);
      break;
    case ND_CALL:
      gen_call(node);
      break;
    case ND_IF:
      gen_if(node);
      break;
    case ND_WHILE:
      gen_while(node);
      break;
    case ND_FOR:
      gen_for(node);
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
