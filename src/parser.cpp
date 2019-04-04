#include "parser.h"
#include <iostream>
#include <cassert>
#include <set>

using std::cerr;
using std::endl;

// Slightly simpler syntax for looking up an element of a set, return true if found
bool sfind(std::set<Symbol> set, Symbol s) { return set.find(s) != set.end(); }

// Groups of types
// bool const_type(PLType t) { return t == PLType::BOOL_CONST || t == PLType::INT_CONST; }
// bool bool_type(PLType t) { return t == PLType::BOOL_CONST || t == PLType::BOOL_VAR; }
// bool int_type(PLType t) { return t == PLType::INT_CONST || t == PLType::INT_VAR; }


Parser::Parser(): line{1}, depth{0}, num_errors{0}
{
    init_symbol_sets();
}


int Parser::verify_syntax(std::vector<Token> *input_tokens)
{
    num_errors = 0;
    line = 1;
    next_token = input_tokens->begin();
    try {
        skip_whitespace();
        program();  
    }
    catch (const eof_error &e) {
        error_preamble();
        cerr << e.what() << endl;
    }
    return num_errors;
}


void Parser::read_next()
{
    if (next_token->symbol == END_OF_FILE) {
        throw eof_error();
    }
    next_token++;
    skip_whitespace();
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


void Parser::match(Symbol s, std::string nonterminal)
{
    if (s != next_token->symbol) {
        error_preamble();
        cerr << "Expected " << SYMBOL_STRINGS.at(s) << ", found " 
             << SYMBOL_STRINGS.at(next_token->symbol) << std::endl;
        synchronize(nonterminal);
    }
    if (s == IDENTIFIER) {
        matched_id = *next_token;
    }
    read_next();
}


void Parser::error_preamble()
{
    num_errors++;
    cerr << "Error " << num_errors << " on line " << line << ": ";
}


void Parser::syntax_error(std::string nonterminal)
{
    error_preamble();
    cerr << "Unexpected " << SYMBOL_STRINGS.at(next_token->symbol) << " symbol" << endl;
    synchronize(nonterminal);
}


void Parser::type_error(std::string msg)
{
    error_preamble();
    std::cout << "TYPE ERROR - " << msg << std::endl;
}


void Parser::synchronize(std::string nonterminal)
{
    auto sync = follow[nonterminal];
    // Skip input tokens until something in the follow set of the current token is found
    while (!sfind(sync, next_token->symbol)) {
        read_next();
    }
    cerr << "-- Resuming from " << SYMBOL_STRINGS.at(next_token->symbol) << " on line " 
         << line << endl; 
}


void Parser::check_follow(std::string nonterminal)
{
    if (!sfind(follow[nonterminal], next_token->symbol)) {
        syntax_error(nonterminal);
    }
}


void Parser::define_var(std::string id, PLType type, int size, bool constant) {
    try {
        block_table.insert(id, type, size, constant);
    }
    catch (const scope_error &e) {
        error_preamble();
        cerr << e.what() << endl;
    }
}


PLType Parser::get_type(std::string id) 
{
    try {
        BlockData &t = block_table.find(id);
        return t.type;
    }
    catch (const scope_error &e) {
        error_preamble();
        cerr << e.what() << endl;
        return PLType::UNDEFINED;
    }
}


void Parser::program()
{
    std::string nonterm = "program";
    block_table.push_new();
    block();
    block_table.pop();
    match(PERIOD, nonterm);
    // Matching an eof token would normally produce an error message, so handle in special case here
    if (next_token->symbol != END_OF_FILE) {
        num_errors++;
        std::cout << "Error " << num_errors << " on line " << line << ": Expected " 
                  << SYMBOL_STRINGS.at(END_OF_FILE) << ", found " 
                  << SYMBOL_STRINGS.at(next_token->symbol) << std::endl;
    }
}


void Parser::block()
{
    std::string nonterm = "block";
    match(BEGIN, nonterm);
    definition_part();
    statement_part();
    match(END, nonterm);
}


void Parser::definition_part()
{
    std::string nonterm = "definition_part";
    std::set<Symbol> first{CONST, INT, BOOL, PROC};
    auto s = next_token->symbol;
    if (sfind(first, s)) {
        definition();
        match(SEMICOLON, nonterm);        
        
        definition_part();
    }
    // epsilon production
    else {
        check_follow(nonterm);        
    }
}


void Parser::definition()
{
    std::string nonterm = "definition";
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
        syntax_error(nonterm);
    }
}


void Parser::constant_definition()
{
    std::string nonterm = "constant_definition";
    match(CONST, nonterm);    
    match(IDENTIFIER, nonterm);
    auto id = matched_id.lexeme;
    match(EQUALS, nonterm);
    auto type = constant();
    define_var(id, type, 1, true);
}


void Parser::variable_definition()
{
	std::string nonterm = "variable_definition";
    auto type = type_symbol();
    variable_definition_type(type);
}


void Parser::variable_definition_type(PLType varlist_type)
{
    std::string nonterm = "variable_definition_type";
    std::vector<std::string> vars;
    int size = 1;
    auto s = next_token->symbol;
    if (s == IDENTIFIER) {
        variable_list(vars);
    }
    else if (s == ARRAY) { 
        // if (equals(varlist_type, PLType::INTEGER)) {
            // varlist_type = PLType::INTEGER_ARRAY;
        // }
        // else if (equals(varlist_type, PLType::BOOLEAN)) {
            // varlist_type = PLType::BOOLEAN_ARRAY;
        // }
        match(ARRAY, nonterm);
        variable_list(vars);
        match(LEFT_BRACKET, nonterm);
        auto arr_size_type = constant();
        if (!equals(arr_size_type, PLType::INTEGER)) {
            type_error("Array bounds must be of type integer");
        }
        match(RIGHT_BRACKET, nonterm);
    }
    else {
        syntax_error(nonterm);
    }
    for (auto &id: vars) {
        define_var(id, varlist_type, size, false);
    }
}


PLType Parser::type_symbol()
{
    std::string nonterm = "type_symbol";
    auto s = next_token->symbol;
    if (s == INT) {
        match(s, nonterm);
        return PLType::INTEGER;
    }
    else if (s == BOOL) {
        match(s, nonterm);
        return PLType::BOOLEAN;
    }
    else {
        syntax_error(nonterm);
        return PLType::UNDEFINED;
    }
}


void Parser::variable_list(std::vector<std::string> &var_list)
{
    std::string nonterm = "variable_list";
    match(IDENTIFIER, nonterm);
    var_list.push_back(matched_id.lexeme);
    variable_list_end(var_list);
}


void Parser::variable_list_end(std::vector<std::string> &var_list)
{
    std::string nonterm = "variable_list_end";
    auto s = next_token->symbol;
    if (s == COMMA) {
        match(s, nonterm);
        match(IDENTIFIER, nonterm);
        var_list.push_back(matched_id.lexeme);
        variable_list_end(var_list);
    }
    // epsilon production
    else {
        check_follow(nonterm);
    }
}


void Parser::procedure_definition()
{
    std::string nonterm = "procedure_definition";
    match(PROC, nonterm);    
    match(IDENTIFIER, nonterm);
    auto id = matched_id.lexeme;
    define_var(id, PLType::PROCEDURE, 1, false);
    block_table.push_new();
    block();
    block_table.pop();
} 


void Parser::statement_part()
{
    std::string nonterm = "statement_part";
    std::set<Symbol> first{SKIP, READ, WRITE, IDENTIFIER, CALL, IF, DO};
    auto s = next_token->symbol;
    if (sfind(first, s)) {
        statement();
        match(SEMICOLON, nonterm);        
        
        statement_part();
    }
    // epsilon production
    else {
		check_follow(nonterm);
	}
}


void Parser::statement()
{
    std::string nonterm = "statement";
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
        syntax_error(nonterm);
    }
}


void Parser::empty_statement()
{
    std::string nonterm = "empty_statement";
    match(SKIP, nonterm);
}


void Parser::read_statement()
{
    std::string nonterm = "read_statement";
    match(READ, nonterm);
    std::vector<std::string> vars;
    variable_access_list(vars);
    for (auto id: vars) {
        try {
            BlockData &data = block_table.find(id);
            if (data.constant) {
                type_error("Cannot read value for constant " + id);
            }
            else if (equals(data.type, PLType::PROCEDURE)) {
                type_error("Cannot read value for procedure " + id);
            }
        }
        catch (const scope_error &e) {
            error_preamble();
            cerr << e.what() << endl;
        }
    }
}


void Parser::variable_access_list(std::vector<std::string> &vars)
{
	std::string nonterm = "variable_access_list";
    auto var_id = variable_access();
    vars.push_back(var_id);
    variable_access_list_end(vars);
}


void Parser::variable_access_list_end(std::vector<std::string> &vars)
{
    std::string nonterm = "variable_access_list_end";
    std::set<Symbol> first{SEMICOLON, ASSIGN};
    auto s = next_token->symbol;
    if (s == COMMA) {
        match(COMMA, nonterm);
        auto id = variable_access();
        vars.push_back(id);
        variable_access_list_end(vars);
    }
    // epsilon production 
    else {
		check_follow(nonterm);
	}
}


void Parser::write_statement()
{
    std::string nonterm = "write_statement";
    match(WRITE, nonterm);
    std::vector<PLType> types;
    expression_list(types);
    for (auto t: types) {
        if (!equals(t, PLType::PROCEDURE)) {
            type_error("Cannot write procedure");
        }
    }
}


void Parser::expression_list(std::vector<PLType> &types)
{
	std::string nonterm = "expression_list";
    auto expr_type = expression();
    types.push_back(expr_type);
    expression_list_end(types);
}


void Parser::expression_list_end(std::vector<PLType> &types)
{
    std::string nonterm = "expression_list_end";
    auto s = next_token->symbol;
    if (s == COMMA) {
        match(s, nonterm);
        expression_list(types);
    }
    // epsilon production 
    else {
		check_follow(nonterm);
	}
}


void Parser::assignment_statement()
{
    std::string nonterm = "assignment_statement";
    std::vector<std::string> vars;
    std::vector<PLType> expr_types;
    variable_access_list(vars);
    match(ASSIGN, nonterm);
    expression_list(expr_types);

    if (vars.size() != expr_types.size()) {
        error_preamble();
        std::cout << "Number of variables does not match number of expressions" << std::endl;
    }
    else {
        for (int i = 0; i < vars.size(); i++) {
            auto id = vars[i];
            auto e_type = expr_types[i];
            try {
                BlockData &data = block_table.find(id);
                if (data.constant) {
                    type_error("Cannot assign value to constant " + id);
                }
                else if (!equals(data.type, e_type)) {
                    type_error("Mismatch between types of LHS and RHS of assignment statement");
                }
            }
            catch (const scope_error &e) {
                error_preamble();
                cerr << e.what() << endl;
            }
        }
    }
}

void Parser::procedure_statement()
{
    std::string nonterm = "procedure_statement";
    match(CALL, nonterm);
    match(IDENTIFIER, nonterm);
    auto id = matched_id.lexeme;
    auto type = get_type(id);
    if (type != PLType::PROCEDURE) {
        type_error("Cannot call a non procedure type");
    }
}


void Parser::if_statement()
{
	std::string nonterm = "if_statement";
    match(IF, nonterm);
    guarded_command_list();
    match(FI, nonterm);
}


void Parser::do_statement()
{
	std::string nonterm = "do_statement";
    match(DO, nonterm);
    guarded_command_list();
    match(OD, nonterm);
}


void Parser::guarded_command_list()
{
	std::string nonterm = "guarded_command_list";
    guarded_command();
    guarded_command_list_end();
}


void Parser::guarded_command_list_end()
{
    std::string nonterm = "guarded_command_list_end";
    auto s = next_token->symbol;
    if (s == DOUBLE_BRACKET) {
        match(s, nonterm);
        guarded_command();
        
        guarded_command_list_end();
    }
    // epsilon production 
    else {
		check_follow(nonterm);
	}
}


void Parser::guarded_command()
{
    std::string nonterm = "guarded_command";
    
    auto guard_type = expression();
    if (!equals(guard_type, PLType::BOOLEAN)) {
        type_error("Guarded command must evaluate to Boolean type");
    }
    match(RIGHT_ARROW, nonterm);
    statement_part();
}


PLType Parser::expression()
{
    std::string nonterm = "expression";
    auto lhs = primary_expression();
    expression_end(lhs);
    return lhs;
}


void Parser::expression_end(PLType lhs)
{
    std::string nonterm = "expression_end";
    auto s = next_token->symbol;
    if (s == AND || s == OR) {
        primary_operator();
        auto rhs = expression();
        if (!(equals(lhs, PLType::BOOLEAN) && equals(rhs, PLType::BOOLEAN))) {
            type_error("Both operands for logical operator must be Boolean");
        }
    }
    // epsilon production 
    else {
		check_follow(nonterm);
	}
}


void Parser::primary_operator()
{
    std::string nonterm = "primary_operator";
    auto s = next_token->symbol;
    if (s == AND || s == OR) {
        match(s, nonterm);
    }
    else {
        syntax_error(nonterm);
    }
}


PLType Parser::primary_expression()
{
	std::string nonterm = "primary_expression";
    auto lhs_type = simple_expression();
    return primary_expression_end(lhs_type);
}


PLType Parser::primary_expression_end(PLType lhs_type)
{
    std::string nonterm = "primary_expression_end";
    auto s = next_token->symbol;
    if (s == LESS_THAN || s == GREATER_THAN || s == EQUALS) {
        relational_operator();
        auto rhs_type = primary_expression();
        if (!(equals(lhs_type, PLType::INTEGER) && equals(PLType::INTEGER, rhs_type))) {
            type_error("Both operands of a comparison must be integers");
            return PLType::UNDEFINED;
        }
        return lhs_type
    }
    // epsilon production 
    else {
		check_follow(nonterm);
        return lhs_type;
	}
}


void Parser::relational_operator()
{
    std::string nonterm = "relational_operator";
    auto s = next_token->symbol;
    if (s == LESS_THAN || s == EQUALS || s == GREATER_THAN) {
        match(s, nonterm);
    }
    else {
        syntax_error(nonterm);
    }
}


PLType Parser::simple_expression()
{
    std::string nonterm = "simple_expression";
    std::set<Symbol> first{NOT, LEFT_PARENTHESIS, NUMERAL, TRUE_KEYWORD, FALSE_KEYWORD, IDENTIFIER};
    auto s = next_token->symbol;
    if (s == SUBTRACT) {
        match(s, nonterm);
        auto lhs_type = term();
        if (!equals(lhs_type, PLType::INTEGER)) {
            type_error("Cannot negate a non integer value");
            lhs_type = PLType::UNDEFINED;
        }
        return simple_expression_end(lhs);
    }
    else if (sfind(first, s)) {
        auto lhs = term();
        return simple_expression_end(lhs);
    }
    else {
        syntax_error(nonterm);
        return PLType::UNDEFINED;
    }
}


PLType Parser::simple_expression_end(PLType lhs_type)
{
    std::string nonterm = "simple_expression_end";
    auto s = next_token->symbol;
    if (s == ADD || s == SUBTRACT) {
        adding_operator();
        auto rhs_type = term();
        if (!equals(lhs_type, PLType::INTEGER) || !equals(rhs_type, PLType::INTEGER)) {
            type_error("Both operands of addition type operator must be integers");
            lhs_type = PLType::UNDEFINED;
        }
        return simple_expression_end(lhs_type);
    }
    // epsilon production 
    else {
        check_follow(nonterm); 
        return lhs_type
    }
}


void Parser::adding_operator()
{
    std::string nonterm = "adding_operator";
    auto s = next_token->symbol;
    if (s == ADD || s == SUBTRACT) {
        match(s, nonterm);
    }
    else {
        syntax_error(nonterm);
    }
}


PLType Parser::term()
{
    std::string nonterm = "term";
    auto type = factor();
    return term_end(type);
}


PLType Parser::term_end(PLType lhs_type)
{
    std::string nonterm = "term_end";
    auto s = next_token->symbol;
    if (s == MULTIPLY || s == DIVIDE || s == MODULO) {
        multiplying_operator();
        auto rhs_type = factor();
        if (!equals(lhs_type, PLType::INTEGER || !equals(rhs_type, PLType::INTEGER))) {
            type_error("Both operands of multiplication type operators must be integers");
            lhs_type = PLType::UNDEFINED;
        }
        return term_end(lhs_type);
    }
    // epsilon production 
    else {
        check_follow(nonterm);
        return lhs_type;
    }
}


void Parser::multiplying_operator()
{
    std::string nonterm = "multiplying_operator";
    auto s = next_token->symbol;
    if (s == MULTIPLY || s == DIVIDE || s ==  MODULO) {
        match(s, nonterm);
    }
    else {
        syntax_error(nonterm);
    }
}


PLType Parser::factor()
{
    std::string nonterm = "factor";
    auto s = next_token->symbol;
    if (s == LEFT_PARENTHESIS) {
        match(s, nonterm);
        auto type = expression();
        match(RIGHT_PARENTHESIS, nonterm);
        return type;
    }
    else if (s == IDENTIFIER) {
        id =  variable_access();
        try {
            BlockData &data = block_table.find(id);
            return data.type;
        }
        catch (const scope_error &e) {
            error_preamble();
            cerr << e.what() << endl;
            return PLType::UNDEFINED;
        }
    }
    else if (s == NUMERAL || s == TRUE_KEYWORD || s == FALSE_KEYWORD) {
        return constant();
    }
    else if (s == NOT) {
        match(s, nonterm);
        auto type = factor();
        if (!equals(type, PLType::BOOLEAN)) {
            type_error("Cannot take logical inverse of a non Boolean expression");
            return PLType::UNDEFINED;
        }
        return type;
    }
    else {
        syntax_error(nonterm);
        return PLType::UNDEFINED;
    }
}


std::string Parser::variable_access()
{
    std::string nonterm = "variable_access";
    match(IDENTIFIER, nonterm);
    auto id = matched_id.lexeme;
    // auto type = get_type(id);
    variable_access_end();
    return id;
}


void Parser::variable_access_end()
{
    std::string nonterm = "variable_access_end";
    auto s = next_token->symbol;
    if (s == LEFT_BRACKET) {
        indexed_selector();
    }
    // epsilon production 
    else {
        check_follow(nonterm);
    }
}


void Parser::indexed_selector()
{
    std::string nonterm = "indexed_selector";
    match(LEFT_BRACKET, nonterm);
    auto ind_type = expression();
    if (!equals(ind_type, PLType::INTEGER)) {
        error_preamble();
        std::cout << "Array index must be integer type" << std::endl;
    }
    match(RIGHT_BRACKET, nonterm);
}


PLType Parser::constant()
{
    std::string nonterm = "constant";
    auto s = next_token->symbol;
    if (s == NUMERAL) {
        match(s, nonterm);
        return PLType::INTEGER;
    }
    else if (s == TRUE_KEYWORD || s == FALSE_KEYWORD) {
        match(s, nonterm);
        return PLType::BOOLEAN;
    }
    else if (s == IDENTIFIER) {
        auto id = next_token->lexeme;
        match(s, nonterm);
        try {
            BlockData &data = block_table.find(id);
            if (!data.constant) {
                error_preamble();
                cerr << "Found non-constant value where constant expected" << endl;
                return PLType::UNDEFINED;
            }
            return data.type;
        }
        catch (const scope_error &e) {
            cerr << e.what() << endl;
            return PLType::UNDEFINED;
        }
    }
    else {
        syntax_error(nonterm);
        return PLType::UNDEFINED;
    }
}


void Parser::init_symbol_sets()
{
    follow["program"] = {END_OF_FILE};
    follow["block"] = {PERIOD, SEMICOLON};
    follow["definition_part"] = {SKIP, READ, WRITE, CALL, DO, READ, IF, IDENTIFIER, END};
    follow["definition"] = {SEMICOLON};
    follow["constant_definition"] = {SEMICOLON};
    follow["variable_definition"] = {SEMICOLON};
    follow["variable_definition_type"] = {SEMICOLON};
    follow["type_symbol"] = {ARRAY, IDENTIFIER};
    follow["variable_list"] = {LEFT_BRACKET, SEMICOLON};
    follow["variable_list_end"] = {LEFT_BRACKET, SEMICOLON};
    follow["procedure_definition"] = {SEMICOLON};
    follow["statement_part"] = {END, DOUBLE_BRACKET, OD, FI};
    follow["statement"] = {SEMICOLON};
    follow["empty_statement"] = {SEMICOLON};
    follow["read_statement"] = {SEMICOLON};
    follow["variable_access_list"] = {ASSIGN, SEMICOLON};
    follow["variable_access_list_end"] = {ASSIGN, SEMICOLON};
    follow["write_statement"] = {SEMICOLON};
    follow["expression_list"] = {SEMICOLON};
    follow["expression_list_end"] = {SEMICOLON};
    follow["assignment_statement"] = {SEMICOLON};
    follow["procedure_statement"] = {SEMICOLON};
    follow["if_statement"] = {SEMICOLON};
    follow["do_statement"] = {SEMICOLON};
    follow["guarded_command_list"] = {OD, FI};
    follow["guarded_command_list_end"] = {OD, FI};
    follow["guarded_command"] = {DOUBLE_BRACKET, OD, FI};
    follow["expression"] = {RIGHT_BRACKET, RIGHT_PARENTHESIS, RIGHT_ARROW, COMMA, SEMICOLON};
    follow["expression_end"] = {RIGHT_BRACKET, RIGHT_PARENTHESIS, RIGHT_ARROW, COMMA, SEMICOLON};
    follow["primary_operator"] = {SUBTRACT, LEFT_PARENTHESIS, NOT, NUMERAL, FALSE_KEYWORD, 
        TRUE_KEYWORD, TRUE_KEYWORD, IDENTIFIER};
    follow["primary_expression"] = 
        {AND, OR, RIGHT_ARROW, RIGHT_BRACKET, RIGHT_PARENTHESIS, RIGHT_ARROW, COMMA, SEMICOLON};
    follow["primary_expression_end"] = 
        {AND, OR, RIGHT_ARROW, RIGHT_BRACKET, RIGHT_PARENTHESIS, RIGHT_ARROW, COMMA, SEMICOLON};
    follow["relational_operator"] = 
        {SUBTRACT, LEFT_PARENTHESIS, NOT, NUMERAL, FALSE_KEYWORD, TRUE_KEYWORD, IDENTIFIER};
    follow["simple_expression"] = {LESS_THAN, EQUALS, GREATER_THAN, AND, OR, RIGHT_BRACKET, 
        RIGHT_PARENTHESIS, RIGHT_ARROW, COMMA, SEMICOLON};
    follow["simple_expression_end"] = {LESS_THAN, EQUALS, GREATER_THAN, AND, OR, RIGHT_BRACKET, 
        RIGHT_PARENTHESIS, RIGHT_ARROW, COMMA, SEMICOLON};
    follow["adding_operator"] = 
        {LEFT_PARENTHESIS, NOT, NUMERAL, FALSE_KEYWORD, TRUE_KEYWORD, IDENTIFIER};
    follow["term"] = {ADD, SUBTRACT, LESS_THAN, EQUALS, GREATER_THAN, AND, OR, RIGHT_BRACKET, 
        RIGHT_PARENTHESIS, RIGHT_ARROW, COMMA, SEMICOLON};
    follow["term_end"] = {ADD, SUBTRACT, LESS_THAN, EQUALS, GREATER_THAN, AND, OR, RIGHT_BRACKET, 
        RIGHT_PARENTHESIS, RIGHT_ARROW, COMMA, SEMICOLON};
    follow["multiplying_operator"] = 
        {LEFT_BRACKET, NOT, NUMERAL, FALSE_KEYWORD, TRUE_KEYWORD, IDENTIFIER};
    follow["factor"] = 
        {MULTIPLY, DIVIDE, MODULO, ADD, SUBTRACT, LESS_THAN, EQUALS, GREATER_THAN, AND, OR, 
         RIGHT_BRACKET, RIGHT_PARENTHESIS, RIGHT_ARROW, COMMA, SEMICOLON};
    follow["variable_access"] = 
        {MULTIPLY, DIVIDE, MODULO, ADD, SUBTRACT, LESS_THAN, EQUALS, GREATER_THAN, AND, OR, 
         RIGHT_BRACKET, RIGHT_PARENTHESIS, RIGHT_ARROW, COMMA, SEMICOLON, ASSIGN};
    follow["variable_access_end"] = 
        {MULTIPLY, DIVIDE, MODULO, ADD, SUBTRACT, LESS_THAN, EQUALS, GREATER_THAN, AND, OR, 
         RIGHT_BRACKET, RIGHT_PARENTHESIS, RIGHT_ARROW, COMMA, SEMICOLON, ASSIGN};
    follow["indexed_selector"] = 
        {MULTIPLY, DIVIDE, MODULO, ADD, SUBTRACT, LESS_THAN, EQUALS, GREATER_THAN, AND, OR, 
         RIGHT_BRACKET, RIGHT_PARENTHESIS, RIGHT_ARROW, COMMA, SEMICOLON, ASSIGN};
    follow["constant"] = 
        {MULTIPLY, DIVIDE, MODULO, ADD, SUBTRACT, LESS_THAN, EQUALS, GREATER_THAN, AND, OR, 
         RIGHT_BRACKET, RIGHT_PARENTHESIS, RIGHT_ARROW, COMMA, SEMICOLON};
}