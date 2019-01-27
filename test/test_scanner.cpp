#include <catch.hpp>
#include <fstream>
#include <iostream>
#include "scanner.h"
#include "symbol_table.h"

std::ifstream open_file(std::string s)
{
    SymbolTable sym;
    std::ifstream in_file(s);
    if (!in_file.good()) {
        std::cout << "Unable to open test input file " + s << std::endl;
        assert(false); 
    }
    return in_file;
}

void check_expected(std::vector<Symbol> &expected, Scanner &sc)
{
    for (auto s: expected) {
        REQUIRE(s == sc.get_token().symbol);
    }
}

TEST_CASE("Check all valid token types", "[token_types]")
{
    SymbolTable sym;
    auto fin = open_file("test/src_files/token_types");
    Scanner sc(fin, sym);
    // std::cout << sc.get_token().lexeme << std::endl;
    // std::cout << sc.get_token().lexeme << std::endl;
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
}

TEST_CASE("Comments are skipped, but leave newline", "[comments]")
{

}

