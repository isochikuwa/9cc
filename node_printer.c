#include <stdio.h>
#include "node_printer.h"

char *node_kind_to_string(NodeKind kind) {
  switch (kind) {
    case ND_ADD:
      return "ND_ADD";
    case ND_SUB:
      return "ND_SUB";
    case ND_MUL:
      return "ND_MUL";
    case ND_DIV:
      return "ND_DIV";
    case ND_EQ:
      return "ND_EQ";
    case ND_NEQ:
      return "ND_NEQ";
    case ND_LT:
      return "ND_LT";
    case ND_LTE:
      return "ND_LTE";
    case ND_GT:
      return "ND_GT";
    case ND_GTE:
      return "ND_GTE";
    case ND_ASSIGN:
      return "ND_ASSIGN";
    case ND_LVAR:
      return "ND_LVAR";
    case ND_NUM:
      return "ND_NUM";
    case ND_IF:
      return "ND_IF";
    case ND_RETURN:
      return "ND_RETURN";
    case ND_WHILE:
      return "ND_WHILE";
    case ND_FOR:
      return "ND_FOR";
    case ND_BLOCK:
      return "ND_BLOCK";
  }
}

void print_node(Node *node, int hierarchy) {
  if (!node) {
    printf("\n%*s(NULL)\n", hierarchy * 2, "");
    return;
  }

  printf("\n%*s(%s)\n", hierarchy * 2, "", node_kind_to_string(node->kind));

  hierarchy++;
  if (node->val) {
    printf("%*sval: %d\n", hierarchy * 2, "", node->val);
  }
  if (node->offset) {
    printf("%*soffset: %d\n", hierarchy * 2, "", node->offset);
  }
  if (node->lhs) {
    printf("%*slhs: ", hierarchy * 2, "");
    print_node(node->lhs, hierarchy);
  }
  if (node->rhs) {
    printf("%*srhs: ", hierarchy * 2, "");
    print_node(node->rhs, hierarchy);
  }
  if (node->condition) {
    printf("%*scondition: ", hierarchy * 2, "");
    print_node(node->condition, hierarchy);
  }
  if (node->consequence) {
    printf("%*sconsequence: ", hierarchy * 2, "");
    print_node(node->consequence, hierarchy);
  }
  if (node->alternative) {
    printf("%*salternative: ", hierarchy * 2, "");
    print_node(node->alternative, hierarchy);
  }
  if (node->initialize) {
    printf("%*sinitialize: ", hierarchy * 2, "");
    print_node(node->initialize, hierarchy);
  }
  if (node->finalize) {
    printf("%*sfinalize: ", hierarchy * 2, "");
    print_node(node->finalize, hierarchy);
  }
  if (node->statements) {
    printf("%*sstatements: ", hierarchy * 2, "");
    for(NodeList *cur = node->statements; cur; cur = cur->next) {
      print_node(cur->node, hierarchy+1);
    }
  }
}
