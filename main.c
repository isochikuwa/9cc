#include <stdio.h>
#include "9cc.h"
#include "node_printer.h"

// 入力プログラム
char *user_input;
// 現在着目しているトークン
Token *token;

NodeList *code;
// ローカル変数
LVar *locals;

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
    printf(".globl main\n");
    printf("main:\n");

    // プロローグ
    // 変数26個分の領域を確保する
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    if (locals) {
      printf("  sub rsp, %d\n", locals->offset);
    }

    // 先頭の式から順にコード生成
    for (NodeList *cur = code; cur; cur = cur->next) {
      gen(cur->node);

      // 式の評価結果としてスタックに1つの値が残っている
      // はずなので、スタックが溢れないようにポップしておく
      printf("  pop rax\n");
    }

    // エピローグ
    // 最後の式の結果がRAXに残っているのでそれが返り値になる
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");

    printf("\n");
    printf(".section .note.GNU-stack,\"\",@progbits\n");
  }

  return 0;
}
