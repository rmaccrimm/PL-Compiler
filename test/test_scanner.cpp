#include <catch.hpp>
#include <fstream>
#include <iostream>
#include "scanner.h"
#include "symbol_table.h"

void check_expected(std::vector<Symbol> &expected, Scanner &sc)
{
    for (auto s: expected) {
        auto t = sc.get_token();
        // std::cout << s << ' ' << t.symbol << ' ' << t.lexeme << std::endl;
        REQUIRE(s == t.symbol);
    }
}

void check_expected(std::vector<Token> &expected, Scanner &sc)
{
    for (auto s: expected) {
        auto t = sc.get_token();
        // std::cout << s << ' ' << t.symbol << ' ' << t.lexeme << std::endl;
        REQUIRE(s.lexeme == t.lexeme);
        REQUIRE(s.value == t.value);
        REQUIRE(s.symbol == t.symbol);
    }
}

TEST_CASE("Check all valid token types", "[token_types]")
{
    SymbolTable sym;
    std::ifstream fin("test/src_files/scan/token_types");
    assert(fin.good());
    Scanner sc(fin, sym);
    std::vector<Symbol> expected = {
        PERIOD, COMMA, SEMICOLON, AND, OR, LEFT_BRACKET, LEFT_PARENTHESIS, RIGHT_PARENTHESIS,
        RIGHT_BRACKET, NOT, LESS_THAN, EQUALS, GREATER_THAN, ADD, SUBTRACT, MULTIPLY, DIVIDE, 
        MODULO, ASSIGN, DOUBLE_BRACKET, RIGHT_ARROW, NEWLINE,
        BEGIN, IDENTIFIER, END, IDENTIFIER, CONST, IDENTIFIER, ARRAY, IDENTIFIER, INT, 
        IDENTIFIER, BOOL, IDENTIFIER, PROC, IDENTIFIER, SKIP, IDENTIFIER, READ, IDENTIFIER, 
        WRITE, IDENTIFIER, CALL, IDENTIFIER, IF, IDENTIFIER, FI, IDENTIFIER, DO, IDENTIFIER, OD, 
        IDENTIFIER, FALSE_KEYWORD, IDENTIFIER, TRUE_KEYWORD, IDENTIFIER, NEWLINE,
        IDENTIFIER, IDENTIFIER, IDENTIFIER, IDENTIFIER, IDENTIFIER, IDENTIFIER, NEWLINE,
        NUMERAL, NUMERAL, NUMERAL, END_OF_FILE
    };
    check_expected(expected, sc);
}

TEST_CASE("Invalid characters, numerals and identifiers", "[invalid]")
{
    SymbolTable sym;
    std::ifstream fin("test/src_files/scan/invalid");
    assert(fin.good());
    Scanner sc(fin, sym);
    std::vector<Symbol> expected = {
        INVALID_WORD, INVALID_WORD, INVALID_WORD, NEWLINE, INVALID_CHAR, INVALID_CHAR, 
        INVALID_SYMBOL, ADD, INVALID_CHAR, IDENTIFIER, INVALID_CHAR, IDENTIFIER, 
        NEWLINE, INVALID_NUMERAL, INVALID_NUMERAL, INVALID_NUMERAL, INVALID_NUMERAL, 
        END_OF_FILE
    };
    check_expected(expected, sc);
}

TEST_CASE("Procedure names", "[proc]")
{
    SymbolTable sym;
    std::ifstream fin("test/src_files/scan/proc_names");
    assert(fin.good());
    Scanner sc(fin, sym);
    std::vector<Token> expected = {
        Token(BEGIN, "begin", 0),
        Token(PROC, "proc", 0),
        Token(IDENTIFIER, "A", 0),
        Token(BEGIN, "begin", 0),
        Token(END, "end", 0),
        Token(SEMICOLON, ";", 0),
        Token(PROC, "proc", 0),
        Token(IDENTIFIER, "B", 0),
        Token(BEGIN, "begin", 0),
        Token(END, "end", 0),
        Token(SEMICOLON, ";", 0),
        Token(PROC, "proc", 0),
        Token(IDENTIFIER, "C", 0),
        Token(BEGIN, "begin", 0),
        Token(END, "end", 0),
        Token(SEMICOLON, ";", 0),
        Token(END, "end", 0),
        Token(PERIOD, ".", 0)
    };
    check_expected(expected, sc);
}
