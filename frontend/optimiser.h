#ifndef OPTIMISER_H
#define OPTIMISER_H

#include <string.h>
#include "ast.h"
#include "ir.h"

/*
 * Performs constant folding on the AST.
 * Does not include variables, since liveness analysis is required.
 */
AST_node *AST_optimiser_constant_folding(AST_node *);

#endif