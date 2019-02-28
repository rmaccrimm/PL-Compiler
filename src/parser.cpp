#include "parser.h"
#include <cassert>

Parser::Parser(std::vector<Token> &input_tokens): 
    input{input_tokens}, next_token{input_tokens.begin()} {}

bool Parser::verify_syntax()
{
    while (next_token->symbol != END_OF_FILE) {

    }
}

void Parser::match(Symbol s)
{
    // Temporary - just crash if token does not match
    assert(s == next_token->symbol);
    do {
        next_token++;
    } while (next_token->symbol == NEWLINE || next_token->symbol == COMMENT);
}

void Parser::syntax_error()
{
    assert(false);
}


void Parser::program()
{
    if (next_token->symbol == BEGIN)
    {
        block();
        match(PERIOD);
    }
    else {
        syntax_error();
    }
}

void Parser::block()
{

}

void Parser::definition_part()
{

}

void Parser::definition()
{

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
    