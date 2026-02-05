/* PROLOGUE */
%{
    #include <stdlib.h>

%}

/* DECLARATIONS */
/* SINGLE CHARACTER TOKENS */
%token T_RIGHT_BRACE
%token T_LEFT_BRACE
%token T_RIGHT_PAREN
%token T_LEFT_PAREN
%token T_COMMA
%token T_ASSIGN
%token ';'
%left  '|' '&'
%left  '+' '-'
%left  '*' '/'

/* DOUBLE CHARACTER TOKENS */
%token T_IF
%token T_ARROW
%token T_OR T_AND
%left  T_LESS_EQ T_GREATER_EQ T_EQUALS

/* LONGER TOKENS */
%token T_RETURN
%token T_ELSE
%token T_INT T_CHAR T_BOOL
%token T_INT_TYPE
%token T_BOOL_TYPE
%token T_CHAR_TYPE
%token T_IDENTIFIER
%token T_FUNCTION

/* GRAMMAR RULES */
%%
program:
    %empty
|   program declaration
;

declaration:
    varDeclaration
|   funcDeclaration
|   statement
;

varDeclaration:
    type T_IDENTIFIER T_ASSIGN expression ';'
;

type:
    T_INT_TYPE
|   T_CHAR_TYPE
|   T_BOOL_TYPE
;

funcDeclaration:
    T_FUNCTION function;

function:
    T_IDENTIFIER T_LEFT_PAREN parameters T_RIGHT_PAREN T_ARROW type block;

parameters:
    %empty
|   type T_IDENTIFIER
|   parameters T_COMMA type T_IDENTIFIER
;

functionCall:
    T_IDENTIFIER T_LEFT_PAREN args T_RIGHT_PAREN
;

args:
    %empty
|   args primary
|   args binaryExpression
|   args T_COMMA primary
|   args T_COMMA binaryExpression
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
    primary
|   T_IDENTIFIER T_ASSIGN expression
|   functionCall
|   binaryExpression
|   unaryExpression
;



binaryExpression:
    expression '+' primary
|   expression '-' primary
|   expression '*' primary
|   expression '/' primary
|   expression T_LESS_EQ primary
|   expression T_GREATER_EQ primary
|   expression T_EQUALS primary
|   expression T_AND primary
|   expression T_OR primary
;

unaryExpression:
    '-' T_INT
|   '-' T_BOOL
;

primary:
    T_INT
|   T_CHAR
|   T_BOOL
|   T_IDENTIFIER
;

%%

/* EPILOGUE */