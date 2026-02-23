
/* PROLOGUE */
%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "ast.h"
    #include "linked_list.h"

    extern int yylex(void);
    void yyerror(char const*);
    extern void yyrestart(FILE*);
    struct AST_node *run_bison(const char*);
    struct AST_node *binexp(void *, int, void *);
    struct AST_node *relexp(void *, int, void *);

    struct AST_node *prog;

    extern FILE *yyin;
    unsigned short line_number = 1;
    long current_character = 1;
    long start_current_character = 1;
%}

/* DECLARATIONS */
%union {
    struct AST_node* nval;
    struct linked_list *llval;
    double fval;
    int ival;
    char cval;
    char* sval;
}

/* TOKENS */
%token T_RIGHT_BRACE
%token T_LEFT_BRACE
%token T_RIGHT_PAREN
%token T_LEFT_PAREN
%token T_COMMA
%token T_ASSIGN
%token ';'
%left  '<' '>' T_LESS_EQ T_GREATER_EQ T_EQUALS T_NEQUALS
%left  '+' '-'
%left  '*' '/'
%right '!'

/* DOUBLE CHARACTER TOKENS */
%token T_ARROW
%token T_OR T_AND

/* LONGER TOKENS */
%token T_IF
%token T_ELSE
%token T_RETURN
%token T_FUNC
%token T_PRINT

%token <ival> T_INT T_BOOL
%token <cval> T_CHAR
%token T_INT_TYPE
%token T_BOOL_TYPE
%token T_CHAR_TYPE
%token T_VOID_TYPE
%token <sval> T_IDENTIFIER

%type <nval> identifier primary postfixExpression unaryExpression castExpression
%type <nval> arithmeticExpression relationalExpression logicalAND logicalOR
%type <nval> conditionalExpression assignExpression expression expressionStatement
%type <nval> block returnStatement else ifStatement printStatement statement
%type <nval> function funcDefinition varDeclaration declaration
%type <nval> module 
%type <llval> blockBody args parameters
%type <ival> type returnType


/* GRAMMAR RULES */
%%
module:
    /*%empty*/          {prog = create_unary_node(0, 0, A_MODULE, linked_list_new());}
|   module declaration  {linked_list_append(prog->module.module_declarations, $2);}
;

declaration:
    varDeclaration  {$$ = $1;}
|   statement       {$$ = $1;}
|   funcDefinition  {$$ = $1;}
;

varDeclaration:
    type identifier ';' {$$ = create_ternary_node(start_current_character, line_number, A_VAR_DECL, $1, $2, (void *) NULL);}
|   type identifier T_ASSIGN conditionalExpression ';' {$$ = create_ternary_node(start_current_character, line_number, A_VAR_DECL, $1, $2, $4);}
;

type:
    T_INT_TYPE      {$$ = (int) TYPE_INT;}
|   T_CHAR_TYPE     {$$ = (int) TYPE_CHAR;}
|   T_BOOL_TYPE     {$$ = (int) TYPE_BOOL;}
|   T_VOID_TYPE     {$$ = (int) TYPE_VOID;}
;

funcDefinition:
    T_FUNC function {$$ = $2;};

function:
    identifier T_LEFT_PAREN parameters T_RIGHT_PAREN returnType block {$$ = create_quaternary_node(start_current_character, line_number, A_FUNC_DEF, $5, $1, $3, $6);}
; 

returnType:
    /*%empty*/      {$$ = (int) TYPE_VOID;}
|   T_ARROW type    {$$ = $2;}
;

parameters:
    /*%empty*/                          {$$ = linked_list_new();}
|   type identifier                     {$$ = linked_list_new(); linked_list_append($$, create_binary_node(start_current_character, line_number, A_PARAMETER_EXPR, $1, $2));}
|   parameters T_COMMA type identifier  {linked_list_append($1, create_binary_node(start_current_character, line_number, A_PARAMETER_EXPR, $3, $4)); $$ = $1;}
;

args:
    /*%empty*/                      {$$ = linked_list_new();}
|   assignExpression                {$$ = linked_list_new(); linked_list_append($$, $1);}
|   args T_COMMA assignExpression   {linked_list_append($1, $3); $$ = $1;}
;

statement:
    ifStatement         {$$ = $1;}             
|   returnStatement     {$$ = $1;}
|   printStatement      {$$ = $1;}
|   expressionStatement {$$ = $1;}
|   error statement
;

ifStatement:
    T_IF T_LEFT_PAREN expression T_RIGHT_PAREN block else {$$ = create_ternary_node(start_current_character, line_number, A_IF_STMT, $3, $5, $6);}
;

else:
    /*%empty*/      {$$ = NULL;}
|   T_ELSE block    {$$ = $2;}
;

returnStatement:
    T_RETURN expression ';' {$$ = create_unary_node(start_current_character, line_number, A_RETURN_STMT, $2);}
|   T_RETURN ';'            {$$ = create_unary_node(start_current_character, line_number, A_RETURN_STMT, NULL);}
;

printStatement:
    T_PRINT T_LEFT_PAREN expression T_RIGHT_PAREN ';' {$$ = create_unary_node(start_current_character, line_number, A_PRINT_STMT, $3);}
;

block:
    T_LEFT_BRACE blockBody T_RIGHT_BRACE {$$ = create_unary_node(start_current_character, line_number, A_BLOCK_STMT, $2);}
;

blockBody:
    /*%empty*/              {$$ = linked_list_new();}
|   blockBody declaration   {linked_list_append($1, $2); $$ = $1;}
;

expressionStatement:
    expression ';' {$$ = $1;}
;

expression:
    assignExpression {$$ = $1;}
|   error             
;

assignExpression:
    conditionalExpression           {$$ = $1;}
|   identifier T_ASSIGN expression  {$$ = create_binary_node(start_current_character, line_number, A_ASSIGN_EXPR, $1, $3);}
;

conditionalExpression:
    logicalOR {$$ = $1;}
;

logicalOR:
    logicalAND                  {$$ = $1;}
|   logicalOR T_OR logicalAND   {$$ = create_ternary_node(start_current_character, line_number, A_LOGICAL_EXPR, $1, (void *) A_OR, $3);}
;

logicalAND:
    relationalExpression                    {$$ = $1;}
|   logicalAND T_AND relationalExpression   {$$ = create_ternary_node(start_current_character, line_number, A_LOGICAL_EXPR, $1, (void*) A_AND, $3);}
;

relationalExpression:
    arithmeticExpression {$$ = $1;}
|   relationalExpression T_EQUALS arithmeticExpression      {$$ = relexp($1, A_EQUALS, $3);}
|   relationalExpression T_NEQUALS arithmeticExpression     {$$ = relexp($1, A_NEQUALS, $3);}
|   relationalExpression '<' arithmeticExpression           {$$ = relexp($1, A_LESS, $3);}
|   relationalExpression '>' arithmeticExpression           {$$ = relexp($1, A_GREATER, $3);}
|   relationalExpression T_LESS_EQ arithmeticExpression     {$$ = relexp($1, A_LESS_EQ, $3);}
|   relationalExpression T_GREATER_EQ arithmeticExpression  {$$ = relexp($1, A_GREATER_EQ, $3);}
;

arithmeticExpression:
    castExpression                          {$$ = $1;}
|   arithmeticExpression '+' castExpression {$$ = binexp($1, A_ADD, $3);}
|   arithmeticExpression '-' castExpression {$$ = binexp($1, A_SUB, $3);}
|   arithmeticExpression '*' castExpression {$$ = binexp($1, A_MULT, $3);}
|   arithmeticExpression '/' castExpression {$$ = binexp($1, A_DIV, $3);}
;

castExpression:
    unaryExpression {$$ = $1;}
;

unaryExpression:
    postfixExpression       {$$ = $1;}
|   '-' postfixExpression   {$$ = create_binary_node(start_current_character, line_number, A_UNARY_EXPR, A_MINUS, $2);}
|   '!' postfixExpression   {$$ = create_binary_node(start_current_character, line_number, A_UNARY_EXPR, A_NEG, $2);}
;

postfixExpression:
    primary {$$ = $1;}
|   postfixExpression T_LEFT_PAREN args T_RIGHT_PAREN {$$ = create_binary_node(start_current_character, line_number, A_CALL_EXPR, $1, $3);}
;

identifier:
    T_IDENTIFIER {$$ = create_binary_node(start_current_character, line_number, A_PRIMARY_EXPR, TYPE_IDENTIFIER, yylval.sval); free(yylval.sval);
                    printf("%s identifier returned to bison at line %d it starts at %ld and ends at %ld\n", $$->primary_expr.identifier_name, line_number, start_current_character, current_character);}
;

primary:
    T_INT           {$$ = create_binary_node(start_current_character, line_number, A_PRIMARY_EXPR, TYPE_INT, (void *) yylval.ival); printf("%d int value returned to bison at line %d it starts at %ld and ends at %ld\n", yylval.ival, line_number, start_current_character, current_character);}
|   T_CHAR          {$$ = create_binary_node(start_current_character, line_number, A_PRIMARY_EXPR, TYPE_CHAR, (void *) yylval.cval); printf("%c character returned to bison at line %d it starts at %ld and ends at %ld\n", yylval.cval, line_number, start_current_character, current_character);}
|   T_BOOL          {$$ = create_binary_node(start_current_character, line_number, A_PRIMARY_EXPR, TYPE_BOOL, (void *) yylval.ival); yylval.ival ? printf("true returned to bison\n") : printf("false returned to bison\n");}
|   T_LEFT_PAREN expression T_RIGHT_PAREN {$$ = $2;}
|   identifier      {$$ = $1;}
;

%%

void yyerror(char const* err) {
    printf("%s: line: %d, character: %ld, token: %s\n", err, line_number, start_current_character, "skill issue");
}

struct AST_node* binexp(void* left, int op, void* right) {
    return create_ternary_node(start_current_character, line_number, A_ARITHMETIC_EXPR, left, (void*) op, right);
}

struct AST_node* relexp(void* left, int op, void* right) {
    return create_ternary_node(start_current_character, line_number, A_RELATIONAL_EXPR, left, (void*) op, right);
}

struct AST_node *run_bison(const char* filename) {
    yyin = fopen(filename,"r");
    if (!yyin) {
        return NULL;
    }

    yyparse();
    printf("\nNumber of lines in the file - %u\n", line_number);
    printf("\nNumber of characters in the file - %ld\n", current_character);
    yyrestart(yyin);
    fclose(yyin);
    return prog;
}

/*int main(int argc, char* argv[]) {
    FILE *fp;
    char *filename = argv[1];

    yyin = fopen(filename,"r");
    if (!yyin) {
        return -2;
    }

    yyparse();
    printf("\nNumber of lines in the file - %u\n", line_number);
    printf("\nNumber of characters in the file - %ld\n", current_character);
    AST_printer(prog);
    return 0;
}*/

/* EPILOGUE */