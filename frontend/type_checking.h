#ifndef TYPE_CHECKING_H
#define TYPE_CHECKING_H

#include "ast.h"
#include "linked_list.h"
#include "symbol_table.h"

/*
 * Does type checking of the provided AST_node. Potential errors are stored in the provided linked list
 * Return 0 if no error was discovered, the amount of errors otherwise
 */
int typecheck(AST_node *, linked_list *);

#endif