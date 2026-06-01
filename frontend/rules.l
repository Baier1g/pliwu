/***DEFINITIONS***/
%{
    //#include <string.h>
    #include "grammar.tab.h"
    #define NUM_KEYWORDS 6
    #define NUM_TYPES    4
    #define NUM_DIGRAPHS 7
    #define STRING_MAX_LENGTH 10000
    #define IDENTIFIER_MAX_LENGTH 128

    extern unsigned short line_number;
    extern long current_character;
    extern long start_current_character;

    char* stringbuffer;
    int stringptr;

    const char keywords[NUM_KEYWORDS][10] = {"if", "else", "while", "return", "func", "print"};
    const int  key_tokens[NUM_KEYWORDS]   = {T_IF, T_ELSE, T_WHILE, T_RETURN, T_FUNC, T_PRINT};

    const char types[NUM_TYPES][10]   = {"int", "char", "bool", "string"};
    const int  token_types[NUM_TYPES] = {T_INT_TYPE, T_CHAR_TYPE, T_BOOL_TYPE, T_STRING_TYPE};

    const char digraphs[NUM_DIGRAPHS][3] = {"->", "&&", "||", "<=", ">=", "==", "!="};
    const int  digraph_tokens[NUM_DIGRAPHS] = {T_ARROW, T_AND, T_OR, T_LESS_EQ, T_GREATER_EQ, T_EQUALS, T_NEQUALS};

    void increment_counter(int);
    void increment_line();
    int find_keyword(char* keywrd);
    int find_type(char* type);
    int find_digraph(char* digraph);

    void yyerror(char const*);
%}

COMMENT_START       \/\*
COMMENT_END         \*\/
SINGLE_LINE_COMMENT \/\/(.*)
LEFT_PAR            \(
RIGHT_PAR           \)
LEFT_BRACE          \{
RIGHT_BRACE         \}
LEFT_BRACKET        \[
RIGHT_BRACKET       \]
COMMA               \,
DIGIT               [0-9]
INTEGER             {DIGIT}+
FLOAT               {INTEGER}\.{INTEGER}
IDENTIFIER          [a-zA-Z]([_a-zA-Z0-9]*)
KEYWORD             if|else|while|func|return|print
BOOL_VALUE          true|false
TYPE                int|char|bool|void|string
SINGLE_OPERATOR     (\+|\-|\*|\/|\!|\<|\>)
OPERATOR            (\-\>|==|\!=|\|\||\&\&|\<=|\>=)
CHARACTER           (\\n|\\t|\n|.)
CHAR                \'{CHARACTER}\'
WHITESPACES         [\t\r ]+
ILLEGAL_IDENTIFIER  [_0-9]([_a-zA-Z0-9]*)
REST                \n|.


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

<INITIAL>{CHAR} {
    if (yyleng == 2) {
        if (yytext[1] == 'n') {yylval.cval = '\n';}
        if (yytext[1] == 't') {yylval.cval = '\t';}
    } else {
        yylval.cval = yytext[1];
    }
    if (yytext[0] == '\n') {
        increment_line();
    }
    return T_CHAR;
}

<INITIAL>\" {;
    increment_counter(yyleng);
    BEGIN(STR);
    stringbuffer = calloc(STRING_MAX_LENGTH, sizeof(char));
    stringptr = 0;
}

<STR>\" {
    increment_counter(yyleng);
    yylval.string.sval = (char *) calloc(stringptr + 1, sizeof(char));
    yylval.string.length = stringptr + 1;
    strncpy(yylval.string.sval, stringbuffer, yylval.string.length);
    BEGIN(INITIAL);
    return T_STRING;
}

<STR><<EOF>> {
    yyerror("non-terminated string");
    BEGIN(INITIAL);
    return YYerror;
}

<STR>{CHARACTER} {
    increment_counter(yyleng);

    if(stringptr > STRING_MAX_LENGTH) {
        printf("String too long on line %d \n", line_number);
        exit(-1);
    }

    if (yyleng == 2) {
        if (yytext[1] == 'n') {stringbuffer[stringptr++] = '\n';}
        if (yytext[1] == 't') {stringbuffer[stringptr++] = '\t';}
    } else {
        stringbuffer[stringptr++] = yytext[0];
    }
    if (yytext[0] == '\n') {
        increment_line();
    }
}

<INITIAL>{SINGLE_LINE_COMMENT} {
    increment_counter(yyleng);
}

<COMMENT>. {
    increment_counter(yyleng);
}

<INITIAL>{LEFT_PAR} {
    increment_counter(yyleng);
    return T_LEFT_PAREN;
}

<INITIAL>{RIGHT_PAR} {
    increment_counter(yyleng);
    return T_RIGHT_PAREN;
}

<INITIAL>{LEFT_BRACE} {
    increment_counter(yyleng);
    return T_LEFT_BRACE;
}

<INITIAL>{RIGHT_BRACE} {
    increment_counter(yyleng);
    return T_RIGHT_BRACE;
}

<INITIAL>{LEFT_BRACKET} {
    increment_counter(yyleng);
    return T_LEFT_BRACKET;
}

<INITIAL>{RIGHT_BRACKET} {
    increment_counter(yyleng);
    return T_RIGHT_BRACKET;
}

<INITIAL>{COMMA} {
    increment_counter(yyleng);
    return T_COMMA;
}

<INITIAL>{INTEGER} {
    increment_counter(yyleng);
    yylval.ival = (long long int) atoll(yytext);
    return T_INT;
}

<INITIAL>{BOOL_VALUE} {
    increment_counter(yyleng);
    if (!strcmp("true", yytext)) {
        yylval.ival = 1;
    } else {
        yylval.ival = 0;
    }
    return T_BOOL;
}

<INITIAL>{KEYWORD} {
    increment_counter(yyleng);
    return find_keyword(yytext);
}


<INITIAL>{TYPE} {
    increment_counter(yyleng);
    return find_type(yytext);
}

<INITIAL>{IDENTIFIER} {
    if (yyleng > IDENTIFIER_MAX_LENGTH) {
        printf("Identifier name too long (%d characters) on line %d\n", yyleng, line_number);
        exit(-1);
    }
    increment_counter(yyleng);
    yylval.string.sval = calloc(yyleng + 1, sizeof(char));
    yylval.string.length = yyleng;
    strncpy(yylval.string.sval, yytext, yyleng);
    return T_IDENTIFIER;
}

<INITIAL>= {
    increment_counter(yyleng);
    return T_ASSIGN;
}

<INITIAL>{SINGLE_OPERATOR} {
    increment_counter(yyleng);
    return yytext[0];
}

<INITIAL>{OPERATOR} {
    increment_counter(yyleng);
    return find_digraph(yytext);
}

<INITIAL>";" {
    increment_counter(yyleng);
    return ';';
}

<INITIAL,COMMENT>"\n"    {
    increment_line();
    increment_counter(yyleng);
}

<INITIAL>{WHITESPACES} {
    increment_counter(yyleng);
}

<<EOF>> {
    return 0; 
}

<INITIAL>{ILLEGAL_IDENTIFIER} {
    increment_counter(yyleng);
    return YYUNDEF;
}

<INITIAL>{REST} {
    increment_counter(yyleng);
    return YYUNDEF;
}

%%

/***CODE***/

//unsupported
int yywrap(void) {return 1;}

void increment_counter(int len) {
    if (current_character+len < current_character) {
        //overflow case
        puts("flex_error: file has too many characters");
        exit(-1);
    }
    if (current_character == start_current_character) {
        current_character += len;
    } else {
        start_current_character = current_character;
        current_character += len;
    }
}

void increment_line(){
    line_number++;
    if (line_number == 0) {
        //overflow case
        puts("flex_error: file has too many lines"); 
        exit(-1);
    }
}

/* This function returns the specific keyword token from a string */
int find_keyword(char* keywrd) {
    for (int i = 0; i < NUM_KEYWORDS; i++) {
        if (!strcmp(keywords[i], keywrd)) {
            return key_tokens[i];
        }
    }
    return -1;
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
    return -1;
}
