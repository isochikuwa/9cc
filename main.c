#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "9cc.h"
#include "node_printer.h"

// 入力プログラム
char *user_input;
// 現在着目しているトークン
Token *token;
// ローカル変数
LVar *locals;
// グローバル変数
GVar *globals;
// 文字列
StringList *strings;

int unique_number = 0;
// 入力ファイル名
char *filename;

char *read_file(char *path) {
  FILE *fp = fopen(path, "r");
  if (!fp) {
    error("cannot open %s: %s", path, strerror(errno));
  }

  // ファイルの長さを調べる
  if (fseek(fp, 0, SEEK_END) == -1) {
    error("%s: fseek: %s", path, strerror(errno));
  }
  size_t size = ftell(fp);
  if (fseek(fp, 0, SEEK_SET) == -1) {
    error("%s: fseek: %s", path, strerror(errno));
  }

  // ファイル内容を読み込む
  char *buf = calloc(1, size + 2);
  fread(buf, size, 1, fp);

  // ファイルが必ず\n\0で終わるようにする
  if (size == 0 || buf[size - 1] != '\n') {
    buf[size++] = '\n';
  }
  buf[size] = '\0';
  fclose(fp);
  return buf;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  // トークナイズしてパースする
  if (argc == 3 && strcmp(argv[2], "t") == 0) {
    // テスト用にコードを引数から直接読み込む
    filename = "";
    user_input = argv[1];
  } else {
    filename = argv[1];
    user_input = read_file(filename);
  }
  tokenize(user_input);
  NodeList *codes = program();

  // 3つ目の引数に d を渡されたらデバッグ用コードを動かす
  if (argc == 3 && strcmp(argv[2], "d") == 0) {
    for (NodeList *cur = codes; cur; cur = cur->next) {
      print_node(cur->node, 0);
    }
  } else {
    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".section .rodata\n");
    for (StringList *cur = strings; cur; cur = cur->next) {
      printf("msg%d:\n", cur->id);
      printf("  .asciz %s\n", cur->str);
    }
    // グローバル変数定義を出力する
    printf(".section .data\n");
    for (GVar *val = globals; val; val = val->next) {
      char *name = create_string_copy(val->name, val->len);
      printf("%s:\n", name);
      if (val->val == 0) {
        printf("  .zero %d\n", val->size);
      } else {
        if (val->type->ty == INT) {
          printf("  .long %d\n", val->val);
        } else if (val->type->ty == CHAR) {
          printf("  .byte %d\n", val->val);
        }
      }
      free(name);
    }

    printf(".section .text\n");
    printf(".globl main\n");

    // 先頭の式から順にコード生成
    for (NodeList *cur = codes; cur; cur = cur->next) {
      gen(cur->node);
    }

    printf("\n");
    printf(".section .note.GNU-stack,\"\",@progbits\n");
  }

  return 0;
}
