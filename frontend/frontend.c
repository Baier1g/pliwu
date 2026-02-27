#include <stdio.h>
#include "ast.h"
#include "y.tab.h"
#include "scope.h"
#include "codegen.h"


extern struct AST_node *run_bison(const char*);

int main(int argc, char* argv[]) {
    FILE *fp;

    struct AST_node *prog = create_unary_node(0, 0, A_PROGRAM, linked_list_new());
    
    for (int i = 1; i < argc; i++) {
        char *filename = argv[i];
        struct AST_node *module = run_bison(filename);
        if (!module) {
            printf("Bison failed :(");
            return -1;
        }
        linked_list_append(prog->program.modules, module);
    }
    AST_printer(prog);

    
    linked_list *errors = linked_list_new();
    printf("scope checking: \n");
    if (scopecheck(prog, errors)) {
        for (linked_list_node *lln = errors->head; lln != NULL; lln = lln->next) {
            printf("scope_error: %s\n", (char *) lln->data);
            
        }
    }
    linked_list *ass = linked_list_new();
    generate_code(ass, prog);
    fp = fopen("gen_asm.asm", "w");
    for (linked_list_node *n = ass->head; n != NULL; n = n->next) {
        char *tmp = (char*) n->data;
        fwrite(tmp, strlen(tmp), 1, fp);
    }
    fclose(fp);
    linked_list_delete(ass);
    linked_list_delete(errors);
    kill_tree(prog);
    return 0;
}