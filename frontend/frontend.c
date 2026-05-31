#include <stdio.h>
#include "ast.h"
#include "y.tab.h"
#include "oc_errors.h"
#include "scope.h"
#include "type_checking.h"
#include "codegen.h"
#include "ir.h"
#include "optimiser.h"
#include "register_allocation.h"
#include "ir_codegen.h"


extern AST_node *run_bison(const char*, int*);

int main(int argc, char* argv[]) {
    short returnValue           = 0;
    unsigned int bison_errors   = 0;
    AST_node *prog = create_unary_node(0, 0, A_PROGRAM, linked_list_new());
    linked_list *errors     = linked_list_new();
    linked_list *gen_asm    = linked_list_new();
    frame *root     = NULL;
    RA_graph *graph = NULL;
    FILE *fp        = NULL;
    
    for (int i = 1; i < argc; i++) {
        char *filename = argv[i];
        AST_node *module = run_bison(filename, &bison_errors);
        if (!module) {
            printf("Bison failed :(\n");
            returnValue = -1;
            goto ret;
        }
        if (bison_errors) {
            printf("%d syntax errors in progam, terminating\n", bison_errors);
            returnValue = -1;
            goto ret;
        }
        linked_list_append(prog->program.modules, module);
    }
    //AST_printer(prog);  
    printf("scope checking: \n");
    if (scopecheck(prog, errors)) {
        print_errors(errors, "scope");
        returnValue = -1;
        goto ret;
    }
    printf("type checking: \n");
    if (typecheck(prog, errors)){
        print_errors(errors, "type");
        returnValue = -1;
        goto ret;
    }

    AST_optimiser_constant_folding(prog);
    //AST_printer(prog);

    int *count = calloc(1, sizeof(int));
    printf("Converting to IR:\n");
    root = create_IR_tree(count, prog);
    //print_IR_tree(root);
    printf("Register allocation\n");
    graph = register_allocation(count[0], root);
    //print_IR_tree(root);
    //print_graph(graph);
    printf("generating code\n");
    codegen(gen_asm, root, graph);

    printf("Writing to file\n");
    fp = fopen("gen_asm.asm", "w");
    int cou = 0;
    for (linked_list_node *n = gen_asm->head; n != NULL; n = n->next) {
        char *tmp = (char*) n->data;
        //printf("%s", tmp);
        fwrite(tmp, strlen(tmp), 1, fp);
    }
    ret:
    if (fp) {
        fclose(fp);
    }    
    linked_list_delete(errors);
    linked_list_delete(gen_asm);
    kill_tree(prog);
    if (graph) {
        kill_graph(graph);
    }
    if (!returnValue) {
        printf("All done!\n");
    }
    return returnValue;
}