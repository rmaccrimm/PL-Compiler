#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include <vector>

class Parser
{
public:
    Parser();

private:
    // Compare token with the next token in input and advance if they match
    void match(Token t);

    // Recursive procedures for nonterminals 
    void program();
    void block();
    void definition_part();
    void definition();
    void constant_definition();
    void variable_definition();
    void type_symbol();
    void variable_list();
    void procedure_definition();
    void statement_part();
    void statement();
    void empty_statement();
    void read_statement();
    void variable_access_list();
    void write_statement();
    void expression_list();
    void assignment_statement();
    void procedure_statement();
    void if_statement();
    void do_statement();
    void guarded_command_list();
    void guarded_command();
    void expression();
    void primary_operator();
    void primary_expression();
    void relational_operator();
    void simple_expression();
    void adding_operator();
    void term();
    void multiplying_operator();
    void factor();
    void variable_access();
    void indexed_selector();
    void constant();
    void numeral();
    void boolean_symbol();
    void name();    

    std::vector<Token>::iterator next_token;
};

#endif