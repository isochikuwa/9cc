#include <stdio.h>
#include <string.h>
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
    case ND_CALL:
      return "ND_CALL";
    case ND_FUNCTION:
      return "ND_FUNCTION";
    case ND_DEREF:
      return "ND_DEREF";
    case ND_ADDR:
      return "ND_ADDR";
  }
}

char *lvar_type_to_string(int kind) {
  if (kind == 0) {
    return "INT";
  } else if (kind == 1) {
    return "PTR";
  }
  return NULL;
}

void print_node(Node *node, int hierarchy) {
  if (!node) {
    printf("%*s(NULL)\n", hierarchy * 2, "");
    return;
  }

  printf("%*s(%s)\n", hierarchy * 2, "", node_kind_to_string(node->kind));

  hierarchy++;
  if (node->name) {
    char name[node->name_len+1];
    strncpy(name, node->name, node->name_len);
    name[node->name_len] = '\0';
    printf("%*sname: %s\n", hierarchy * 2, "", name);
  }
  if (node->val) {
    printf("%*sval: %d\n", hierarchy * 2, "", node->val);
  }
  if (node->offset) {
    printf("%*soffset: %d\n", hierarchy * 2, "", node->offset);
  }
  if (node->lhs) {
    printf("%*slhs:\n", hierarchy * 2, "");
    print_node(node->lhs, hierarchy+1);
  }
  if (node->rhs) {
    printf("%*srhs:\n", hierarchy * 2, "");
    print_node(node->rhs, hierarchy+1);
  }
  if (node->condition) {
    printf("%*scondition:\n", hierarchy * 2, "");
    print_node(node->condition, hierarchy+1);
  }
  if (node->consequence) {
    printf("%*sconsequence:\n", hierarchy * 2, "");
    print_node(node->consequence, hierarchy+1);
  }
  if (node->alternative) {
    printf("%*salternative:\n", hierarchy * 2, "");
    print_node(node->alternative, hierarchy+1);
  }
  if (node->initialize) {
    printf("%*sinitialize:\n", hierarchy * 2, "");
    print_node(node->initialize, hierarchy+1);
  }
  if (node->finalize) {
    printf("%*sfinalize:\n", hierarchy * 2, "");
    print_node(node->finalize, hierarchy+1);
  }
  if (node->arguments) {
    printf("%*sarguments:\n", hierarchy * 2, "");
    for(NodeList *cur = node->arguments; cur; cur = cur->next) {
      print_node(cur->node, hierarchy+1);
    }
  }
  if (node->statements) {
    printf("%*sstatements:\n", hierarchy * 2, "");
    for(NodeList *cur = node->statements; cur; cur = cur->next) {
      print_node(cur->node, hierarchy+1);
    }
  }
  if (node->lvar) {
    printf("%*slvar:\n", hierarchy * 2, "");
    printf("%*soffset: %d\n", (hierarchy + 2) * 2, "", node->lvar->offset);
    printf("%*stype:\n", (hierarchy + 2) * 2, "");
    printf("%*sty: %s\n", (hierarchy + 4) * 2, "", lvar_type_to_string(node->lvar->type->ty));
  }
  if (node->index) {
    printf("%*sindex:\n", hierarchy * 2, "");
    print_node(node->index, hierarchy+1);
  }
}
