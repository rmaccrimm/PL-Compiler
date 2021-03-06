#ifndef PL_SYMBOL_H
#define PL_SYMBOL_H

#include <map>
#include <string>

enum Symbol
{
    EMPTY,
    END_OF_FILE,
    NEWLINE, 
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
    LEFT_PARENTHESIS,   // (
    RIGHT_PARENTHESIS,  // )
    LEFT_BRACKET,       // [
    RIGHT_BRACKET,      // ]
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
    INVALID_WORD,
    INVALID_SYMBOL 
};

static const std::map<Symbol, std::string> SYMBOL_STRINGS = 
{
    {EMPTY, "EMPTY"},
    {END_OF_FILE, "END_OF_FILE"},
    {NEWLINE, "NEWLINE"},
    {IDENTIFIER, "IDENTIFIER"},
    {NUMERAL, "NUMERAL"},
    {BEGIN, "BEGIN"},
    {END, "END"},
    {CONST, "CONST"},
    {ARRAY, "ARRAY"},
    {INT, "INT"},
    {BOOL, "BOOL"},
    {PROC, "PROC"},
    {SKIP, "SKIP"},
    {READ, "READ"},
    {WRITE, "WRITE"},
    {CALL, "CALL"},
    {IF, "IF"},
    {FI, "FI"},
    {DO, "DO"},
    {OD, "OD"},
    {TRUE_KEYWORD, "TRUE_KEYWORD"},
    {FALSE_KEYWORD, "FALSE_KEYWORD"},
    {PERIOD, "PERIOD"},
    {COMMA, "COMMA"},
    {SEMICOLON, "SEMICOLON"},
    {LEFT_PARENTHESIS, "LEFT_PARENTHESIS"},
    {RIGHT_PARENTHESIS, "RIGHT_PARENTHESIS"},
    {LEFT_BRACKET, "LEFT_BRACKET"},
    {RIGHT_BRACKET, "RIGHT_BRACKET"},
    {AND, "AND"},
    {OR, "OR"},
    {NOT, "NOT"},
    {LESS_THAN, "LESS_THAN"},
    {EQUALS, "EQUALS"},
    {GREATER_THAN, "GREATER_THAN"},
    {ADD, "ADD"},
    {SUBTRACT, "SUBTRACT"},
    {MULTIPLY, "MULTIPLY"},
    {DIVIDE, "DIVIDE"},
    {MODULO, "MODULO"},
    {ASSIGN, "ASSIGN"},
    {RIGHT_ARROW, "RIGHT_ARROW"},
    {DOUBLE_BRACKET, "DOUBLE_BRACKET"},
    {COMMENT, "COMMENT"},
    {INVALID_CHAR, "INVALID_CHAR"},
    {INVALID_NUMERAL, "INVALID_NUMERAL"},
    {INVALID_WORD, "INVALID_WORD"},
    {INVALID_SYMBOL, "INVALID_SYMBOL"}
};

#endif