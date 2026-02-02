#ifndef TOKEN_H
#define TOKEN_H
#include <stdlib.h>

/*
 * This enum defines the different kinds of
 * token types
 */
enum TokenType {
    RIGHT_BRACE,
    LEFT_BRACE,
    RIGHT_PARENTHESES,
    LEFT_PARENTHESES,
    COMMA,
    KEYWORD,
    TYPE,
    INT_TYPE,
    CHAR_TYPE,
    STRING_TYPE,
    FLOAT_TYPE,
    BOOL_TYPE,
    OPERATOR,
    IDENTIFIER,
};

/*
 * This struct defines a type of token, that
 * contains all relevant information pertaining
 * to a scanned token
 */
struct token {
    long character_count;
    unsigned short line;
    enum TokenType type;
    char *lexeme;
    void *literal;
} typedef token;

/*
 * Allocates and creates a token representing the provided variables
 * - long count: Index of the first character of this token
 * - long line: The line this token is on
 * - char* lexeme: String representing the characters of the token
 * - void* literal: The literal value of the token
 * - enum TokenType type: The type of this token
 */
token* create_token(long count, unsigned short line, enum TokenType type, char* lexeme, void* literal, size_t size);

/*
 * Frees the memory used by this token
 * - token tok: The token to be released
 */
void delete_token(token* tok);

/*
 * Prints the contents of the token to the
 * console in a human-readable format
 * - token tok: The token to be printed
 */
void print_token(token* tok);

#endif