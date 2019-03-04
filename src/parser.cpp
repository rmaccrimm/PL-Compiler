#include "parser.h"
#include <iostream>
#include <cassert>
#include <set>

#define PRINT 1
#define INDENT 1

Parser::Parser(std::vector<Token> &input_tokens):
    input{input_tokens}, next_token{input_tokens.begin()}, line{1}, depth{0} {}

void Parser::print(std::string msg) 
{
    if (PRINT) {
        if (INDENT) {
            for (int i = 0; i < depth; i++) {
                std::cout << ' ';
            }
        }
        std::cout << msg << std::endl;
    }
}

bool Parser::verify_syntax()
{
    skip_whitespace();
    program();
    return true;   
}

void Parser::skip_whitespace()
{
    while (next_token->symbol == NEWLINE || next_token->symbol == COMMENT) {
        if (next_token->symbol == NEWLINE) {
            line++;
        }
        next_token++;
    }
}

void Parser::match(Symbol s)
{
    if (s != next_token->symbol) {
        std::cerr << "Error in match" << " for " << SYMBOL_STRINGS.at(next_token->symbol) 
                  << " symbol" << std::endl;
        syntax_error();
    }
    else {
        next_token++;
        skip_whitespace();
    }    
}

void Parser::syntax_error()
{
    // Temporary - just crash if token does not match 
    std::cerr << "Syntax error on line " << line << " for " 
              << SYMBOL_STRINGS.at(next_token->symbol) << " symbol" << std::endl;
    assert(false);
}

void Parser::program()
{
    print("program");
    depth++;
    block();
    depth--;
    match(PERIOD);
}

void Parser::block()
{
	print("block");
    match(BEGIN);
    depth++;
    definition_part();
    statement_part();
    depth--;
    match(END);
}

void Parser::definition_part()
{
	print("definition_part");
    depth++;
    std::set<Symbol> first{CONST, INT, BOOL, PROC};
    std::set<Symbol> follow{SKIP, READ, WRITE, IDENTIFIER, CALL, IF, DO, PERIOD, SEMICOLON};
    auto s = next_token->symbol;
    if (first.find(s) != first.end()) {        
        definition();
        match(SEMICOLON);
        depth--;
        definition_part();
    }
    else if (follow.find(s) != follow.end()) {
        depth--;
    }
    else {
        syntax_error();
    }
    // depth--;
}

void Parser::definition()
{
	print("definition");
    depth++;
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
    depth--;
}

void Parser::constant_definition()
{
	print("constant_definition");
    depth++;
    match(CONST);
    match(IDENTIFIER);
    match(EQUALS);
    constant();
    depth--;
}

void Parser::variable_definition()
{
	print("variable_definition");
    depth++;
    type_symbol();
    variable_definition_type();
    depth--;
}

void Parser::variable_definition_type()
{
	print("variable_definition_type");
	depth++;
    auto s = next_token->symbol;
    if (s == IDENTIFIER) {
        variable_list();
    }
    else if (s == ARRAY) {
        match(ARRAY);
        variable_list();
        match(LEFT_BRACKET);
        constant();
        match(RIGHT_BRACKET);
    }
    else {
        syntax_error();
    }
	depth--;
}

void Parser::type_symbol()
{
	print("type_symbol");
	depth++;
    auto s = next_token->symbol;
    if (s == INT || s == BOOL) {
        match(s);
    }
    else {
        syntax_error();
    }
	depth--;
}

void Parser::variable_list()
{
	print("variable_list");
	depth++;
    match(IDENTIFIER);
    variable_list_end();
	depth--;
}

void Parser::variable_list_end()
{
	print("variable_list_end");
	depth++;
    auto s = next_token->symbol;
    if (s == COMMA) {
        match(s);
        match(IDENTIFIER);
        variable_list_end();
    }
    else if (s == SEMICOLON || s == LEFT_BRACKET) {
    }
    else {
        syntax_error();
    }	
    depth--;
}

void Parser::procedure_definition()
{
	print("procedure_definition");
	depth++;
    match(PROC);
    match(IDENTIFIER);
    block();
	depth--;
}

void Parser::statement_part()
{
	print("statement_part");
	depth++;
    std::set<Symbol> first{SKIP, READ, WRITE, IDENTIFIER, CALL, IF, DO};
    std::set<Symbol> follow{END, DOUBLE_BRACKET, FI, OD};
    auto s = next_token->symbol;
    if (first.find(s) != first.end()) {
        statement();
        match(SEMICOLON);
        depth--;
        statement_part();
    }
    else if (follow.find(s) != follow.end()) {
        depth--;
    }
    else {
        syntax_error();
    }
}

void Parser::statement()
{
	print("statement");
	depth++;
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
	depth--;
}

void Parser::empty_statement()
{
	print("empty_statement");
	depth++;
    match(SKIP);
	depth--;
}

void Parser::read_statement()
{
	print("read_statement");
	depth++;
    match(READ);
    variable_access_list();
	depth--;
}

void Parser::variable_access_list()
{
	print("variable_access_list");
	depth++;
    variable_access();
    variable_access_list_end();
	depth--;
}

void Parser::variable_access_list_end()
{
	print("variable_access_list_end");
	depth++;
    std::set<Symbol> first{SEMICOLON, ASSIGN};
    auto s = next_token->symbol;
    if (s == COMMA) {
        match(COMMA);
        variable_access();
        variable_access_list_end();
    }
    else if (first.find(s) != first.end()) {
        
    }
    else {
        syntax_error();
    }
    depth--;
}

void Parser::write_statement()
{
	print("write_statement");
	depth++;
    match(WRITE);
    expression_list();
	depth--;
}

void Parser::expression_list()
{
	print("expression_list");
	depth++;
    expression();
    expression_list_end();
	depth--;
}

void Parser::expression_list_end()
{
	print("expression_list_end");
	depth++;
    auto s = next_token->symbol;
    if (s == COMMA) {
        match(s);
        expression();
        expression_list_end();
    }
    else if (s == SEMICOLON) {
    }
    depth--;
}

void Parser::assignment_statement()
{
	print("assignment_statement");
	depth++;
    variable_access_list();
    match(ASSIGN);
    expression_list();
	depth--;
}

void Parser::procedure_statement()
{
	print("procedure_statement");
	depth++;
    match(CALL);
    match(IDENTIFIER);
	depth--;
}

void Parser::if_statement()
{
	print("if_statement");
	depth++;
    match(IF);
    guarded_command_list();
    match(FI);
	depth--;
}

void Parser::do_statement()
{
	print("do_statement");
	depth++;
    match(DO);
    guarded_command_list();
    match(OD);
	depth--;
}

void Parser::guarded_command_list()
{
	print("guarded_command_list");
	depth++;
    guarded_command();
    guarded_command_list_end();
	depth--;
}

void Parser::guarded_command_list_end()
{
	print("guarded_command_list_end");
	depth++;
    auto s = next_token->symbol;
    if (s == DOUBLE_BRACKET) {
        match(s);
        guarded_command();
        depth--;
        guarded_command_list_end();
    }
    else if (s == FI || s == OD) {
        depth--;
    }
    else {
        syntax_error();
    }
}

void Parser::guarded_command()
{
	print("guarded_command");
	depth++;
    expression();
    match(RIGHT_ARROW);
    statement_part();
	depth--;
}

void Parser::expression()
{
	print("expression");
	depth++;
    primary_expression();
    expression_end();
	depth--;
}

void Parser::expression_end()
{
	print("expression_end");
	depth++;
    std::set<Symbol> follow{COMMA, SEMICOLON, RIGHT_ARROW, RIGHT_BRACKET, RIGHT_PARENTHESIS};
    auto s = next_token->symbol;
    if (s == AND || s == OR) {
        primary_operator();
        primary_expression();
        expression_end();
    }
    else if (follow.find(s) != follow.end()) {
    }
    else {
        syntax_error();
    }
    depth--;
}

void Parser::primary_operator()
{
	print("primary_operator");
	depth++;
    auto s = next_token->symbol;
    if (s == AND || s == OR) {
        match(s);
    }
    else {
        syntax_error();
    }
	depth--;
}

void Parser::primary_expression()
{
	print("primary_expression");
	depth++;
    simple_expression();
    primary_expression_end();
	depth--;
}

void Parser::primary_expression_end()
{
	print("primary_expression_end");
	depth++;
    std::set<Symbol> follow{AND, OR, COMMA, SEMICOLON, RIGHT_ARROW, RIGHT_PARENTHESIS, 
        RIGHT_BRACKET};
    auto s = next_token->symbol;
    if (s == LESS_THAN || s == GREATER_THAN || s == EQUALS) {
        relational_operator();
        simple_expression();
    }
    else if (follow.find(s) != follow.end()) {
    }
    else {
        syntax_error();
    }
	depth--;
}

void Parser::relational_operator()
{
	print("relational_operator");
	depth++;
    auto s = next_token->symbol;
    if (s == LESS_THAN || s == EQUALS || s == GREATER_THAN) {
        match(s);
    }
    else {
        syntax_error();
    }
	depth--;
}

void Parser::simple_expression()
{
	print("simple_expression");
	depth++;
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
	depth--;
}

void Parser::simple_expression_end()
{
	print("simple_expression_end");
	depth++;
    std::set<Symbol> follow{LESS_THAN, EQUALS, GREATER_THAN, AND, OR, SEMICOLON, COMMA, RIGHT_ARROW, 
        RIGHT_BRACKET, RIGHT_PARENTHESIS};
    auto s = next_token->symbol;
    if (s == ADD || s == SUBTRACT) {
        adding_operator();
        term();
        depth--;
        simple_expression_end();
    }
    else if (follow.find(s) != follow.end()) {
        depth--;
    }
    else {
        syntax_error();
    }
}

void Parser::adding_operator()
{
	print("adding_operator");
	depth++;
    auto s = next_token->symbol;
    if (s == ADD || s == OR) {
        match(s);
    }
    else {
        syntax_error();
    }
	depth--;
}

void Parser::term()
{
	print("term");
	depth++;
    factor();
    term_end();
	depth--;
}

void Parser::term_end()
{
	print("term_end");
	depth++;
    std::set<Symbol> follow{LESS_THAN, EQUALS, GREATER_THAN, AND, OR, SEMICOLON, COMMA, RIGHT_ARROW,
        RIGHT_BRACKET, RIGHT_PARENTHESIS, ADD, SUBTRACT};
    auto s = next_token->symbol;
    if (s == MULTIPLY || s == DIVIDE || s == MODULO) {
        multiplying_operator();
        factor();
        depth--;
        term_end();
    }
    else if (follow.find(s) != follow.end()) {
        depth--;
    }
    else {
        syntax_error();
    }
}

void Parser::multiplying_operator()
{
	print("multiplying_operator");
	depth++;
    auto s = next_token->symbol;
    if (s == MULTIPLY || s == DIVIDE || s ==  MODULO) {
        match(s);
    }
    else {
        syntax_error();
    }
	depth--;
}

void Parser::factor()
{
	print("factor");
	depth++;
    // LL(1) issue - does not correctly decide between VarAccess and Constant
    auto s = next_token->symbol;
    if (s == LEFT_PARENTHESIS) {
        match(s);
        expression();
        match(RIGHT_PARENTHESIS);
    }
    else if (s == IDENTIFIER) {
        variable_access();
    }
    else if (s == NUMERAL || s == TRUE_KEYWORD || s == FALSE_KEYWORD || s == IDENTIFIER) {
        constant();
    }
    else if (s == NOT) {
        match(s);
        factor();
    }
    else {
        syntax_error();
    }
	depth--;
}

void Parser::variable_access()
{
	print("variable_access");
	depth++;
    match(IDENTIFIER);
    variable_access_end();
	depth--;
}

void Parser::variable_access_end()
{
	print("variable_access_end");
	depth++;
    std::set<Symbol> follow{MULTIPLY, DIVIDE, MODULO, ADD, SUBTRACT, LESS_THAN, EQUALS, 
        GREATER_THAN, AND, OR, SEMICOLON, COMMA, RIGHT_ARROW, RIGHT_PARENTHESIS, RIGHT_BRACKET,
        ASSIGN};
    auto s = next_token->symbol;
    if (s == LEFT_BRACKET) {
        match(s);
        expression();
        match(RIGHT_BRACKET);
    }
    else if (follow.find(s) != follow.end()) {
    }
    else {
        syntax_error();
    }
	depth--;
}

void Parser::indexed_selector()
{
	print("indexed_selector");
	depth++;
    match(LEFT_BRACKET);
    expression();
    match(RIGHT_BRACKET);
	depth--;
}

void Parser::constant()
{
	print("constant");
    auto s = next_token->symbol;
    if (s == NUMERAL || s == TRUE_KEYWORD || s == FALSE_KEYWORD || s == IDENTIFIER) {
        match(s);
    }
}

    