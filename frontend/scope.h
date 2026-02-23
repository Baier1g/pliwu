#ifndef SCOPE_H
#define SCOPE_H

#include "ast.h"
#include "linked_list.h"
#include "symbol_table.h"

/*
 * Scopechecks the given AST node
 * returns 0 if an errors was discovered.
 */
int scopecheck (struct AST_node *, linked_list *);

#endif
