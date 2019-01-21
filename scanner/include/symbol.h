#ifndef SCANNER_SYMBOL_H
#define SCANNER_SYMBOL_H

enum Symbol
{
    EMPTY,
    END_OF_FILE,    
    IDENTIFIER,
    NUMERAL,
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
    TRUE_KEYWORD,
    FALSE_KEYWORD,
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
    COMMENT,
    // Error symbols
    INVALID_CHAR,
    INVALID_NUMERAL,
    INVALID_WORD    
};

#endif