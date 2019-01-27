#include <catch.hpp>
#include <fstream>
#include <iostream>
#include "scanner.h"
#include "symbol_table.h"

TEST_CASE("Check all valid token types", "[token_types]")
{
    SymbolTable sym;
    std::ifstream in_file("test/src_files/token_types");
    if (!in_file.good()) {
        std::cout << "Unable to open test input file" << std::endl;
    }
    Scanner sc(in_file, sym);
    Symbol expected[] = {
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
    for (auto s: expected) {
        REQUIRE(s == sc.get_token().symbol);
    }
}

TEST_CASE("Invalid characters, numerals and identifiers", "[invalid]")
{

}

TEST_CASE("Comments are skipped, but leave newline", "[comments]")
{

}

