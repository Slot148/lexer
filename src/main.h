#ifndef MAIN_H
#define MAIN_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <regex.h>

#define CATCH_STATUS(status_code, message) catch_status(status_code, message, __FILE__, __LINE__, __func__)

typedef enum TokenType{TOK_KEYWORD, TOK_IDENTIFIER, TOK_ASSIGN, TOK_NUMBER, TOK_WHITESPACE, TOK_EOF}TokenType;
typedef struct Token* Token;
struct Token{
    TokenType type;
    char *lexeme;
    int line;
    int column;
};

typedef struct TokenPattern{
    TokenType type;
    const char *pattern;
    int should_keep;
    regex_t regex;
}TokenPattern;

static const TokenPattern patterns[] = {
    {TOK_KEYWORD,    "\\b(int)\\b", 1},
    {TOK_IDENTIFIER, "[a-zA-Z_][a-zA-Z0-9_]*", 1},
    {TOK_ASSIGN,     "=", 1},
    {TOK_NUMBER,     "\\d+(\\.\\d+)?([eE][+-]?\\d+)?", 1},
    {TOK_WHITESPACE, "\\s+", 0},
    {TOK_EOF,        "\\0", 1}
};

static const char* token_type_names[] = {
    [TOK_KEYWORD] = "KEYWORD",
    [TOK_IDENTIFIER] = "IDENTIFIER",
    [TOK_ASSIGN] = "ASSIGN",
    [TOK_NUMBER] = "NUMBER",
    [TOK_WHITESPACE] = "WHITESPACE",
};

typedef struct Vector* Vector;
struct Vector{
    Token* tokens;
    size_t size, capacity;
};

typedef enum Status{OK, ERROR, WARNING}Status;


#endif // MAIN_H
