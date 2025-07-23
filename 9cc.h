#ifndef _H_9CC
#define _H_9CC

#include <stdbool.h>

typedef enum {
  TK_RESERVED,  // 記号
  TK_IDENT,     // 識別子
  TK_NUM,       // 数値
  TK_RETURN,    // リターン
  TK_IF,        // IF
  TK_ELSE,      // ELSE
  TK_WHILE,     // WHILE
  TK_FOR,       // FOR
  TK_EOF,       // 入力の終わりを示すトークン
} TokenKind;

typedef struct Token Token;
typedef struct Node Node;
typedef struct LVar LVar;
typedef struct NodeList NodeList;

struct Token {
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMの場合、その数値
  char *str;      // トークン文字列
  int len;        // トークンの長さ
};

typedef enum {
  ND_ADD,      // +
  ND_SUB,      // -
  ND_MUL,      // *
  ND_DIV,      // /
  ND_EQ,       // ==
  ND_NEQ,      // !=
  ND_LT,       // <
  ND_LTE,      // <=
  ND_GT,       // >
  ND_GTE,      // >=
  ND_ASSIGN,   // =
  ND_LVAR,     // ローカル変数
  ND_NUM,      // 整数
  ND_IF,       // IF
  ND_RETURN,   // リターン
  ND_WHILE,    // WHILE
  ND_FOR,      // FOR
  ND_BLOCK,    // ブロック
  ND_CALL,     // 関数呼び出し
  ND_FUNCTION, // 関数
} NodeKind;


// 抽象構文木のノードの型
struct Node {
  NodeKind kind;  // ノードの型
  Node *lhs;      // 左辺
  Node *rhs;      // 右辺
  Node *condition;     // 条件式 IF, WHILE, FOR の場合のみ使う
  Node *consequence;  // condition が true の場合に評価される。IF, WHLIE, FOR の場合のみ使う
  Node *alternative;  // condition が false の場合に評価される。IF の場合のみ使う
  Node *initialize;   // FOR の場合のみ使う
  Node *finalize;   // FOR の場合のみ使う
  NodeList *statements; // ブロックの場合のみ使う
  NodeList *arguments; // 関数呼び出し式の場合に使う
  int val;        // kindがND_NUMの場合のみ使う
  int offset;     // kindがND_LVARの場合のみ使う
};

// ローカル変数の型
struct LVar {
  LVar *next; // 次の変数かNULL
  char *name; // 変数の名前
  int len;    // 名前の長さ;
  int offset; // RBPからのオフセット
};

struct NodeList {
  NodeList *next;
  Node *node;
};

void program();
Node *function();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
bool consume_token(TokenKind expected);
Token *consume_ident();
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
void tokenize(char *p);
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();
void gen(Node *node);
LVar *find_lvar(Token *tok);

// 入力プログラム
extern char *user_input;
// 現在着目しているトークン
extern Token *token;
extern NodeList *code;
// ローカル変数
extern LVar *locals;
// goto文用の通し番号
extern int unique_number;

#endif // _H_9CC
