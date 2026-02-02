/***DEFINITIONS***/
%{
    #include <stdlib.h>
    #include <string.h>
    #include "token.h"
    unsigned short line_number = 1;
    long current_character = 1;
    int size = 1000;
    int token_index = 0;
    token** tokens;

    extern token* create_token(long, unsigned short, enum TokenType, char*, void*, size_t);
    extern void print_token(token*);
    int increment_counter(char*);
%}

LEFT_PAR    \(
RIGHT_PAR   \)
LEFT_BRACE  \{
RIGHT_BRACE \}
COMMA       \,
DIGIT       [0-9]
INTEGER     {DIGIT}+
FLOAT       {INTEGER}\.{INTEGER}
IDENTIFIER  [a-zA-Z]([a-zA-Z0-9]*)
KEYWORD     if|else|while|for|func|return
TYPE        int|char|string|float|bool
OPERATOR    \+|\-|\*|\/|\-\>|==|\!=|\!|\|\||\&\&|\&|\|
STRING      \"(.*)\"



/***RULES***/
%%
{LEFT_PAR} {
    printf("%d", LEFT_PARENTHESES);
    tokens[token_index] = create_token(current_character, line_number, LEFT_PARENTHESES, yytext, NULL, 0);
    print_token(*(tokens + token_index++));
    current_character++; 
}

{RIGHT_PAR} {
    *(tokens + token_index) = create_token(current_character, line_number, RIGHT_PARENTHESES, yytext, NULL, 0); 
    print_token(*(tokens + token_index++));
    current_character++;
}

{LEFT_BRACE} {
    *(tokens + token_index) = create_token(current_character, line_number, LEFT_BRACE, yytext, NULL, 0); 
    print_token(*(tokens + token_index++));
    current_character++;
}

{RIGHT_BRACE} {
    *(tokens + token_index) = create_token(current_character, line_number, RIGHT_BRACE, yytext, NULL, 0); 
    print_token(*(tokens + token_index++));
    current_character++;
}

{COMMA} {
    *(tokens + token_index) = create_token(current_character, line_number, COMMA, yytext, NULL, 0); 
    print_token(*(tokens + token_index++));
    current_character++;
}

{INTEGER} {
    printf("%s integer\n", yytext);
    increment_counter(yytext);
}

{KEYWORD} {
    printf("%s keyword\n", yytext);
    increment_counter(yytext);
}

{FLOAT} {
    printf("%s float\n", yytext);
    increment_counter(yytext);
}

{TYPE} {
    printf("%s type\n", yytext);
    increment_counter(yytext);
}

{IDENTIFIER} {
    printf("%s identifier\n", yytext);
    increment_counter(yytext);
}

= {
    printf("%s assign\n", yytext);
    current_character++;
}

{OPERATOR} {
    printf("%s operator\n", yytext);
    increment_counter(yytext);
}

{STRING} {
    printf("%s string\n", yytext);
    increment_counter(yytext);
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

int increment_counter(char* str) {
    current_character += strlen(str);
}

/***main functions, reads files and processes contents***/
int main(int argc, char** argv)  {   
    tokens = malloc(sizeof(struct token*) * size);
    if (!tokens) {
        return -1;
    }

    FILE *fp;
    char *filename = argv[1];

    fp = fopen(filename,"r");
    if (!fp) {
        return -2;
    }
    yyin = fp;

    yylex();
    printf("\nNumber of lines in the file - %ud\n", line_number);
    printf("\nNumber of characters in the file - %ld\n", current_character);
    for (int i = 0; i < token_index; i++) {
        free(*(tokens + i));
    }
    free(tokens);
    return 0;
}
