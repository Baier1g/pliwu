
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
    AST_node *run_bison(const char*, int*);
    AST_node *binexp(void *, int, void *);
    AST_node *relexp(void *, int, void *);

    AST_node *prog;
    AST_node *node;

    int* bison_errors;

    extern FILE *yyin;
    unsigned short line_number = 1;
    long current_character = 1;
    long start_current_character = 1;
%}

/* DECLARATIONS */
%union {
    struct AST_node *nval;
    struct linked_list *llval;
    double fval;
    int ival;
    char cval;
    struct {
        int length;
        char *sval;
    } string;
}

/* TOKENS */
%token T_RIGHT_BRACE
%token T_LEFT_BRACE
%token T_RIGHT_PAREN
%token T_LEFT_PAREN
%token T_RIGHT_BRACKET
%token T_LEFT_BRACKET
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
%token T_WHILE
%token T_RETURN
%token T_FUNC
%token T_PRINT

%token <ival> T_INT T_BOOL
%token <cval> T_CHAR
%token <string> T_STRING
%token T_INT_TYPE
%token T_BOOL_TYPE
%token T_CHAR_TYPE
%token T_VOID_TYPE
%token T_STRING_TYPE
%token <string> T_IDENTIFIER

%type <nval> identifier primary postfixExpression unaryExpression castExpression
%type <nval> arithmeticExpression relationalExpression logicalAND logicalOR
%type <nval> conditionalExpression assignExpression expression expressionStatement
%type <nval> block returnStatement else ifStatement printStatement statement
%type <nval> loopStatement whileLoop 
%type <nval> function funcDefinition varDeclaration declaration multiplicativeExpression
%type <nval> module 
%type <llval> blockBody args parameters declarator initializerList initializerValue arrayInitializer initializerDim
%type <ival> type returnType 

%define parse.error detailed

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
|   type identifier declarator ';' {$$ = create_quaternary_node(start_current_character, line_number, A_ARRAY_DECL, $1, $2, $3, NULL);}
|   type identifier T_ASSIGN conditionalExpression ';' {$$ = create_ternary_node(start_current_character, line_number, A_VAR_DECL, $1, $2, $4);}
|   type identifier initializerDim T_ASSIGN arrayInitializer ';' {(node = create_quaternary_node(start_current_character, line_number, A_ARRAY_DECL, $1, $2, $3, $5)) ? $$ = node : yyerror("parse error: Wrong dimensionality on initialised array\n");}
|   error ';' {/*yyerror("error in varDeclaration");*/ yyerrok;}
;

initializerDim:
    ///*%empty*/                                      {$$ = linked_list_new();}
    T_LEFT_BRACKET T_RIGHT_BRACKET                  {$$ = linked_list_new(); linked_list_append($$, NULL);}
|   initializerDim T_LEFT_BRACKET T_RIGHT_BRACKET   {linked_list_append($$, NULL); $$ = $1;}
;

declarator:
    ///*%empty*/                                              {$$ = linked_list_new();}
   T_LEFT_BRACKET expression T_RIGHT_BRACKET               {$$ = linked_list_new(); linked_list_append($$, $2);}
|   declarator T_LEFT_BRACKET expression T_RIGHT_BRACKET    {linked_list_append($1, $3); $$ = $1;}
;

arrayInitializer:
    T_LEFT_BRACE initializerList T_RIGHT_BRACE  {$$ = $2;}
;

initializerList:
    /*%empty*/                                  {$$ = linked_list_new();}
|   initializerValue                            {$$ = linked_list_new(); linked_list_append($$, $1);}
|   initializerList T_COMMA initializerValue    {linked_list_append($1, $3); $$ = $1;}
;

initializerValue:
    primary             {$$ = $1;}
|   arrayInitializer    {$$ = $1;}
;

type:
    T_INT_TYPE      {$$ = (int) TYPE_INT;}
|   T_CHAR_TYPE     {$$ = (int) TYPE_CHAR;}
|   T_BOOL_TYPE     {$$ = (int) TYPE_BOOL;}
|   T_VOID_TYPE     {$$ = (int) TYPE_VOID;}
|   T_STRING_TYPE   {$$ = (int) TYPE_STRING;}
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
|   type initializerDim identifier      {$$ = linked_list_new(); linked_list_append($$, create_binary_node(start_current_character, line_number, A_PARAMETER_EXPR, $1, $3)); ((AST_node *)$$->tail->data)->parameter.array = $2->size; free($2);}
|   parameters T_COMMA type identifier  {linked_list_append($1, create_binary_node(start_current_character, line_number, A_PARAMETER_EXPR, $3, $4)); $$ = $1;}
;

args:
    /*%empty*/                      {$$ = linked_list_new();}
|   assignExpression                {$$ = linked_list_new(); linked_list_append($$, $1);}
|   args T_COMMA assignExpression   {linked_list_append($1, $3); $$ = $1;}
;

statement:
    ifStatement         {$$ = $1;}
|   loopStatement       {$$ = $1;}         
|   returnStatement     {$$ = $1;}
|   printStatement      {$$ = $1;}
|   expressionStatement {$$ = $1;}
;

ifStatement:
    T_IF T_LEFT_PAREN expression T_RIGHT_PAREN block else {$$ = create_ternary_node(start_current_character, line_number, A_IF_STMT, $3, $5, $6);}
;

loopStatement:
    whileLoop   {$$ = $1;}
;

whileLoop:
    T_WHILE T_LEFT_PAREN assignExpression T_RIGHT_PAREN block {$$ = create_binary_node(start_current_character, line_number, A_WHILE_LOOP, $3, $5);}
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
/*|   error ';' {printf("error in expression on line %d", line_number); yyerrok;}*/
;

expression:
    assignExpression {$$ = $1;}
;

assignExpression:
    conditionalExpression           {$$ = $1;}
|   identifier T_ASSIGN expression  {$$ = create_binary_node(start_current_character, line_number, A_ASSIGN_EXPR, $1, $3);}
|   identifier declarator T_ASSIGN expression {$$ = create_binary_node(start_current_character, line_number, A_ASSIGN_EXPR, create_binary_node(start_current_character, line_number, A_INDEX_EXPR, $1,$2), $4);}
;

conditionalExpression:
    logicalAND {$$ = $1;}
;

logicalAND:
    logicalOR                               {$$ = $1;}
|   logicalAND T_AND logicalOR   {$$ = create_ternary_node(start_current_character, line_number, A_LOGICAL_EXPR, $1, (void*) A_AND, $3);}
;

logicalOR:
    relationalExpression                    {$$ = $1;}
|   logicalOR T_OR relationalExpression     {$$ = create_ternary_node(start_current_character, line_number, A_LOGICAL_EXPR, $1, (void *) A_OR, $3);}
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
    multiplicativeExpression                          {$$ = $1;}
|   arithmeticExpression '+' multiplicativeExpression {$$ = binexp($1, A_ADD, $3);}
|   arithmeticExpression '-' multiplicativeExpression {$$ = binexp($1, A_SUB, $3);}
;

multiplicativeExpression:
    castExpression
|   multiplicativeExpression '*' castExpression {$$ = binexp($1, A_MULT, $3);}
|   multiplicativeExpression '/' castExpression {$$ = binexp($1, A_DIV, $3);}

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
|   postfixExpression T_LEFT_PAREN args T_RIGHT_PAREN   {$$ = create_binary_node(start_current_character, line_number, A_CALL_EXPR, $1, $3);}
|   postfixExpression T_LEFT_PAREN error T_RIGHT_PAREN  {/*printf("error in negated grouping on line %d", line_number);*/ yyerrok;}
|   identifier declarator                               {$$ = create_binary_node(start_current_character, line_number, A_INDEX_EXPR, $1, $2);}
;

identifier:
    T_IDENTIFIER {$$ = create_binary_node(start_current_character, line_number, A_PRIMARY_EXPR, TYPE_IDENTIFIER, yylval.string.sval); free(yylval.string.sval);
                    /*printf("%s identifier returned to bison at line %d it starts at %ld and ends at %ld\n", $$->primary_expr.identifier_name, line_number, start_current_character, current_character);*/}
;

primary:
    T_INT           {$$ = create_binary_node(start_current_character, line_number, A_PRIMARY_EXPR, TYPE_INT, (void *) yylval.ival); /*printf("%d int value returned to bison at line %d it starts at %ld and ends at %ld\n", yylval.ival, line_number, start_current_character, current_character);*/}
|   T_CHAR          {$$ = create_binary_node(start_current_character, line_number, A_PRIMARY_EXPR, TYPE_CHAR, (void *) yylval.cval); /*printf("%c character returned to bison at line %d it starts at %ld and ends at %ld\n", yylval.cval, line_number, start_current_character, current_character);*/}
|   T_BOOL          {$$ = create_binary_node(start_current_character, line_number, A_PRIMARY_EXPR, TYPE_BOOL, (void *) yylval.ival); /*yylval.ival ? printf("true returned to bison\n") : printf("false returned to bison\n");*/}
|   T_STRING        {$$ = create_ternary_node(start_current_character, line_number, A_PRIMARY_EXPR, TYPE_STRING, (void *) yylval.string.sval, yylval.string.length); free(yylval.string.sval); /*printf("String literal: \"%s\" of length %d\n", $$->primary_expr.string.value, yylval.string.length);*/}
|   T_LEFT_PAREN expression T_RIGHT_PAREN {$$ = $2;}
|   T_LEFT_PAREN error T_RIGHT_PAREN {/*printf("error in grouping on line %d", line_number);*/ yyerrok;}
|   identifier      {$$ = $1;}
;

%%

void yyerror(char const* err) {
    (*bison_errors)++;
    printf("bison_error: %s; line %d, character %ld, tokentype: %d\n", err, line_number, start_current_character, yychar);
}

AST_node* binexp(void* left, int op, void* right) {
    return create_ternary_node(start_current_character, line_number, A_ARITHMETIC_EXPR, left, (void*) op, right);
}

AST_node* relexp(void* left, int op, void* right) {
    return create_ternary_node(start_current_character, line_number, A_RELATIONAL_EXPR, left, (void*) op, right);
}

AST_node* run_bison(const char* filename, int* errors) {
    yyin = fopen(filename,"r");
    if (!yyin) {
        return NULL;
    }
    bison_errors = errors;

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