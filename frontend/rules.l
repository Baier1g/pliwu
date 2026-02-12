/***DEFINITIONS***/
%{
    #include <string.h>
    #include "grammar.tab.h"
    #define NUM_KEYWORDS 4
    #define NUM_TYPES    3

    unsigned short line_number = 1;
    long current_character = 1;


    const char keywords[NUM_KEYWORDS][10] = {"if", "else", "return", "func"};
    const int  key_tokens[NUM_KEYWORDS]   = {T_IF, T_ELSE, T_RETURN, T_FUNC};

    const char types[NUM_TYPES][10]   = {"int", "char", "bool"};
    const int  token_types[NUM_TYPES] = {T_INT_TYPE, T_CHAR_TYPE, T_BOOL_TYPE};

    int increment_counter(int);
    int find_keyword(char* keywrd);
    int find_type(char* type);
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
SINGLE_OPERATOR     [\+\-\*\/\!\<\>\&\|]
OPERATOR            (\-\>|==|\!=|\|\||\&\&)
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
    return T_LEFT_PAREN;
}

<INITIAL>{RIGHT_PAR} {
    printf("%s right parentheses\n", yytext);
    increment_counter(yyleng);
    return T_RIGHT_PAREN;
}

<INITIAL>{LEFT_BRACE} {
    printf("%s left brace\n", yytext);
    increment_counter(yyleng);
    return T_LEFT_BRACE;
}

<INITIAL>{RIGHT_BRACE} {
    printf("%s right brace\n", yytext);
    increment_counter(yyleng);
    return T_RIGHT_BRACE;
}

<INITIAL>{COMMA} {
    printf("%s comma\n", yytext);
    increment_counter(yyleng);
    return T_COMMA;
}

<INITIAL>{INTEGER} {
    printf("%s integer\n", yytext);
    increment_counter(yyleng);
    yylval.ival = atoi(yytext);
    return T_INT;
}

<INITIAL>{KEYWORD} {
    printf("%s keyword\n", yytext);
    increment_counter(yyleng);
    return find_keyword(yytext);
}


<INITIAL>{TYPE} {
    printf("%s type\n", yytext);
    increment_counter(yyleng);
    return find_type(yytext);
}

<INITIAL>{IDENTIFIER} {
    printf("%s identifier\n", yytext);
    increment_counter(yyleng);
    strncpy(yylval.sval, yytext, yyleng);
    return T_IDENTIFIER;
}

<INITIAL>= {
    printf("%s assign\n", yytext);
    increment_counter(yyleng);
    return T_ASSIGN;
}

<INITIAL>{SINGLE_OPERATOR} {
    return yytext[0];
}

<INITIAL>{OPERATOR} {
    printf("%s operator\n", yytext);
    increment_counter(yyleng);
}

<INITIAL>";" {
    printf("%s semicolon\n", yytext);
    increment_counter(yyleng);
    return ';';
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
int yywrap(void) {}

int increment_counter(int len) {
    current_character += len;
}

/*  This function returns the specific keyword 
    token from a string */
int find_keyword(char* keywrd) {
    for (int i = 0; i < NUM_KEYWORDS; i++) {
        if (!strcmp(keywords[i], keywrd)) {
            return key_tokens[i];
        }
    }
}

int find_type(char* type) {
    for (int i = 0; i < NUM_TYPES; i++) {
        if (!strcmp(types[i], type)) {
            return token_types[i];
        }
    }
    return T_VOID_TYPE;
}

/***main functions, reads files and processes contents***/
/***int main(int argc, char** argv) {   
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
}***/
