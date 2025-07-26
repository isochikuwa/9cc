#include <stdlib.h>
#include <string.h>
#include "9cc.h"

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_ident(Token *tok) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;

  LVar *lvar = find_lvar(tok);
  if (lvar) {
    node->offset = lvar->offset;
  } else {
    lvar = calloc(1, sizeof(LVar));
    lvar->next = locals;
    lvar->name = tok->str;
    lvar->len = tok->len;
    if (locals) {
      lvar->offset = locals->offset + 8;
    } else {
      lvar->offset = 8;
    }
    node->offset = lvar->offset;
    locals = lvar;
  }
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

NodeList *parse_expression_list() {
  NodeList head;
  NodeList *cur = &head;
  head.next = NULL;

  while (!consume(")") && !at_eof()) {
    NodeList *new_node_list = calloc(1, sizeof(NodeList));
    new_node_list->node = expr();
    cur->next = new_node_list;
    cur = cur->next;
    if (!consume(",")) {
      expect(")");
      break;
    }
  }

  return head.next;
}

Node *parse_define_variable() {
  Token *tok = consume_ident();

  if (!tok) {
    // TODO: エラー処理
    return NULL;
  }

  return new_node_ident(tok);
}

NodeList *parse_function_arguments() {
  NodeList head;
  NodeList *cur = &head;
  head.next = NULL;

  while (!consume(")") && !at_eof()) {
    NodeList *new_node_list = calloc(1, sizeof(NodeList));
    new_node_list->node = parse_define_variable();
    cur->next = new_node_list;
    cur = cur->next;
    if (!consume(",")) {
      expect(")");
      break;
    }
  }

  return head.next;
}

void program() {
  NodeList head;
  NodeList *cur = &head;
  head.next = NULL;
  
  while (!at_eof()) {
    NodeList *new_node_list = calloc(1, sizeof(NodeList));
    new_node_list->node = function();
    cur->next = new_node_list;
    cur = cur->next;
  }
  
  code = head.next;
}

Node *function() {
  Node *node;

  Token *ident = consume_ident();
  if (!ident) {
    // TODO: エラー表示
    return NULL;
  }
  node = calloc(1, sizeof(Node));
  node->kind = ND_FUNCTION;
  node->name = ident->str;
  node->name_len = ident->len;

  // ローカル変数初期化
  locals = NULL;

  expect("(");
  node->arguments = parse_function_arguments(); 
  node->lhs = stmt();

  // 確保したオフセットを記録しておく
  if (locals) {
    node->offset = locals->offset;
  }

  return node;
}

Node *stmt() {
  Node *node;
  if (consume_token(TK_RETURN)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
    expect(";");
  } else if (consume_token(TK_IF)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_IF;
    expect("(");
    node->condition = expr();
    expect(")");
    node->consequence = stmt();
    if (consume_token(TK_ELSE)) {
      node->alternative = stmt();
    }
  } else if (consume_token(TK_WHILE)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_WHILE;
    expect("(");
    node->condition = expr();
    expect(")");
    node->consequence = stmt();
  } else if (consume_token(TK_FOR)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_FOR;
    expect("(");
    if (!consume(";")) {
      node->initialize = expr();
      expect(";");
    }
    if (!consume(";")) {
      node->condition = expr();
      expect(";");
    }
    if (!consume(")")) {
      node->finalize = expr();
      expect(")");
    }
    node->consequence = stmt();
  } else if (consume("{")) {
    NodeList head;
    NodeList *cur = &head;
    // ブロック処理
    while (!consume("}") && !at_eof()) {
      NodeList *new_node_list = calloc(1, sizeof(NodeList));
      new_node_list->node = stmt();
      cur->next = new_node_list;
      cur = cur->next;
    }
    node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    node->statements = head.next;
  } else {
    node = expr();
    expect(";");
  }
  return node;
}

Node *expr() {
  return assign();
}

Node *assign() {
  Node *node = equality();
  if (consume("=")) {
    node = new_node(ND_ASSIGN, node, assign());
  }

  return node;
}

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("==")) {
      node = new_node(ND_EQ, node, relational());
    } else if (consume("!=")) {
      node = new_node(ND_NEQ, node, relational());
    } else {
      return node;
    }
  }
}

Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<")) {
      node = new_node(ND_LT, node, add());
    } else if (consume("<=")) {
      node = new_node(ND_LTE, node, add());
    } else if (consume(">")) {
      node = new_node(ND_GT, node, add());
    } else if (consume(">=")) {
      node = new_node(ND_GTE, node, add());
    } else {
      return node;
    }
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+")) {
      node = new_node(ND_ADD, node, mul());
    } else if (consume("-")) {
      node = new_node(ND_SUB, node, mul());
    } else {
      return node;
    }
  }
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*")) {
      node = new_node(ND_MUL, node, unary());
    } else if (consume("/")) {
      node = new_node(ND_DIV, node, unary());
    } else {
      return node;
    }
  }
}

Node *unary() {
  if (consume("+")) {
    return primary();
  }
  if (consume("-")) {
    return new_node(ND_SUB, new_node_num(0), primary());
  }
  if (consume("*")) {
    return new_node(ND_DEREF, NULL, unary());
  }
  if (consume("&")) {
    return new_node(ND_ADDR, NULL, unary());
  }
  return primary();
}

Node *primary() {
  // 次のトークンが"("なら、"(" expr ")"のはず
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  if (tok) {
    if (consume("(")) {
      Node *node = calloc(1, sizeof(Node));
      node->kind = ND_CALL;
      node->name = tok->str;
      node->name_len = tok->len;
      // 関数呼び出し
      node->arguments = parse_expression_list();
      return node;
    } else {
      // 変数トークン
      return new_node_ident(tok);
    }
  }

  // そうでなければ数値のはず
  return new_node_num(expect_number());
}

LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
      return var;
    }
  }
  return NULL;
}
