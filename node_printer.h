#ifndef _H_NODE_PRINTER
#define _H_NODE_PRINTER

#include "9cc.h"

char *node_kind_to_string(NodeKind kind);
void print_node(Node *node, int hierarchy);

#endif // _H_NODE_PRINTER
