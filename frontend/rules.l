/***DEFINITIONS***/
%{
    //#include <string.h>
    #include "y.tab.h"
    #define NUM_KEYWORDS 5
    #define NUM_TYPES    3
    #define NUM_DIGRAPHS 7

    extern unsigned short line_number;
    extern long current_character;
    extern long start_current_character;


    const char keywords[NUM_KEYWORDS][10] = {"if", "else", "return", "func", "print"};
    const int  key_tokens[NUM_KEYWORDS]   = {T_IF, T_ELSE, T_RETURN, T_FUNC, T_PRINT};

    const char types[NUM_TYPES][10]   = {"int", "char", "bool"};
    const int  token_types[NUM_TYPES] = {T_INT_TYPE, T_CHAR_TYPE, T_BOOL_TYPE};

    const char digraphs[NUM_DIGRAPHS][3] = {"->", "&&", "||", "<=", ">=", "==", "!="};
    const int  digraph_tokens[NUM_DIGRAPHS] = {T_ARROW, T_AND, T_OR, T_LESS_EQ, T_GREATER_EQ, T_EQUALS, T_NEQUALS};

    int increment_counter(int);
    int find_keyword(char* keywrd);
    int find_type(char* type);
    int find_digraph(char* digraph);
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
KEYWORD             if|else|func|return|print
BOOL_VALUE          true|false
TYPE                int|char|bool|void
SINGLE_OPERATOR     (\+|\-|\*|\/|\!|\<|\>)
OPERATOR            (\-\>|==|\!=|\|\||\&\&|\<=|\>=)
CHARACTER           (.|\n)
WHITESPACES         [\t\r ]+
ILLEGAL_IDENTIFIER  [0-9]([_a-zA-Z0-9]*)


%s STR COMMENT
/***RULES***/
%%

<INITIAL>{COMMENT_START} {
    increment_counter(yyleng);
    BEGIN(COMMENT);
}

<COMMENT>{COMMENT_END} {
    printf("multi line comment\n");
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

<STR><<EOF>> {
    return YYerror;
}

<STR>{CHARACTER} {
    printf("%s \tcharacter\n", yytext);
    increment_counter(yyleng);
    yylval.cval = yytext[0];
    return T_CHAR;
}

<INITIAL>{SINGLE_LINE_COMMENT} {
    printf("%s \tsingle line comment\n", yytext);
    increment_counter(yyleng);
}

<COMMENT>.* {
    printf("%s\n", yytext);
    increment_counter(yyleng);
}

<INITIAL>{LEFT_PAR} {
    printf("%s \tleft parentheses\n", yytext);
    increment_counter(yyleng);
    return T_LEFT_PAREN;
}

<INITIAL>{RIGHT_PAR} {
    printf("%s \tright parentheses\n", yytext);
    increment_counter(yyleng);
    return T_RIGHT_PAREN;
}

<INITIAL>{LEFT_BRACE} {
    printf("%s \tleft brace\n", yytext);
    increment_counter(yyleng);
    return T_LEFT_BRACE;
}

<INITIAL>{RIGHT_BRACE} {
    printf("%s \tright brace\n", yytext);
    increment_counter(yyleng);
    return T_RIGHT_BRACE;
}

<INITIAL>{COMMA} {
    printf("%s \tcomma\n", yytext);
    increment_counter(yyleng);
    return T_COMMA;
}

<INITIAL>{INTEGER} {
    printf("%s \tinteger\n", yytext);
    increment_counter(yyleng);
    yylval.ival = atoi(yytext);
    return T_INT;
}

<INITIAL>{BOOL_VALUE} {
    printf("%s \tboolean value\n", yytext);
    increment_counter(yyleng);
    if (!strcmp("true", yytext)) {
        yylval.ival = 1;
    } else {
        yylval.ival = 0;
    }
    return T_BOOL;
}

<INITIAL>{KEYWORD} {
    printf("%s \tkeyword\n", yytext);
    increment_counter(yyleng);
    return find_keyword(yytext);
}


<INITIAL>{TYPE} {
    printf("%s \ttype\n", yytext);
    increment_counter(yyleng);
    return find_type(yytext);
}

<INITIAL>{IDENTIFIER} {
    printf("%s \tidentifier, length: %d\n", yytext, yyleng);
    increment_counter(yyleng);
    yylval.sval = calloc(yyleng + 1, sizeof(char));
    //memset(yylval.sval, 0, (yyleng + 1) *sizeof(char));
    printf("%s \tyylval post memset, length: %ld\n", yylval.sval, strlen(yylval.sval) + 1);
    strncpy(yylval.sval, yytext, yyleng);
    printf("%s \tyylval efter copying, length: %ld\n", yylval.sval, strlen(yylval.sval));
    return T_IDENTIFIER;
}

<INITIAL>= {
    printf("%s \tassign\n", yytext);
    increment_counter(yyleng);
    return T_ASSIGN;
}

<INITIAL>{SINGLE_OPERATOR} {
    printf("%s \tsingle operator\n", yytext);
    increment_counter(yyleng);
    return yytext[0];
}

<INITIAL>{OPERATOR} {
    printf("%s \tdigraph operator\n", yytext);
    increment_counter(yyleng);
    return find_digraph(yytext);
}

<INITIAL>";" {
    printf("%s \tsemicolon\n", yytext);
    increment_counter(yyleng);
    return ';';
}

<INITIAL,COMMENT>"\n"    {
    line_number++;
    increment_counter(yyleng);
}

<INITIAL>{WHITESPACES} {
    printf("%s \twhitespace\n", yytext); 
    increment_counter(yyleng);
}

<<EOF>> {
    printf("%s \tEOF\n", yytext);
    return 0; 
}

<INITIAL>{ILLEGAL_IDENTIFIER} {
    increment_counter(yyleng);
    return YYUNDEF;
}

%%

/***CODE***/

/***yywrap idk bro***/
int yywrap(void) {return 1;}

void non_dogshit_memset() {
    ;
}

int increment_counter(int len) {
    if (current_character == start_current_character) {
        current_character += len;
    } else {
        start_current_character = current_character;
        current_character += len;
    }
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

int find_digraph(char* digraph) {
    for (int i = 0; i < NUM_DIGRAPHS; i++) {
        if (!strcmp(digraphs[i], digraph)) {
            return digraph_tokens[i];
        }
    }
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
