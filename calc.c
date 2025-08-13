#include "9cc.h"

int Node_calc(Node *node) {
  if (node->kind == ND_NUM) {
    return node->val;
  }

  int l, r;
  switch (node->kind) {
  case ND_ADD:
    l = Node_calc(node->lhs);
    r = Node_calc(node->rhs);
    return l + r;
  case ND_SUB:
    l = Node_calc(node->lhs);
    r = Node_calc(node->rhs);
    return l - r;
  case ND_MUL:
    l = Node_calc(node->lhs);
    r = Node_calc(node->rhs);
    return l * r;
  case ND_DIV:
    l = Node_calc(node->lhs);
    r = Node_calc(node->rhs);
    return l / r;
  default:
    error_at(token->str, ERR_MSG_DETECT_UNPARSABLE_TYPE);
    return 0; // unreachable
  }
}
