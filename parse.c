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

void program() {
  int i = 0;
  for (i = 0; !at_eof(); i++) {
    code[i] = stmt();
  }
  code[i] = NULL;
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
  return primary();
}

Node *primary() {
  // 次のトークンが"("なら、"(" expr ")"のはず
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  // 変数トークン
  Token *tok = consume_ident();
  if (tok) {
    return new_node_ident(tok);
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
