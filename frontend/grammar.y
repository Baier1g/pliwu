
/* PROLOGUE */
%{
    int yylex(void);
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    extern int yylex(void);
    void yyerror(char const*);
    extern FILE *yyin;
%}

/* DECLARATIONS */
%union {
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

%token T_INT T_CHAR T_BOOL
%token T_INT_TYPE
%token T_BOOL_TYPE
%token T_CHAR_TYPE
%token T_VOID_TYPE
%token T_IDENTIFIER


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
    type T_IDENTIFIER T_ASSIGN expression ';'
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
    T_IDENTIFIER T_LEFT_PAREN parameters T_RIGHT_PAREN returnType block;

returnType:
    /*%empty*/ 
|   T_ARROW type
;

parameters:
    /*%empty*/
|   type T_IDENTIFIER
|   parameters T_COMMA type T_IDENTIFIER
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
;

assignExpression:
    conditionalExpression
|   T_IDENTIFIER T_ASSIGN expression
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

primary:
    T_INT           {printf("%d int value returned to bison\n", yylval.ival);}
|   T_CHAR          {printf("%c character returned to bison\n", yylval.cval);}
|   T_BOOL          {yylval.ival ? printf("true returned to bison\n") : printf("false returned to bison\n");}
|   T_IDENTIFIER    {printf("%s identifier returned to bison\n", yylval.sval);}
|   T_LEFT_PAREN expression T_RIGHT_PAREN
;

%%

void yyerror(char const* err) {
    printf("Shit fucked at %s\n", err);
}

int main(int argc, char* argv[]) {
    FILE *fp;
    char *filename = argv[1];

    yyin = fopen(filename,"r");
    if (!yyin) {
        return -2;
    }

    yyparse();
    return 0;
}

/* EPILOGUE */