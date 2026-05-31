#include "oc_errors.h"

OC_error pack_error(AST_node* node, char* message){
    OC_error e = calloc(1, sizeof(OC_error));
    e->message = message;
    e->node = node;

    return e;
}

void print_errors(linked_list* ll, char* name){
    for (linked_list_node* lln = ll->head; lln != NULL; lln = lln->next){
        OC_error e = (OC_error) lln->data;
        if (e->node){
            printf("%s_error on line %d: %s\n", name, e->node->pos.line, e->message);
        } else {
            printf("%s_error: %s\n", name, e->message);
        }
    }   
}