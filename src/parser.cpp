#include "parser.h"
#include <cassert>
#include <set>

Parser::Parser(std::vector<Token> &input_tokens):
    input{input_tokens}, next_token{input_tokens.begin()} {}

bool Parser::verify_syntax()
{
    return false;
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
    std::set<Symbol> follow{SKIP, READ, WRITE, IDENTIFIER, CALL, IF, DO, PERIOD, SEMICOLON};
    auto s = next_token->symbol;
    if (first.find(s) != first.end()) {
        definition();
        match(SEMICOLON);
    }
    else if (follow.find(s) != follow.end()) {
        return;
    }
    else {
        syntax_error();
    }
}

void Parser::definition()
{
    auto s = next_token->symbol;
    if (s == CONST) {
        constant_definition();
    }
    else if (s == INT || s == BOOL) {
        variable_definition();
    }
    else if (s == PROC) {
        procedure_definition();
    }
    else {
        syntax_error();
    }
}

void Parser::constant_definition()
{
    match(CONST);
    match(IDENTIFIER);
    match(EQUALS);
    constant();
}

void Parser::variable_definition()
{
    type_symbol();
    variable_definition_type();
}

void Parser::type_symbol()
{
    auto s = next_token->symbol;
    if (s == INT || s == BOOL) {
        match(s);
    }
    else {
        syntax_error();
    }
}

void Parser::variable_list()
{
    match(IDENTIFIER);
    variable_list_end();
}

void Parser::procedure_definition()
{
    match(PROC);
    match(IDENTIFIER);
    block();
}

void Parser::statement_part()
{
    std::set<Symbol> first{SKIP, READ, WRITE, IDENTIFIER, CALL, IF, DO};
    std::set<Symbol> follow{END, DOUBLE_BRACKET, FI, OD};
    auto s = next_token->symbol;
    if (first.find(s) != first.end()) {
        statement();
        match(SEMICOLON);
    }
    else if (follow.find(s) != follow.end()) {
        return;
    }
    else {
        syntax_error();
    }
}

void Parser::statement()
{
    auto s = next_token->symbol;
    if (s == SKIP) {
        empty_statement();
    }
    else if (s == READ) {
        read_statement();
    }
    else if (s == WRITE) {
        write_statement();
    }
    else if (s == IDENTIFIER) {
        assignment_statement();
    }
    else if (s == CALL) {
        procedure_statement();
    }
    else if (s == IF) {
        if_statement();
    }
    else if (s == DO) {
        do_statement();
    }
    else {
        syntax_error();
    }
}

void Parser::empty_statement()
{
    match(SKIP);
}

void Parser::read_statement()
{
    match(READ);
    variable_access_list();
}

void Parser::variable_access_list()
{
    variable_access();
    variable_access_list_end();
}

void Parser::variable_access_list_end()
{
    std::set<Token> first{SEMICOLON, ASSIGN};
    auto s = next_token->symbol;
    if (s == COMMA) {
        match(COMMA);
        variable_access();
        variable_access_list_end();
    }
    else if (first.find(s) != first.end()) {
        return;
    }
    else {
        syntax_error();
    }
}

void Parser::write_statement()
{
    match(WRITE);
    expression_list();
}

void Parser::expression_list()
{
    expression();
    expression_list_end();
}

void Parser::expression_list_end()
{
    auto s = next_token->symbol;
    if (s == COMMA) {
        match(s);
        expression();
        expression_list_end();
    }
    else if (s == SEMICOLON) {
        return;
    }
}

void Parser::assignment_statement()
{
    variable_access_list();
    match(ASSIGN);
    expression_list();
}

void Parser::procedure_statement()
{
    match(CALL);
    match(IDENTIFIER);
}

void Parser::if_statement()
{
    match(IF);
    guarded_command_list();
    match(FI);
}

void Parser::do_statement()
{
    match(DO);
    guarded_command_list();
    match(OD);
}

void Parser::guarded_command_list()
{
    guarded_command();
    guarded_command_list_end();
}

void Parser::guarded_command_list_end()
{
    auto s = next_token->symbol;
    if (s == DOUBLE_BRACKET) {
        match(s);
        guarded_command();
        guarded_command_list();
    }
    else if (s == FI || s == OD) {
        return;
    }
    else {
        syntax_error();
    }
}

void Parser::guarded_command()
{
    expression();
    match(RIGHT_ARROW);
    statement_part();
}

void Parser::expression()
{
    primary_expression();
    expression_end();
}

void Parser::expression_end()
{
    std::set<Symbol> follow{COMMA, SEMICOLON, RIGHT_ARROW, RIGHT_BRACKET, RIGHT_PARENTHESIS};
    auto s = next_token->symbol;
    if (s == AND || s == OR) {
        primary_operator();
        primary_expression();
        expression_end();
    }
    else if (follow.find(s) != follow.end()) {
        return;
    }
    else {
        syntax_error();
    }
}

void Parser::primary_operator()
{
    auto s = next_token->symbol;
    if (s == AND || s == OR) {
        match(s);
    }
    else {
        syntax_error();
    }
}

void Parser::primary_expression()
{
    simple_expression();
    primary_expression_end();
}

void Parser::primary_expression_end()
{
    relational_operator();
    simple_expression();
}

void Parser::relational_operator()
{
    auto s = next_token->symbol;
    if (s == LESS_THAN || s == EQUALS || s == GREATER_THAN) {
        match(s);
    }
    else {
        syntax_error();
    }
}

void Parser::simple_expression()
{
    std::set<Symbol> first{NOT, LEFT_PARENTHESIS, NUMERAL, TRUE_KEYWORD, FALSE_KEYWORD, IDENTIFIER};
    auto s = next_token->symbol;
    if (s == SUBTRACT) {
        match(s);
        term();
        simple_expression_end();
    }
    else if (first.find(s) != first.end()) {
        term();
        simple_expression_end();
    }
    else {
        syntax_error();
    }
}

void Parser::simple_expression_end()
{
    std::set<Symbol> follow{LESS_THAN, EQUALS, GREATER_THAN, AND, OR, SEMICOLON, COMMA, RIGHT_ARROW, 
        RIGHT_BRACKET, RIGHT_PARENTHESIS};
    auto s = next_token->symbol;
    if (s == ADD || s == SUBTRACT) {
        adding_operator();
        term();
        simple_expression_end();
    }
    else if (follow.find(s) != follow.end()) {
        return;
    }
    else {
        syntax_error();
    }
}

void Parser::adding_operator()
{
    auto s = next_token->symbol;
    if (s == ADD || s == OR) {
        match(s);
    }
    else {
        syntax_error();
    }
}

void Parser::term()
{
    factor();
    term_end();
}

void Parser::term_end()
{
    std::set<Symbol> follow{LESS_THAN, EQUALS, GREATER_THAN, AND, OR, SEMICOLON, COMMA, RIGHT_ARROW,
        RIGHT_BRACKET, RIGHT_PARENTHESIS};
    auto s = next_token->symbol;
    if (s == MULTIPLY || s == DIVIDE || s == MODULO) {
        multiplying_operator();
        factor();
        term_end();
    }
    else if (follow.find(s) != follow.end()) {
        return;
    }
    else {
        syntax_error();
    }
    
}

void Parser::multiplying_operator()
{
    auto s = next_token->symbol;
    if (s == MULTIPLY || s == DIVIDE || s ==  MODULO) {
        match(s);
    }
    else {
        syntax_error();
    }
}

void Parser::factor()
{
    auto s = next_token->symbol;
    if (s == LEFT_PARENTHESIS) {

    }
    else if (s == IDENTIFIER) {

    }
    else if (s == NUMERAL || s == TRUE_KEYWORD || s == FALSE_KEYWORD || s == IDENTIFIER) {

    }
    else if (s == NOT || s == LEFT_PARENTHESIS ) {

    }
    else {
        syntax_error();
    }
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


void Parser::boolean_symbol()
{

}
    