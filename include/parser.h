#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include <vector>
#include <string>
#include <map>
#include <set>

class Parser
{
public:
    // Construct a parser to verify the given input represents a syntactically correct PL program
    Parser(std::vector<Token> &input_tokens);

    /*  Parse input and verify it can be derived from the PL language grammar. The call stack is 
        printed as it runs, showing the structure of the parse tree. 
        
        When an error is encountered, the program crashes and prints an error message indicating the
        line and token where the error occurred.
    */
    bool verify_syntax();

private:
    // Skip newline and comment tokens
    void skip_whitespace();

    // Check next token in input and advance if symbol matches s
    void match(Symbol s);

    // Handle syntax errors, currently just crashes
    void syntax_error(); 

    // Used to print the function call stack 
    void print(std::string msg);

    // Recursive procedures for nonterminals
    void program();
    void block();
    void definition_part();
    void definition();
    void constant_definition();
    void variable_definition();
    void variable_definition_type();
    void type_symbol();
    void variable_list();
    void variable_list_end();
    void procedure_definition();
    void statement_part();
    void statement();
    void empty_statement();
    void read_statement();
    void variable_access_list();
    void variable_access_list_end();
    void write_statement();
    void expression_list();
    void expression_list_end();
    void assignment_statement();
    void procedure_statement();
    void if_statement();
    void do_statement();
    void guarded_command_list();
    void guarded_command_list_end();
    void guarded_command();
    void expression();
    void expression_end();
    void primary_operator();
    void primary_expression();
    void primary_expression_end();
    void relational_operator();
    void simple_expression();
    void simple_expression_end();
    void adding_operator();
    void term();
    void term_end();
    void multiplying_operator();
    void factor();
    void variable_access();
    void variable_access_end();
    void indexed_selector();
    void constant();

    void init_symbol_sets();

    std::map<std::string, std::set<Symbol>> first, follow;
    std::vector<Token> &input;
    std::vector<Token>::iterator next_token;
    int line;
    // Used to track height of call stack for printing function calls with indentation
    int depth;
};

#endif