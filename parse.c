#include <stdlib.h>
#include <string.h>
#include "9cc.h"

Type *parse_type_with_pointers(Token *typetok, int pdepth) {
  Type *type = calloc(1, sizeof(Type));
  if (!type) {
    error("メモリ割り当てに失敗しました");
  }
  
  // TODO: 他の型にも対応する
  if (strncmp("int", typetok->str, typetok->len) == 0) {
    type->ty = INT;
  } else if (strncmp("char", typetok->str, typetok->len) == 0) {
    type->ty = CHAR;
  }
  
  for (int i = 0; i < pdepth; i++) {
    Type *ptype = calloc(1, sizeof(Type));
    if (!ptype) {
      error("メモリ割り当てに失敗しました");
    }
    ptype->ty = PTR;
    ptype->ptr_to = type;
    type = ptype;
  }
  
  return type;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_declared_ident(Token *tok) {
  Node *node = calloc(1, sizeof(Node));
  LVar *lvar = find_lvar(tok);
  if (lvar) {
    node->kind = ND_LVAR;
    node->offset = lvar->offset;
    node->lvar = lvar;
  } else {
    GVar *gvar = find_gvar(tok);
    if (!gvar) {
      error_at(tok->str, ERR_MSG_UNDEFINED_VAR);
    }

    node->kind = ND_GVAR;
    node->gvar = gvar;
  }
  return node;
}

Node *new_ident(Token *tok, int offset, Type *type) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;

  LVar *lvar = calloc(1, sizeof(LVar));
  lvar->next = locals;
  lvar->name = tok->str;
  lvar->len = tok->len;
  lvar->type = type;
  if (locals) {
    lvar->offset = locals->offset + offset;
  } else {
    lvar->offset = offset;
  }
  node->offset = lvar->offset;
  node->lvar = lvar;
  locals = lvar;

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
  Node * node = declare();
  if (!node) {
    error_at(token->str, ERR_MSG_NO_TYPE);
  }

  return node;
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

NodeList *program() {
  NodeList head;
  NodeList *cur = &head;
  head.next = NULL;
  
  while (!at_eof()) {
    Node *node = global();
    if (!node) {
      continue;
    }

    NodeList *new_node_list = calloc(1, sizeof(NodeList));
    new_node_list->node = node;
    cur->next = new_node_list;
    cur = cur->next;
  }
  
  return head.next;
}

void *create_new_global(Token *ident, int size, Type *type) {
  GVar *gvar = calloc(1, sizeof(GVar));
  gvar->next = globals;
  gvar->name = ident->str;
  gvar->len = ident->len;
  gvar->type = type;
  gvar->size = size;

  globals = gvar;
}

Node *parse_function_definition(Token *ident, Token *typetok) {
  locals = NULL;
  
  Node *node = calloc(1, sizeof(Node));
  if (!node) {
    error(ERR_MSG_FAILED_TO_ATTACH_MEMORIES);
  }
  
  node->kind = ND_FUNCTION;
  node->name = ident->str;
  node->name_len = ident->len;
  node->arguments = parse_function_arguments(); 
  node->lhs = stmt();

  // 確保したオフセットを記録しておく
  if (locals) {
    node->offset = locals->offset;
  }
  
  return node;
}

Node *parse_global_variable(Token *ident, Token *typetok, int pdepth) {
  Type *type = parse_type_with_pointers(typetok, pdepth);
  
  int size;
  if (consume("[")) {
    Type *ptype = calloc(1, sizeof(Type));
    if (!ptype) {
      error(ERR_MSG_FAILED_TO_ATTACH_MEMORIES);
    }
    ptype->ty = ARRAY;
    ptype->ptr_to = type;
    type = ptype;

    // 配列
    Node *index = expr();
    if (index->kind != ND_NUM) {
      error_at(token->str, ERR_MSG_ARRAY_INDEX_NOT_NUM);
    }

    ptype->array_size = index->val;
    size = ptype->array_size * WORD_SIZE;

    expect("]");
  } else {
    size = decide_sizeof(type->ty);
  }
  
  create_new_global(ident, size, type);
  expect(";");

  return NULL;
}

Node *global() {
  Token *typetok = expect_type();
  int pdepth = 0;
  
  // 変数名の前に '*' がある場合はポインタ
  while (consume("*")) {
    pdepth++;
  }

  Token *ident = consume_ident();
  if (!ident) {
    error_at(token->str, ERR_MSG_NO_IDENTIFIER);
  }

  if (consume("(")) {
    // 関数定義
    return parse_function_definition(ident, typetok);
  } else {
    // グローバル変数定義
    return parse_global_variable(ident, typetok, pdepth);
  }
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
    node = declare();
    if (!node) {
      node = expr();
    }
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

TyType search_adder_type(Node *node) {
  int l, r;
  switch (node->kind) {
    case ND_NUM:
      return INT;
    case ND_ADD:
    case ND_SUB:
      l = search_adder_type(node->lhs);
      r = search_adder_type(node->rhs);
      if (l == r == INT) {
        return INT;
      } else {
        return PTR;
      }
    case ND_LVAR:
      return node->lvar->type->ty;
    default:
      error("計算対象外です");
  }
}

int decide_sizeof(TyType t) {
  switch (t) {
    case INT:
      return 4;
    case CHAR:
      return 1;
    case PTR:
      return 8;
    default:
      error(ERR_MSG_DETECT_UNPARSABLE_TYPE);
  }
}

Node *unary() {
  if (consume_sizeof()) {
    Node *node = unary();
    return new_node_num(decide_sizeof(search_adder_type(node)));
  }
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

Token *consume_string() {
  if (token->kind != TK_STRING) return NULL;

  Token *tok = token;
  token = token->next;
  return tok;
}

Node *parse_string(Token *tok) {
  char *str = create_string_copy(tok->str, tok->len);

  StringList *string = calloc(1, sizeof(StringList));
  string->str = str;
  string->next = strings;
  if (strings) {
    string->id = strings->id + 1;
  } else {
    string->id = 0;
  }
  strings = string;

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_STRING;
  node->string = string;

  return node;
}

Node *primary() {
  // 次のトークンが"("なら、"(" expr ")"のはず
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *str_token = consume_string();
  if (str_token) {
    Node *node = parse_string(str_token);
    return node;
  }

  Token *ident_token = consume_ident();
  if (ident_token) {
    if (consume("(")) {
      Node *node = calloc(1, sizeof(Node));
      node->kind = ND_CALL;
      node->name = ident_token->str;
      node->name_len = ident_token->len;
      // 関数呼び出し
      node->arguments = parse_expression_list();
      return node;
    } else if (consume("[")) {
      // 配列添字
      // a[b] => *(a + b) とみなす
      Node *node = calloc(1, sizeof(Node));
      node->kind = ND_DEREF;
      node->rhs = new_node(ND_ADD, new_node_declared_ident(ident_token), mul());
      expect("]");
      return node;
    } else {
      // 変数トークン
      return new_node_declared_ident(ident_token);
    }
  }

  // そうでなければ数値のはず
  return new_node_num(expect_number());
}

Node *declare() {
  Token *typetok = consume_type();
  if (!typetok) {
    return NULL;
  }

  int pdepth = 0;
  // 変数名の前に '*' がある場合はポインタ
  while (consume("*")) {
    pdepth++;
  }

  Token *tok = consume_ident();
  if (!tok) {
    error_at(typetok->str + typetok->len, ERR_MSG_NO_IDENTIFIER);
  }

  Type *type = parse_type_with_pointers(typetok, pdepth);

  Node *node;
  // 配列かどうか
  if (consume("[")) {
    Type *ptype = calloc(1, sizeof(Type));
    ptype->ty = ARRAY;
    ptype->ptr_to = type;

    Node *index = expr();
    if (index->kind == ND_NUM) {
      // 添字がただの数値なら先にoffsetを計算する
      ptype->array_size = index->val;
      int offset = ptype->array_size * decide_sizeof(ptype->ptr_to->ty);

      node = new_ident(tok, offset, ptype);
    } else {
      // 添字が数値でない場合はここではoffsetを計算せず、後ほど行う
      // TODO: 添字が数値でない場合の実装
    }
    expect("]");
  } else {
    int offset = decide_sizeof(type->ty);
    node = new_ident(tok, offset, type);
  }

  return node;
}

LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
      return var;
    }
  }
  return NULL;
}

GVar *find_gvar(Token *tok) {
  for (GVar *var = globals; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
      return var;
    }
  }

  return NULL;
}
