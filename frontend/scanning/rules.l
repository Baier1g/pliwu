/***DEFINITIONS***/
%{
    #include <stdlib.h>
    #include <string.h>
    unsigned short line_number = 1;
    long current_character = 1;
    int size = 1000;

    int increment_counter(int);
%}

SINGLE_LINE_COMMENT \/\/(.*)
/* HØJ LEVEL GØGL */
MULTI_LINE_COMMENT  \/\*((.*\n*)*)\*\\
LEFT_PAR            \(
RIGHT_PAR           \)
LEFT_BRACE          \{
RIGHT_BRACE         \}
COMMA               \,
DIGIT               [0-9]
INTEGER             {DIGIT}+
FLOAT               {INTEGER}\.{INTEGER}
IDENTIFIER          [a-zA-Z]([a-zA-Z0-9]*)
KEYWORD             if|else|while|for|func|return
TYPE                int|char|string|float|bool
OPERATOR            (\+|\-|\*|\/|\-\>|==|\!=|\!|\|\||\&\&|\&|\|)
STRING              \"(.*\n*)*\"



/***RULES***/
%%
{SINGLE_LINE_COMMENT} {
    printf("%s single line comment\n", yytext);
    increment_counter(yyleng);
}

{MULTI_LINE_COMMENT} {
    printf("%s multi line comment\n", yytext);
    increment_counter(yyleng);
}

{LEFT_PAR} {
    printf("%s left parentheses\n", yytext);
    increment_counter(yyleng); 
}

{RIGHT_PAR} {
    printf("%s right parentheses\n", yytext);
    increment_counter(yyleng);
}

{LEFT_BRACE} {
    printf("%s left brace\n", yytext);
    increment_counter(yyleng);
}

{RIGHT_BRACE} {
    printf("%s right brace\n", yytext);
    increment_counter(yyleng);
}

{COMMA} {
    printf("%s comma\n", yytext);
    increment_counter(yyleng);
}

{INTEGER} {
    printf("%s integer\n", yytext);
    increment_counter(yyleng);
}

{KEYWORD} {
    printf("%s keyword\n", yytext);
    increment_counter(yyleng);
}

{FLOAT} {
    printf("%s float\n", yytext);
    increment_counter(yyleng);
}

{TYPE} {
    printf("%s type\n", yytext);
    increment_counter(yyleng);
}

{IDENTIFIER} {
    printf("%s identifier\n", yytext);
    increment_counter(yyleng);
}

= {
    printf("%s assign\n", yytext);
    current_character++;
}

{OPERATOR} {
    printf("%s operator\n", yytext);
    increment_counter(yyleng);
}

{STRING} {
    printf("%s string\n", yytext);
    increment_counter(yyleng);
}

; {
    printf("%s semicolon\n", yytext);
    current_character++;
}

\n    {
    line_number++;
    current_character++;
}

<<EOF>> {
    printf("%s EOF", yytext);
    return 0; 
}

.     { current_character++; }

%%

/***CODE***/

/***yywrap idk bro***/
int yywrap(void){}

int increment_counter(int len) {
    current_character += len;
}

/***main functions, reads files and processes contents***/
int main(int argc, char** argv)  {   

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
