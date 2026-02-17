
/* PROLOGUE */
%{
    int yylex(void);
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "ast.h"
    extern int yylex(void);
    void yyerror(char const*);
    extern FILE *yyin;
    unsigned short line_number = 1;
    long current_character = 1;
    long start_current_character = 1;
%}

/* DECLARATIONS */
%union {
    struct AST_node* nval;
    double fval;
    int ival;
    char cval;
    char* sval;
} 
/* SINGLE CHARACTER TOKENS */
%token T_RIGHT_BRACE
%token T_LEFT_BRACE
%token T_RIGHT_PAREN
%token T_LEFT_PAREN
%token T_COMMA
%token T_ASSIGN
%token ';'
%left  '+' '-'
%left  '*' '/'
%left  '<' '>'
%right '!'

/* DOUBLE CHARACTER TOKENS */
%token T_ARROW
%token T_OR T_AND
%left  T_LESS_EQ T_GREATER_EQ T_EQUALS T_NEQUALS

/* LONGER TOKENS */
%token T_IF
%token T_ELSE
%token T_RETURN
%token T_FUNC

%token <ival> T_INT 
%token <cval> T_CHAR
%token <ival> T_BOOL
%token T_INT_TYPE
%token T_BOOL_TYPE
%token T_CHAR_TYPE
%token T_VOID_TYPE
%token <sval> T_IDENTIFIER

%type <nval> identifier
%type <nval> primary
%type <nval> expression


/* GRAMMAR RULES */
%%
program:
    /*%empty*/
|   program module
;

module:
    funcDefinition
|   declaration
;

declaration:
    varDeclaration
|   statement
;

varDeclaration:
    type identifier ';'
|   type identifier T_ASSIGN expression ';'
;

type:
    T_INT_TYPE
|   T_CHAR_TYPE
|   T_BOOL_TYPE
|   T_VOID_TYPE
;

funcDefinition:
    T_FUNC function;

function:
    identifier T_LEFT_PAREN parameters T_RIGHT_PAREN returnType block
; 

returnType:
    /*%empty*/ 
|   T_ARROW type
;

parameters:
    /*%empty*/
|   type identifier
|   parameters T_COMMA type identifier
;

args:
    /*%empty*/
|   assignExpression
|   args T_COMMA assignExpression
;

statement:
    ifStatement             
|   returnStatement
|   expressionStatement
|   error statement
;

ifStatement:
    T_IF T_LEFT_PAREN expression T_RIGHT_PAREN block else
;

else:
    /*%empty*/
|   T_ELSE block
;

returnStatement:
    T_RETURN expression ';'
;

block:
    T_LEFT_BRACE blockBody T_RIGHT_BRACE
;

blockBody:
    /*%empty*/
|   blockBody declaration
;

expressionStatement:
    expression ';'
;

expression:
    assignExpression
|   error             
;

assignExpression:
    conditionalExpression
|   identifier T_ASSIGN expression
;

conditionalExpression:
    logicalOR
;

logicalOR:
    logicalAND
|   logicalOR T_OR logicalAND
;

logicalAND:
    equalityExpression
|   logicalAND T_AND equalityExpression
;

equalityExpression:
    relationalExpression
|   equalityExpression T_EQUALS relationalExpression
|   equalityExpression T_NEQUALS relationalExpression
;

relationalExpression:
    additiveExpression
|   relationalExpression '<' additiveExpression
|   relationalExpression '>' additiveExpression
|   relationalExpression T_LESS_EQ additiveExpression
|   relationalExpression T_GREATER_EQ additiveExpression
;

additiveExpression:
    multiplicativeExpression
|   additiveExpression '+' multiplicativeExpression
|   additiveExpression '-' multiplicativeExpression
;

multiplicativeExpression:
    castExpression
|   multiplicativeExpression '*' castExpression
|   multiplicativeExpression '/' castExpression
;

castExpression:
    unaryExpression
;

unaryExpression:
    postfixExpression
|   '-' postfixExpression
|   '!' postfixExpression
;

postfixExpression:
    primary
|   postfixExpression T_LEFT_PAREN args T_RIGHT_PAREN
;

identifier:
    T_IDENTIFIER {$$ = create_binary_node(start_current_character, line_number, A_PRIMARY_EXPR, (void *)TYPE_IDENTIFIER, yylval.sval);
        printf("%s identifier returned to bison at line %d it starts at %ld and ends at %ld\n", yylval.sval, line_number, start_current_character, current_character);}
;

primary:
    T_INT           {printf("%d int value returned to bison at line %d it starts at %ld and ends at %ld\n", yylval.ival, line_number, start_current_character, current_character);}
|   T_CHAR          {printf("%c character returned to bison at line %d it starts at %ld and ends at %ld\n", yylval.cval, line_number, start_current_character, current_character);}
|   T_BOOL          {yylval.ival ? printf("true returned to bison\n") : printf("false returned to bison\n");}
|   T_LEFT_PAREN expression T_RIGHT_PAREN {$$ = $2;}
|   identifier      {$$ = $1;}
;

%%

void yyerror(char const* err) {
    printf("%s: line: %d, character: %ld, token: %s\n", err, line_number, start_current_character, "skill issue");
}

int main(int argc, char* argv[]) {
    FILE *fp;
    char *filename = argv[1];

    yyin = fopen(filename,"r");
    if (!yyin) {
        return -2;
    }

    yyparse();
    printf("\nNumber of lines in the file - %u\n", line_number);
    printf("\nNumber of characters in the file - %ld\n", current_character);
    return 0;
}

/* EPILOGUE */