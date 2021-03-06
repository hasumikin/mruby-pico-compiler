#ifndef PICORBC_NODE_H_
#define PICORBC_NODE_H_

#include <stdint.h>
#include <stdbool.h>
#include "parse_header.h"
#ifndef PICORBC_PTR_SIZE
#include <ptr_size.h>
#endif

const char *Node_valueName(Node *self);

void Node_setValue(Node *self, const char *s);

bool Node_isAtom(Node *self);

bool Node_isCons(Node *self);

bool Node_isLiteral(Node *self);

AtomType Node_atomType(Node *self);

const char *Node_literalName(Node *self);

NodeBox *Node_newBox(ParserState *p);

Node *Node_new(ParserState *p);

void Node_freeAllNode(NodeBox *box);

void freeNode(Node *n);

#endif
