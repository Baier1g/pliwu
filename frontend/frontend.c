#include <stdio.h>
#include "ast.h"
#include "y.tab.h"

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
    kill_tree(prog);
    return 0;
}