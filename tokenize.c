#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "9cc.h"

// 次のトークンが期待している記号のときは、トークンを1つ読み進めて
// 真を返す。それ以外は偽を返す。
bool consume(char *op) {
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
    return false;
  }

  token = token->next;
  return true;
}

// 次のトークンが変数のときはトークンを1つよみ進めてトークンを返す。
// それ以外は偽を返す。
Token *consume_ident() {
  if (token->kind != TK_IDENT) {
    return NULL;
  }

  Token *tok = token;
  token = token->next;
  return tok;
}

Token *consume_type() {
  if (token->kind != TK_IDENT ||
      (strncmp("int", token->str, 3) != 0 &&
       strncmp("char", token->str, 4) != 0)) {
    return NULL;
  }

  Token *tok = token;
  token = token->next;
  return tok;
}

Token *expect_type() {
  Token *tok = consume_type();
  if (!tok) {
    error_at(token->str, ERR_MSG_NO_TYPE);
  }

  return tok;
}

bool consume_token(TokenKind expected) {
  if (token->kind != expected) {
    return false;
  }

  token = token->next;
  return true;
}

bool consume_else() {
  if (token->kind != TK_ELSE) {
    return false;
  }

  token = token->next;
  return true;
}

bool consume_sizeof() {
  if (token->kind != TK_SIZEOF) {
    return false;
  }

  token = token->next;
  return true;
}

// 次のトークンが期待している記号のときは、トークンを1つ読み進める。
// それ以外はエラーを報告する。
void expect(char *op) {
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
    error_at(token->str, "'%s'ではありません", op);
  }

  token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外はエラーを報告する。
int expect_number() {
  if(token->kind != TK_NUM) {
    error_at(token->str, "数ではありません");
  }

  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

int is_alnum(char c) {
  return ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') ||
         (c == '_');
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

int string_length(char *p) {
  if (*p != '"') return 0;

  int i = 1;
  while (*(p + i) != '"') {
    i++;
  }

  return i + 1;
}

void tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    // 行コメントをスキップ
    if (strncmp(p, "//", 2) == 0) {
      p += 2;
      while (*p != '\n') {
        p++;
      }
      continue;
    }

    // ブロックコメントをスキップ
    if (strncmp(p, "/*", 2) == 0) {
      char *q = strstr(p + 2, "*/");
      if (!q) {
        error_at(p, "コメントが閉じられていません");
      }

      p = q + 2;
      continue;
    }

    int len = string_length(p);
    if (len > 0) {
      // string
      cur = new_token(TK_STRING, cur, p, len);
      p += len;
      continue;
    }

    if (strncmp(p, "sizeof", 6) == 0 && !is_alnum(p[6])) {
      cur = new_token(TK_SIZEOF, cur, p, 6);
      p += 6;
      continue;
    }

    if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
      cur = new_token(TK_RETURN, cur, p, 6);
      p += 6;
      continue;
    }

    if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
      cur = new_token(TK_IF, cur, p, 2);
      p += 2;
      continue;
    }

    if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
      cur = new_token(TK_ELSE, cur, p, 4);
      p += 4;
      continue;
    }

    if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
      cur = new_token(TK_WHILE, cur, p, 5);
      p += 5;
      continue;
    }

    if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
      cur = new_token(TK_FOR, cur, p, 3);
      p += 3;
      continue;
    }

    if (!(strncmp(p, "==", 2) && strncmp(p, "!=", 2) && strncmp(p, ">=", 2) && strncmp(p, "<=", 2))) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
        *p == '(' || *p == ')' || *p == '>' || *p == '<' ||
        *p == '=' || *p == ';' || *p == '{' || *p == '}' ||
        *p == ',' || *p == '&' || *p == '[' || *p == ']') {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    // 複数文字のローカル変数
    int i = 0;
    while ('a' <= *(p+i) && *(p+i) <= 'z') {
      i++;
    }
    if (i > 0) {
      cur = new_token(TK_IDENT, cur, p, i);
      p += i;
      continue;
    }

    error_at(cur->str, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p, 0);
  /* return head.next; */
  token = head.next;
}
