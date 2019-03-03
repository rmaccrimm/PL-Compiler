#include "parser.h"
#include <set>
#include <cassert>
#include <iostream>

Parser::Parser(std::vector<Token> &input_tokens):
    input{input_tokens}, next_token{input_tokens.begin()} {}

bool Parser::verify_syntax()
{
    skip_whitespace();
    program();
    return true;   
}

void Parser::skip_whitespace()
{
    while (next_token->symbol == NEWLINE || next_token->symbol == COMMENT) {
        next_token++;
    }
}

void Parser::match(Symbol s)
{
    // Temporary - just crash if token does not match 
    if (s != next_token->symbol) {
        std::cerr << "Error: expected " << SYMBOL_STRINGS.at(s) << " token type but got " 
                  << SYMBOL_STRINGS.at(next_token->symbol) << " instead" << std::endl;
        assert(false);
    }
    else {
        next_token++;
        skip_whitespace();
    }    
}

void Parser::syntax_error()
{
    assert(false);
}


void Parser::program()
{
    block();
    match(PERIOD);
}

void Parser::block()
{
    match(BEGIN);
    definition_part();
    statement_part();
    match(END);
}

void Parser::definition_part()
{
    std::set<Symbol> first{CONST, INT, BOOL, PROC};
    std::set<Symbol> follow{SKIP, READ, WRITE, ID, CALL, IF, DO, PERIOD, SEMICOLON};
    
}

void Parser::definition()
{
    switch (next_token->symbol) 
    {
    case CONST:
        constant_definition();
        break;
    case INT:
    case BOOL:
        variable_definition();
        break;
    case PROC:
        procedure_definition();
        break;
    default:
        syntax_error();
    };
}

void Parser::constant_definition()
{
    
}

void Parser::variable_definition()
{

}

void Parser::type_symbol()
{

}

void Parser::variable_list()
{

}

void Parser::procedure_definition()
{

}

void Parser::statement_part()
{

}

void Parser::statement()
{

}

void Parser::empty_statement()
{

}

void Parser::read_statement()
{

}

void Parser::variable_access_list()
{

}

void Parser::write_statement()
{

}

void Parser::expression_list()
{

}

void Parser::assignment_statement()
{

}

void Parser::procedure_statement()
{

}

void Parser::if_statement()
{

}

void Parser::do_statement()
{

}

void Parser::guarded_command_list()
{

}

void Parser::guarded_command()
{

}

void Parser::expression()
{

}

void Parser::primary_operator()
{

}

void Parser::primary_expression()
{

}

void Parser::relational_operator()
{

}

void Parser::simple_expression()
{

}

void Parser::adding_operator()
{

}

void Parser::term()
{

}

void Parser::multiplying_operator()
{

}

void Parser::factor()
{

}

void Parser::variable_access()
{

}

void Parser::indexed_selector()
{

}

void Parser::constant()
{

}

void Parser::numeral()
{

}

void Parser::boolean_symbol()
{

}

void Parser::name()
{

}
    