#include <catch.hpp>
#include <fstream>
#include <iostream>
#include "parser.h"
#include "scanner.h"

std::vector<Token> read_file(std::string fname) {
    SymbolTable sym;
    std::ifstream fin(fname);
    assert(fin.good());
    Scanner sc(fin, sym);
    Token t;
    std::vector<Token> tok_list;
    while (t.symbol != END_OF_FILE) {
        t = sc.get_token();
        tok_list.push_back(t);
    }
    return tok_list;
}

Parser P;

TEST_CASE("Variable redefinitions", "[var-defs]")
{
    std::vector<Token> tlist = read_file("test/src_files/scope/var_redefined");
    REQUIRE(P.verify_syntax(&tlist) == 4);

}

TEST_CASE("Constant redefinitions", "[const-defs]") 
{
    std::vector<Token> tlist = read_file("test/src_files/scope/const_redefined");
    REQUIRE(P.verify_syntax(&tlist) == 2);
}

TEST_CASE("Mixed redefinitions", "[all-defs]")
{ 
    std::vector<Token> tlist = read_file("test/src_files/scope/mixed_redefines");
    REQUIRE(P.verify_syntax(&tlist) == 4);
}

TEST_CASE("No redefinitions", "[all-defs-good]")
{ 
    std::vector<Token> tlist = read_file("test/src_files/scope/no_redefines");
    REQUIRE(P.verify_syntax(&tlist) == 0);
}

TEST_CASE("Array redefinitions", "[arr-defs]")
{ 
    std::vector<Token> tlist = read_file("test/src_files/scope/array_redefined");
    REQUIRE(P.verify_syntax(&tlist) == 2);
}

TEST_CASE("Procedure redefinitions", "[proc-defs]")
{ 
    std::vector<Token> tlist = read_file("test/src_files/scope/proc_redefined");
    REQUIRE(P.verify_syntax(&tlist) == 3);
}

TEST_CASE("Procedure scope", "[proc-scope]")
{ 
    std::vector<Token> tlist = read_file("test/src_files/scope/redefined_in_func");
    REQUIRE(P.verify_syntax(&tlist) == 3);
}

TEST_CASE("Array bounds type", "[array-bounds]")
{
    std::vector<Token> tlist = read_file("test/src_files/type/array_size");
    REQUIRE(P.verify_syntax(&tlist) == 5);
}