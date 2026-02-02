#include <string.h>
#include <stdio.h>
#include "token.h"

token* create_token(long count, unsigned short line, enum TokenType type, char *lexeme, void *literal, size_t size)
{
    token *tok = malloc(sizeof(struct token));
    if (!tok) {
        return NULL;
    }

    tok->character_count = count;
    tok->line = line;
    tok->type = type;

    tok->lexeme = malloc(sizeof(char *) * strlen(lexeme));
    if (!tok->lexeme) {
        free(tok);
        return NULL;
    }
    strncpy(lexeme, tok->lexeme, strlen(lexeme));

    if (literal) {
        tok->literal = malloc(size);
        if (!tok->literal) {
            free(tok->lexeme);
            free(tok);
            return NULL;
        }
    }

    return tok;
}

void delete_token(token* tok) {
    free(tok->literal);
    free(tok->lexeme);
    free(tok);
}

/*
 * Returns a string representation of the token. RETURNED STRING MUST BE FREED
 * - enum TokenType tt: The TokenType to get the string representation of
 */
char* token_type_to_string(enum TokenType tt) {
    char* type = malloc(sizeof(char) * 20);
    switch (tt) {
        case 0:
            strncpy("RIGHT_BRACE", type, strlen("RIGHT_BRACE")); 
            break;
        case 1:
            strncpy("LEFT_BRACE", type, strlen("LEFT_BRACE"));
            break;
        case 2:
            strncpy("RIGHT_PARENTHESES", type, strlen("RIGHT_PARENTHESES"));
            break;
        case 3:
            strncpy("LEFT_PARENTHESES", type, strlen("LEFT_PARENTHESES"));
            break;
        case 4:
            strncpy("COMMA", type, strlen("COMMA"));
            break;
        case 5:
            strncpy("KEYWORD", type, strlen("KEYWORD"));
            break;
        case 6:
            strncpy("TYPE", type, strlen("TYPE"));
            break;
        case 7:
            strncpy("INT_TYPE", type, strlen("INT_TYPE"));
            break;
        case 8:
            strncpy("CHAR_TYPE", type, strlen("CHAR_TYPE"));
            break;
        case 9:
            strncpy("STRING_TYPE", type, strlen("STRING_TYPE"));
            break;
        case 10:
            strncpy("FLOAT_TYPE", type, strlen("FLOAT_TYPE"));
            break;
        case 11:
            strncpy("BOOL_TYPE", type, strlen("BOOL_TYPE"));
            break;
        case 12:
            strncpy("OPERATOR", type, strlen("OPERATOR"));
            break;
        case 13:
            strncpy("IDENTIFIER", type, strlen("IDENTIFIER"));
            break;
    }
    return type;
}

void print_token(token* tok) {
    char* type = token_type_to_string(tok->type);
    long c = tok->character_count;
    printf("<word count: %ld, line: %d, type: %s, lexeme: %s>\n", c, tok->line, type, tok->lexeme);
    free(type);
}

