#ifndef OPTIMISER_H
#define OPTIMISER_H

#include "ast.h"
#include "ir.h"
#include <string.h>

/*
 * Performs constant folding on the AST.
 * Does not include variables, since liveness analysis is required.
 */
AST_node *AST_optimiser_constant_folding(AST_node *);

/*
 * Performs dead code elimination on the provided AST.
 */
AST_node *AST_optimiser_dead_code_elimination(AST_node *);

#endif