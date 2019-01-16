#ifndef SCANNER_SYMBOL_H
#define SCANNER_SYMBOL_H

enum class Symbol: int
{
    ID,
    INT_LITERAL,
    // Keywords
    BEGIN,
    END,
    CONST,
    ARRAY,
    INT,
    BOOL,
    PROC,
    SKIP,
    READ,
    WRITE,
    CALL,
    IF,
    FI,
    DO,
    OD,
    TRUE,
    FALSE,
    // Special Symbols
    PERIOD,
    COMMA,
    SEMICOLON,
    LEFT_PARENTHESIS,
    RIGHT_PARENTHESIS,
    LEFT_BRACKET,
    RIGHT_BRACKET,
    AND,
    OR,
    NOT,
    LESS_THAN,
    EQUALS,
    GREATER_THAN,
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    MODULO,
    ASSIGN,
    RIGHT_ARROW,
    DOUBLE_BRACKET,
    COMMENT
};

#endif