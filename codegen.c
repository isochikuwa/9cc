#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "9cc.h"

const char *rpointer[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
const char *epointer[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};

char *trim_name(char *str, int len) {
  char *name = calloc(1, sizeof(len+1));
  strncpy(name, str, len);
  name[len] = '\0';

  return name;
}

void gen_lval(Node *node) {
  if (node->kind != ND_LVAR && node->kind != ND_GVAR) {
    error("代入の左辺値が変数ではありません");
  }

  if (node->kind == ND_LVAR) {
    printf("  lea rax, [rbp-%d]\n", node->offset);
  } else if (node->kind == ND_GVAR) {
    char *name = trim_name(node->gvar->name, node->gvar->len);
    printf("  lea rax, %s[rip]\n", name);
  }
  printf("  push rax\n");
}

char *function_name(Node *node) {
  if (!(node->name || node->name_len)) {
    // TODO: エラーハンドリング
    return NULL;
  }

  return trim_name(node->name, node->name_len);
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
  if (node->offset > 0) {
    printf("  sub rsp, %d\n", node->offset);
  }

  int i = 0;
  // arguments
  for (NodeList *cur = node->arguments; cur; cur = cur->next) {
    if (cur->node->kind != ND_LVAR) continue;
 
    // 引数をレジスタからスタックに移す
    // TODO: 領域が8以外のときで場合分けをする
    printf("  mov [rbp-%d], %s\n", cur->node->offset, rpointer[i]);
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
    printf("  mov %s, rax\n", rpointer[i]);
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
  if (node->lvar->type->ty == ARRAY) {
    // 配列の場合はポインタをそのまま返す
    gen_lval(node);
    return;
  }

  gen_lval(node);
  printf("  pop rax\n");
  printf("  mov rax, [rax]\n");
  printf("  push rax\n");
}

void gen_assign(Node *node) {
  if (node->lhs->kind == ND_DEREF) {
    // 左辺がポインタだった場合に参照先のポインタがRAXに入るようにする
    gen(node->lhs->rhs);
  } else {
    gen_lval(node->lhs);
  }

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

// TODO: バイナリに書き出す前に値を変化するように修正する
int calc_offset_ratio(Node *node) {
  int ratio = 1;
  if (node->lhs->lvar->type->ty == PTR || node->lhs->lvar->type->ty == ARRAY) {
    switch (node->lhs->lvar->type->ptr_to->ty) {
      case INT:
        ratio = 8;
        break;
      case PTR:
        ratio = 8;
        break;
      case ARRAY:
        ratio = 8;
        break;
    }
  }
  return ratio;
}

void gen_add(Node *node) {
  gen_for_infix(node);

  // lhsの種類に応じてrdiの値を変化させる
  if (node->lhs->kind == ND_LVAR) {
    // 左辺の方によって変化させる値を変える
    int ratio = calc_offset_ratio(node);
    if (ratio != 1) {
      printf("  imul rdi, %d\n", ratio);
    }
  }
  
  printf("  add rax, rdi\n");
  printf("  push rax\n");
}

void gen_sub(Node *node) {
  gen_for_infix(node);

  // lhsの種類に応じてrdiの値を変化させる
  // TODO: バイナリに書き出す前に値を変化するように修正する
  if (node->lhs->kind == ND_LVAR) {
    // 左辺の方によって変化させる値を変える
    int ratio = calc_offset_ratio(node);
    if (ratio != 1) {
      printf("  imul rdi, %d\n", ratio);
    }
  }

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

void gen_addr(Node *node) {
  gen_lval(node->rhs);
}

void gen_deref(Node *node) {
  gen(node->rhs);
  printf("  pop rax\n");
  printf("  mov rax, [rax]\n");
  printf("  push rax\n");
}

void gen_gvar(Node *node) {
  if (node->gvar->type->ty == ARRAY) {
    gen_lval(node);
    return;
  }

  gen_lval(node);
  printf("  pop rax\n");
  printf("  mov rax, [rax]\n");
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
    case ND_ADDR:
      gen_addr(node);
      break;
    case ND_DEREF:
      gen_deref(node);
      break;
    case ND_GVAR:
      gen_gvar(node);
      break;
    }
}
