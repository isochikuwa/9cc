#include <stdio.h>
#include <string.h>
#include "9cc.h"
#include "node_printer.h"

// 入力プログラム
char *user_input;
// 現在着目しているトークン
Token *token;

NodeList *code;
// ローカル変数
LVar *locals;
// グローバル変数
GVar *globals;

int unique_number = 0;

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  // トークナイズしてパースする
  user_input = argv[1];
  tokenize(user_input);
  program();

  // 3つ目の引数に何か渡されたらデバッグ用コードを動かす
  if (argc == 3) {
    for (NodeList *cur = code; cur; cur = cur->next) {
      print_node(cur->node, 0);
    }
  } else {
    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    // グローバル変数定義を出力する
    printf(".section .bss\n");
    for (GVar *val = globals; val; val = val->next) {
      char name[val->len+1];
      strncpy(name, val->name, val->len);
      name[val->len] = '\0';
      printf("%s:\n", name);
      printf("  .zero %d\n", val->size);
    }

    printf(".section .text\n");
    printf(".globl main\n");

    // 先頭の式から順にコード生成
    for (NodeList *cur = code; cur; cur = cur->next) {
      gen(cur->node);
    }

    printf("\n");
    printf(".section .note.GNU-stack,\"\",@progbits\n");
  }

  return 0;
}
