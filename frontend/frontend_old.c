#include <stdio.h>
#include "ast.h"
#include "y.tab.h"
#include "scope.h"
#include "type_checking.h"
#include "codegen.h"


extern AST_node *run_bison(const char*, int*);

int main(int argc, char* argv[]) {
    FILE *fp;
    AST_node *prog = create_unary_node(0, 0, A_PROGRAM, linked_list_new());
    unsigned int bison_errors = 0;
    linked_list *errors = linked_list_new();
    
    for (int i = 1; i < argc; i++) {
        char *filename = argv[i];
        AST_node *module = run_bison(filename, &bison_errors);
        if (!module) {
            printf("Bison failed :(");
            return -1;
        }
        if (bison_errors) {
            printf("%d syntax errors in progam, terminating\n", bison_errors);
            exit(-1);
        }
        linked_list_append(prog->program.modules, module);
    }
    //AST_printer(prog);

    
    printf("scope checking: \n");
    if (scopecheck(prog, errors)) {
        for (linked_list_node *lln = errors->head; lln != NULL; lln = lln->next) {
            printf("scope_error: %s\n", (char *) lln->data);
        }
        exit(-1);
    }
    printf("type checking: \n");
    if (typecheck(prog, errors)){
        for (linked_list_node *lln = errors->head; lln != NULL; lln = lln->next) {
            printf("type_error: %s\n", (char *) lln->data);
        }
        exit(-1);
    }

    //AST_optimiser_constant_folding(prog);
    //AST_printer(prog);

    linked_list *gen_asm_old = linked_list_new();
    generate_code(gen_asm_old, prog);
    for (linked_list_node* ln = gen_asm_old->head; ln != NULL; ln = ln->next) {
        //printf("%s\n", ln->data);
    }
    

    printf("Writing to file\n");
    fp = fopen("gen_asm_old.asm", "w");
    int cou = 0;
    for (linked_list_node *n = gen_asm_old->head; n != NULL; n = n->next) {
        char *tmp = (char*) n->data;
        //printf("%s", tmp);
        fwrite(tmp, strlen(tmp), 1, fp);
    }
    fclose(fp);
    linked_list_delete(gen_asm_old);
    linked_list_delete(errors);
    kill_tree(prog);
    printf("All done!\n");
    return 0;
}