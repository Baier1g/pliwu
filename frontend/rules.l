/***DEFINITIONS***/
%{
    #include <stdlib.h>
    #include "grammar.tab.h"
    unsigned short line_number = 1;
    long current_character = 1;
    int size = 1000;

    int increment_counter(int);
%}

COMMENT_START       \/\*
COMMENT_END         \*\/
SINGLE_LINE_COMMENT \/\/(.*)
LEFT_PAR            \(
RIGHT_PAR           \)
LEFT_BRACE          \{
RIGHT_BRACE         \}
COMMA               \,
DIGIT               [0-9]
INTEGER             {DIGIT}+
FLOAT               {INTEGER}\.{INTEGER}
IDENTIFIER          [a-zA-Z]([_a-zA-Z0-9]*)
KEYWORD             if|else|func|return
TYPE                int|char|bool
OPERATOR            (\+|\-|\*|\/|\-\>|==|\!=|\!|\|\||\&\&)
CHARACTER           (.|\n)
WHITESPACES         [\t\r ]+


%s STR COMMENT
/***RULES***/
%%
<INITIAL>{COMMENT_START} {
    increment_counter(yyleng);
    BEGIN(COMMENT);
}

<COMMENT>{COMMENT_END} {
    increment_counter(yyleng);
    BEGIN(INITIAL);
}

<INITIAL>\" {;
    increment_counter(yyleng);
    BEGIN(STR);
}

<STR>\" {
    increment_counter(yyleng);
    BEGIN(INITIAL);
}

<STR>{CHARACTER} {
    printf("%s character\n", yytext);
    increment_counter(yyleng);
}

<INITIAL>{SINGLE_LINE_COMMENT} {
    printf("%s single line comment\n", yytext);
    increment_counter(yyleng);
}

<COMMENT>.* {
    printf("%s multi line comment\n", yytext);
    increment_counter(yyleng);
}

<INITIAL>{LEFT_PAR} {
    printf("%s left parentheses\n", yytext);
    increment_counter(yyleng); 
}

<INITIAL>{RIGHT_PAR} {
    printf("%s right parentheses\n", yytext);
    increment_counter(yyleng);
}

<INITIAL>{LEFT_BRACE} {
    printf("%s left brace\n", yytext);
    increment_counter(yyleng);
}

<INITIAL>{RIGHT_BRACE} {
    printf("%s right brace\n", yytext);
    increment_counter(yyleng);
}

<INITIAL>{COMMA} {
    printf("%s comma\n", yytext);
    increment_counter(yyleng);
}

<INITIAL>{INTEGER} {
    printf("%s integer\n", yytext);
    increment_counter(yyleng);
}

<INITIAL>{KEYWORD} {
    printf("%s keyword\n", yytext);
    increment_counter(yyleng);
}

<INITIAL>{FLOAT} {
    printf("%s float\n", yytext);
    increment_counter(yyleng);
}

<INITIAL>{TYPE} {
    printf("%s type\n", yytext);
    increment_counter(yyleng);
}

<INITIAL>{IDENTIFIER} {
    printf("%s identifier\n", yytext);
    increment_counter(yyleng);
}

<INITIAL>= {
    printf("%s assign\n", yytext);
    increment_counter(yyleng);
}

<INITIAL>{OPERATOR} {
    printf("%s operator\n", yytext);
    increment_counter(yyleng);
}

<INITIAL>";" {
    printf("%s semicolon\n", yytext);
    increment_counter(yyleng);
}

<INITIAL,COMMENT>"\n"    {
    line_number++;
    increment_counter(yyleng);
}

<INITIAL>{WHITESPACES} {
    printf("%s whitespace\n", yytext); 
    increment_counter(yyleng);
}

<<EOF>> {
    printf("%s EOF\n", yytext);
    return 0; 
}

<INITIAL>.     { printf("%s report errors, bruv\n", yytext); increment_counter(yyleng); }

%%

/***CODE***/

/***yywrap idk bro***/
int yywrap(void){}

int increment_counter(int len) {
    current_character += len;
}

/***main functions, reads files and processes contents***/
int main(int argc, char** argv) {   
    FILE *fp;
    char *filename = argv[1];

    fp = fopen(filename,"r");
    if (!fp) {
        return -2;
    }
    yyin = fp;

    yylex();
    printf("\nNumber of lines in the file - %u\n", line_number);
    printf("\nNumber of characters in the file - %ld\n", current_character);
    return 0;
}
