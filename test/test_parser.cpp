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

void run_test(std::string fname, int nerrors) 
{
    std::cout << fname << std::endl;
    std::vector<Token> tlist = read_file(fname);
    REQUIRE(P.verify_syntax(&tlist) == nerrors);
    std::cout << std::endl;
}

TEST_CASE("Variable redefinitions", "[var-defs]")
{
    run_test("test/src_files/scope/var_redefined", 4);
}

TEST_CASE("Constant redefinitions", "[const-defs]") 
{
    run_test("test/src_files/scope/const_redefined", 2);
}

TEST_CASE("Mixed redefinitions", "[all-defs]")
{ 
    run_test("test/src_files/scope/mixed_redefines", 30);
}

TEST_CASE("No redefinitions", "[all-defs-good]")
{ 
    run_test("test/src_files/scope/no_redefines", 0);
}

TEST_CASE("Array redefinitions", "[arr-defs]")
{ 
    run_test("test/src_files/scope/array_redefined", 2);
}

TEST_CASE("Procedure redefinitions", "[proc-defs]")
{ 
    run_test("test/src_files/scope/proc_redefined", 2);
}

TEST_CASE("Procedure scope", "[proc-scope]")
{ 
    run_test("test/src_files/scope/redefined_in_func", 0);
}

TEST_CASE("Nested procedures", "[nested_proc]")
{
    run_test("test/src_files/scope/nested_funcs", 3);
}

TEST_CASE("Array bounds type", "[array-bounds]")
{
    run_test("test/src_files/type/array_size", 5);
}

TEST_CASE("Assigning literal", "[assign-literal]")
{
    run_test("test/src_files/type/assigning_literals", 20);
}

TEST_CASE("Assigning procedures", "[assign-proc]")
{
    run_test("test/src_files/type/assigning_procs", 9);
}

TEST_CASE("Read and write statements", "[read-write]")
{
    run_test("test/src_files/type/read_write", 3);
}

TEST_CASE("Assign arithmetic expressions", "[assign-arithmetic]")
{
    run_test("test/src_files/type/assigning_arithmetic", 23);
}

TEST_CASE("Assign logic expressions", "[assign-logic]")
{
    run_test("test/src_files/type/assigning_comparisons", 25);
}

TEST_CASE("Guarded commands", "[guarded]")
{
    run_test("test/src_files/type/guarded_commands", 15);
}

TEST_CASE("Undefined variables", "[vars]")
{
    run_test("test/src_files/scope/vars_undefined", 23);
}
