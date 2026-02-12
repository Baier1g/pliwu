
/* PROLOGUE */
%{
    int yylex(void);
    #include <stdlib.h>
    extern int yylex(void);
    void yyerror(char const*);
%}

/* DECLARATIONS */
%union {
    double fval;
    int ival;
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
    %empty
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
    T_IDENTIFIER T_LEFT_PAREN parameters T_RIGHT_PAREN T_ARROW type block;

parameters:
    %empty
|   type T_IDENTIFIER
|   parameters T_COMMA type T_IDENTIFIER
;

args:
    %empty
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
    %empty
|   T_ELSE T_LEFT_PAREN expression T_RIGHT_PAREN block
;

returnStatement:
    T_RETURN expressionStatement
;

block:
    T_LEFT_BRACE blockBody T_RIGHT_BRACE
;

blockBody:
    %empty
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
    T_INT
|   T_CHAR
|   T_BOOL
|   T_IDENTIFIER
|   T_LEFT_PAREN expression T_RIGHT_PAREN
;

%%

/* EPILOGUE */