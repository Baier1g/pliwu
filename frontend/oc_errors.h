#ifndef ERRORS_H
#define ERRORS_H

#include "ast.h"
#include "linked_list.h"

typedef struct OC_error* OC_error;

struct OC_error {
    char* message;
    AST_node* node;
};

OC_error pack_error(AST_node*, char*);
void print_errors(linked_list*, char*);

#endif