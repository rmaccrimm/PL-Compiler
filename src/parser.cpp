#include "parser.h"
#include <iostream>
#include <cassert>
#include <set>

using std::cerr;
using std::endl;

// Change to 1 for debugging purposes
#define PRINT 0
#define INDENT 1

/*  Three macros are used to reduce some of the typing in the nonterminal functions.

    All nonterminal functions that call another nonterminal continue only if true is returned, 
    returning false otherwise.
*/
#define TRY(func) func()//if(!func()) return false

// Same as above, but passes an arugment
#define TRY_WITH_ARG(func, arg) func(arg) //if (!func(arg)) return false

/*  When matching, continue only if successful, otherwise attempt to synchronize and skip the
    rest of the current noterminal function
*/
#define MATCH_AND_SYNC(symbol, nonterminal) if (!match(symbol)) synchronize(nonterminal)


// Slightly simpler syntax for looking up an element of a set 
bool sfind(std::set<Symbol> set, Symbol s) { return set.find(s) != set.end(); }

// Groups of types
bool const_type(PLType t) { return t == PLType::BOOL_CONST || t == PLType::INT_CONST; }
bool bool_type(PLType t) { return t == PLType::BOOL_CONST || t == PLType::BOOL_VAR; }
bool int_type(PLType t) { return t == PLType::INT_CONST || t == PLType::INT_VAR; }


Parser::Parser(): line{1}, depth{0}, num_errors{0}
{
    init_symbol_sets();
}

// Debugging function - prints the call stack
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


bool Parser::match(Symbol s)
{
    if (s != next_token->symbol) {
        num_errors++;
        std::cout << "Error " << num_errors << " on line " << line << ": Expected " 
                  << SYMBOL_STRINGS.at(s) << ", found " << SYMBOL_STRINGS.at(next_token->symbol) 
                  << std::endl;
        return false;
    }
    if (s == IDENTIFIER) {
        matched_id = *next_token;
    }
    read_next();
    return true;
}


bool Parser::check_follow(std::string non_terminal)
{
    return sfind(follow[non_terminal], next_token->symbol);
}


void Parser::error_preamble()
{
    num_errors++;
    cerr << "Error " << num_errors << " on line " << line << ": ";
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
        auto t = block_table.find_type(id);
        return t;
    }
    catch (const scope_error &e) {
        error_preamble();
        cerr << e.what() << endl;
        return PLType::UNDEFINED;
    }
}


void Parser::syntax_error(std::string non_terminal)
{
    error_preamble();
    std::cout << "SYNTAX ERROR - Unexpected " << SYMBOL_STRINGS.at(next_token->symbol) 
              << " symbol" << std::endl; 
    synchronize(non_terminal);
}


void Parser::type_error(std::string msg)
{
    error_preamble();
    std::cout << "TYPE ERROR - " << msg << std::endl;
}


void Parser::synchronize(std::string non_terminal)
{
    auto sync = follow[non_terminal];
    // Skip input tokens until something in the follow set of the current token is found
    while (!sfind(sync, next_token->symbol)) {
        read_next();
    }
    std::cout << "-- Resuming from " << SYMBOL_STRINGS.at(next_token->symbol) << " on line " << line
              << std::endl; 
}


void Parser::program()
{
    std::string nonterm = "program";
    print(nonterm);
    depth++;
    block_table.push_new();
    TRY(block);
    depth--;
    block_table.pop();

    MATCH_AND_SYNC(PERIOD, nonterm);
    // Matching an eof token would normally produce an error message, so handle in special case here
    if (next_token->symbol != END_OF_FILE) {
        num_errors++;
        std::cout << "Error " << num_errors << " on line " << line << ": Expected " 
                  << SYMBOL_STRINGS.at(END_OF_FILE) << ", found " 
                  << SYMBOL_STRINGS.at(next_token->symbol) << std::endl;
    }
    // return true;
}


void Parser::block()
{
    std::string nonterm = "block";
	print(nonterm);
    MATCH_AND_SYNC(BEGIN, nonterm);
    depth++;
    TRY(definition_part);
    TRY(statement_part);
    depth--;
    MATCH_AND_SYNC(END, nonterm);
}


void Parser::definition_part()
{
    std::string nonterm = "definition_part";
	print(nonterm);
    depth++;
    std::set<Symbol> first{CONST, INT, BOOL, PROC};
    auto s = next_token->symbol;
    if (sfind(first, s)) {
        TRY(definition);
        MATCH_AND_SYNC(SEMICOLON, nonterm);        
        depth--;
        TRY(definition_part);
    }
    // epsilon production
    else if (!check_follow(nonterm)) {
        depth--;
        syntax_error(nonterm);
    }
    // return true;
}


void Parser::definition()
{
    std::string nonterm = "definition";
	print(nonterm);
    depth++;
    auto s = next_token->symbol;
    if (s == CONST) {
        TRY(constant_definition);
    }
    else if (s == INT || s == BOOL) {
        TRY(variable_definition);
    }
    else if (s == PROC) {
        TRY(procedure_definition);
    }
    else {
        return syntax_error(nonterm);
    }
    depth--;
    //return true;
}


void Parser::constant_definition()
{
    std::string nonterm = "constant_definition";
	print(nonterm);

    depth++;
    MATCH_AND_SYNC(CONST, nonterm);    
    MATCH_AND_SYNC(IDENTIFIER, nonterm);
    MATCH_AND_SYNC(EQUALS, nonterm);

    PLType type = PLType::UNDEFINED;
    TRY_WITH_ARG(constant, type);
    depth--;

    if (!const_type(type)) {
        type_error("Non const value cannot be assigned a constant");
    }
    auto id = matched_id.lexeme;
    define_var(id, type, 1, true);
    //return true;
}


void Parser::variable_definition()
{
	print("variable_definition");
    depth++;
    PLType type;
    // Type is first determined from type_symbol
    TRY_WITH_ARG(type_symbol, type);
    // type contains the type of the list of identifiers that follows
    TRY_WITH_ARG(variable_definition_type, type);
    depth--;
    //return true;
}


void Parser::variable_definition_type(PLType varlist_type)
{
    std::string nonterm = "variable_definition_type";
    print(nonterm);
	depth++;
    std::vector<std::string> vars;
    int size = 1;

    auto s = next_token->symbol;
    if (s == IDENTIFIER) {
        TRY_WITH_ARG(variable_list, vars);
    }
    else if (s == ARRAY) { 
        MATCH_AND_SYNC(ARRAY, nonterm);
        TRY_WITH_ARG(variable_list, vars);
        MATCH_AND_SYNC(LEFT_BRACKET, nonterm);

        PLType bound_type = PLType::UNDEFINED;
        TRY_WITH_ARG(constant, bound_type);

        if (bound_type != PLType::INT_CONST) {
            type_error("Array bounds must be of type const integer");
        }
        MATCH_AND_SYNC(RIGHT_BRACKET, nonterm);
    }
    else {
        return syntax_error(nonterm);
    }
	depth--;
    for (auto &id: vars) {
        define_var(id, varlist_type, size, false);
    }
    //return true;
}


void Parser::type_symbol(PLType &type)
{
    std::string nonterm = "type_symbol";
    print(nonterm);
	depth++;
    auto s = next_token->symbol;
    if (s == INT) {
        type = PLType::INT_VAR;
        MATCH_AND_SYNC(s, nonterm);
    }
    else if (s == BOOL) {
        type = PLType::BOOL_VAR;
        MATCH_AND_SYNC(s, nonterm);
    }
    else {
        return syntax_error(nonterm);
    }
	depth--;
    //return true;
}


void Parser::variable_list(std::vector<std::string> &var_list)
{
    std::string nonterm = "variable_list";
    print(nonterm);
	depth++;
    MATCH_AND_SYNC(IDENTIFIER, nonterm);
    var_list.push_back(matched_id.lexeme);
    TRY_WITH_ARG(variable_list_end, var_list);
	depth--;
    //return true;
}


void Parser::variable_list_end(std::vector<std::string> &var_list)
{
    std::string nonterm = "variable_list_end";
    print(nonterm);
	depth++;
    auto s = next_token->symbol;
    if (s == COMMA) {
        MATCH_AND_SYNC(s, nonterm);
        MATCH_AND_SYNC(IDENTIFIER, nonterm);
        var_list.push_back(matched_id.lexeme);
        TRY_WITH_ARG(variable_list_end, var_list);
    }
    // epsilon production
    else if (!check_follow(nonterm)) {
        return syntax_error(nonterm);
    }
    depth--;
    //return true;
}


void Parser::procedure_definition()
{
    std::string nonterm = "procedure_definition";
	print(nonterm);
	depth++;
    MATCH_AND_SYNC(PROC, nonterm);    
    MATCH_AND_SYNC(IDENTIFIER, nonterm);
    auto id = matched_id.lexeme;

    define_var(id, PLType::PROCEDURE, 1, false);
    block_table.push_new();
    // bool success = block();
    block();
    block_table.pop();
	depth--;
    // if (!success) {
        // return false;
    // }
    //return true;
} 


void Parser::statement_part()
{
    std::string nonterm = "statement_part";
	print(nonterm);
	depth++;
    std::set<Symbol> first{SKIP, READ, WRITE, IDENTIFIER, CALL, IF, DO};
    auto s = next_token->symbol;
    if (sfind(first, s)) {
        TRY(statement);
        MATCH_AND_SYNC(SEMICOLON, nonterm);        
        depth--;
        TRY(statement_part);
    }
    // epsilon production
    else if (!check_follow(nonterm)) {
        depth--;
        return syntax_error(nonterm);
    }
    //return true;
}


void Parser::statement()
{
    std::string nonterm = "statement";
	print(nonterm);
	depth++;
    auto s = next_token->symbol;
    if (s == SKIP) {
        TRY(empty_statement);
    }
    else if (s == READ) {
        TRY(read_statement);
    }
    else if (s == WRITE) {
        TRY(write_statement);
    }
    else if (s == IDENTIFIER) {
        TRY(assignment_statement);
    }
    else if (s == CALL) {
        TRY(procedure_statement);
    }
    else if (s == IF) {
        TRY(if_statement);
    }
    else if (s == DO) {
        TRY(do_statement);
    }
    else {
        syntax_error(nonterm);
    }
	depth--;
    //return true;
}


void Parser::empty_statement()
{
    std::string nonterm = "empty_statement";
	print(nonterm);
	depth++;
    MATCH_AND_SYNC(SKIP, nonterm);
	depth--;
    //return true;
}


void Parser::read_statement()
{
    std::string nonterm = "read_statement";
	print(nonterm);
	depth++;
    MATCH_AND_SYNC(READ, nonterm);
    std::vector<PLType> types;
    TRY_WITH_ARG(variable_access_list, types);
    for (auto t: types) {
        if (const_type(t)) {
            type_error("Cannot read value for constant");
        }
        if (t == PLType::PROCEDURE || t == PLType::UNDEFINED) {
            std::string ts = (t == PLType::PROCEDURE ? "procedure" : "undefined");
            type_error("Cannot read variable of type " + ts);
        }
    }
	depth--;
    //return true;
}


void Parser::variable_access_list(std::vector<PLType> &types)
{
	print("variable_access_list");
	depth++;
    PLType var_type = PLType::UNDEFINED;
    TRY_WITH_ARG(variable_access, var_type);
    types.push_back(var_type);
    TRY_WITH_ARG(variable_access_list_end, types);
	depth--;
    //return true;
}


void Parser::variable_access_list_end(std::vector<PLType> &types)
{
    std::string nonterm = "variable_access_list_end";
    print(nonterm);
	depth++;
    std::set<Symbol> first{SEMICOLON, ASSIGN};
    auto s = next_token->symbol;
    if (s == COMMA) {
        MATCH_AND_SYNC(COMMA, nonterm);
        PLType var_type = PLType::UNDEFINED;
        TRY_WITH_ARG(variable_access, var_type);
        types.push_back(var_type);
        TRY_WITH_ARG(variable_access_list_end, types);
    }
    // epsilon production 
    else if (!check_follow(nonterm)) {
        return syntax_error(nonterm);
    }
    depth--;
    //return true;
}


void Parser::write_statement()
{
    std::string nonterm = "write_statement";
	print(nonterm);
	depth++;
    MATCH_AND_SYNC(WRITE, nonterm);
    std::vector<PLType> types;
    TRY_WITH_ARG(expression_list, types);
    for (auto t: types) {
        if (!int_type(t) && !bool_type(t)) {
            std::string ts = (t == PLType::PROCEDURE ? "procedure" : "undefined");
            type_error("Cannot write variable of type " + ts);
        }
    }
	depth--;
    //return true;
}


void Parser::expression_list(std::vector<PLType> &types)
{
	print("expression_list");
	depth++;
    PLType expr_type = PLType::UNDEFINED; 
    TRY_WITH_ARG(expression, expr_type);
    types.push_back(expr_type);
    TRY_WITH_ARG(expression_list_end, types);
	depth--;
    //return true;
}


void Parser::expression_list_end(std::vector<PLType> &types)
{
    std::string nonterm = "expression_list_end";
    print(nonterm);
	depth++;
    auto s = next_token->symbol;
    if (s == COMMA) {
        MATCH_AND_SYNC(s, nonterm);
        PLType expr_type = PLType::UNDEFINED;
        TRY_WITH_ARG(expression, expr_type);
        types.push_back(expr_type);
        TRY_WITH_ARG(expression_list_end, types);
    }
    // epsilon production 
    else if (!check_follow(nonterm)) {
        return syntax_error(nonterm);
    }
    depth--;
    //return true;
}


void Parser::assignment_statement()
{
    std::string nonterm = "assignment_statement";
    print(nonterm);
	depth++;
    std::vector<PLType> var_types, expr_types;
    TRY_WITH_ARG(variable_access_list, var_types);
    MATCH_AND_SYNC(ASSIGN, nonterm);
    TRY_WITH_ARG(expression_list, expr_types);
	depth--;

    if (var_types.size() != expr_types.size()) {
        error_preamble();
        std::cout << "Number of variables does not match number of expressions" << std::endl;
    }
    else {
        for (int i = 0; i < var_types.size(); i++) {
            auto v = var_types[i];
            auto e = expr_types[i];
            if (const_type(v)) {
                type_error("Cannot assign a value to const type");
            }
            else if (!((int_type(v) && int_type(e)) || (bool_type(v) && bool_type(e)))) {
                type_error("Mismatch between types of LHS and RHS of assignment statement");
            }
        }
    }
    //return true;
}

void Parser::procedure_statement()
{
    std::string nonterm = "procedure_statement";
	print(nonterm);
	depth++;
    MATCH_AND_SYNC(CALL, nonterm);
    MATCH_AND_SYNC(IDENTIFIER, nonterm);
    auto id = matched_id.lexeme;

    auto type = get_type(id);
    if (type != PLType::PROCEDURE) {
        type_error("Cannot call a non procedure type");
    }
	depth--;
    //return true;
}


void Parser::if_statement()
{
	std::string nonterm = "if_statement";
    print(nonterm);
	depth++;
    MATCH_AND_SYNC(IF, nonterm);
    TRY(guarded_command_list);
    MATCH_AND_SYNC(FI, nonterm);
	depth--;
    //return true;
}


void Parser::do_statement()
{
	std::string nonterm = "do_statement";
    print(nonterm);
	depth++;
    MATCH_AND_SYNC(DO, nonterm);
    TRY(guarded_command_list);
    MATCH_AND_SYNC(OD, nonterm);
	depth--;
    //return true;
}


void Parser::guarded_command_list()
{
	print("guarded_command_list");
	depth++;
    TRY(guarded_command);
    TRY(guarded_command_list_end);
	depth--;
    //return true;
}


void Parser::guarded_command_list_end()
{
    std::string nonterm = "guarded_command_list_end";
    print(nonterm);
	depth++;
    auto s = next_token->symbol;
    if (s == DOUBLE_BRACKET) {
        MATCH_AND_SYNC(s, nonterm);
        TRY(guarded_command);
        depth--;
        TRY(guarded_command_list_end);
    }
    // epsilon production 
    else if (!check_follow(nonterm)) {
        depth--;
        return syntax_error(nonterm);
    }
    //return true;
}


void Parser::guarded_command()
{
    std::string nonterm = "guarded_command";
    print(nonterm);
	depth++;
    auto guard_type = PLType::UNDEFINED;
    TRY_WITH_ARG(expression, guard_type);
    if (!bool_type(guard_type)) {
        type_error("Guarded command must evaluate to Boolean type");
    }
    MATCH_AND_SYNC(RIGHT_ARROW, nonterm);
    TRY(statement_part);
	depth--;
    //return true;
}


void Parser::expression(PLType &type)
{
	print("expression");
	depth++;
    TRY_WITH_ARG(primary_expression, type);
    TRY_WITH_ARG(expression_end, type);
	depth--;
    //return true;
}


void Parser::expression_end(PLType &lhs_type)
{
    std::string nonterm = "expression_end";
    print(nonterm);
	depth++;
    auto s = next_token->symbol;
    if (s == AND || s == OR) {
        TRY(primary_operator);
        auto rhs_type = PLType::UNDEFINED;
        TRY_WITH_ARG(primary_expression, rhs_type);
        TRY_WITH_ARG(expression_end, rhs_type);
        if (!bool_type(lhs_type) || !bool_type(rhs_type)) {
            type_error("Both sides of logical operation must be Boolean");
            lhs_type = PLType::UNDEFINED;
        }
        else {
            lhs_type = PLType::BOOL_CONST;
        }
    }
    // epsilon production 
    else if (!check_follow(nonterm)) {
        return syntax_error(nonterm);
    }
    depth--;
    //return true;
}


void Parser::primary_operator()
{
    std::string nonterm = "primary_operator";
    print(nonterm);
	depth++;
    auto s = next_token->symbol;
    if (s == AND || s == OR) {
        match(s);
    }
    else {
        return syntax_error(nonterm);
    }
	depth--;
    //return true;
}


void Parser::primary_expression(PLType &type)
{
	print("primary_expression");
	depth++;
    TRY_WITH_ARG(simple_expression, type);
    TRY_WITH_ARG(primary_expression_end, type);
	depth--;
    //return true;
}


void Parser::primary_expression_end(PLType &lhs_type)
{
    std::string nonterm = "primary_expression_end"; 
    print(nonterm);
	depth++;
    auto s = next_token->symbol;
    if (s == LESS_THAN || s == GREATER_THAN || s == EQUALS) {
        TRY(relational_operator);
        auto rhs_type = PLType::UNDEFINED;
        TRY_WITH_ARG(simple_expression, rhs_type);
        if (!int_type(lhs_type) || !int_type(rhs_type)) {
            type_error("Both operands of a comparison must be integers");
            lhs_type = PLType::UNDEFINED;
        }
        else {
            lhs_type = PLType::BOOL_CONST;
        }
    }
    // epsilon production 
    else if (!check_follow(nonterm)) {
        return syntax_error(nonterm);
    }
	depth--;
    //return true;
}


void Parser::relational_operator()
{
    std::string nonterm = "relational_operator";
    print(nonterm);
	depth++;
    auto s = next_token->symbol;
    if (s == LESS_THAN || s == EQUALS || s == GREATER_THAN) {
        match(s);
    }
    else {
        return syntax_error(nonterm);
    }
	depth--;
    //return true;
}


void Parser::simple_expression(PLType &type)
{
    std::string nonterm = "simple_expression";
    print(nonterm);
	depth++;
    std::set<Symbol> first{NOT, LEFT_PARENTHESIS, NUMERAL, TRUE_KEYWORD, FALSE_KEYWORD, IDENTIFIER};
    auto s = next_token->symbol;
    if (s == SUBTRACT) {
        match(s);
        TRY_WITH_ARG(term, type);
        if (!int_type(type)) {
            type_error("Cannot negate a non integer value");
            type = PLType::UNDEFINED;
        }
        TRY_WITH_ARG(simple_expression_end, type);
    }
    else if (sfind(first, s)) {
        TRY_WITH_ARG(term, type);
        TRY_WITH_ARG(simple_expression_end, type);
    }
    else {
        return syntax_error(nonterm);
    }
	depth--;
    //return true;
}


void Parser::simple_expression_end(PLType &lhs_type)
{
    std::string nonterm = "simple_expression_end";
    print(nonterm);
	depth++;
    auto s = next_token->symbol;
    if (s == ADD || s == SUBTRACT) {
        TRY(adding_operator);
        auto rhs_type = PLType::UNDEFINED;
        TRY_WITH_ARG(term, rhs_type);
        if (int_type(lhs_type) && int_type(rhs_type)) {
            lhs_type = PLType::INT_CONST;
        }
        else {
            type_error("Both operands of addition type operator must be integers");
            lhs_type = PLType::UNDEFINED;
        }
        depth--;
        TRY_WITH_ARG(simple_expression_end, lhs_type);
    }
    // epsilon production 
    else if (!check_follow(nonterm)) {
        depth--;
        return syntax_error(nonterm);
    }
    //return true;    
}


void Parser::adding_operator()
{
    std::string nonterm = "adding_operator";
    print(nonterm);
	depth++;
    auto s = next_token->symbol;
    if (s == ADD || s == SUBTRACT) {
        match(s);
    }
    else {
        return syntax_error(nonterm);
    }
	depth--;
    //return true;
}


void Parser::term(PLType &type)
{
    std::string nonterm = "term";
	print(nonterm);
	depth++;
    TRY_WITH_ARG(factor, type);
    TRY_WITH_ARG(term_end, type);
	depth--;
    //return true;
}


void Parser::term_end(PLType &lhs_type)
{
    std::string nonterm = "term_end";
    print(nonterm);
	depth++;
    auto s = next_token->symbol;
    if (s == MULTIPLY || s == DIVIDE || s == MODULO) {
        TRY(multiplying_operator);
        auto rhs_type = PLType::UNDEFINED;
        TRY_WITH_ARG(factor, rhs_type);
        if (int_type(lhs_type) && int_type(rhs_type)) {
            lhs_type = PLType::INT_CONST;
        }
        else {
            type_error("Both operands of multiplication type operators must be integers");
            lhs_type = PLType::UNDEFINED;
        }
        depth--;
        TRY_WITH_ARG(term_end, lhs_type);
    }
    // epsilon production 
    else if (!check_follow(nonterm)) {
        depth--;
        return syntax_error(nonterm);
    }
    //return true;
}


void Parser::multiplying_operator()
{
    std::string nonterm = "multiplying_operator";
    print(nonterm);
	depth++;
    auto s = next_token->symbol;
    if (s == MULTIPLY || s == DIVIDE || s ==  MODULO) {
        match(s);
    }
    else {
        return syntax_error(nonterm);
    }
	depth--;
    //return true;
}


void Parser::factor(PLType &type)
{
    std::string nonterm = "factor";
    print(nonterm);
	depth++;
    auto s = next_token->symbol;
    if (s == LEFT_PARENTHESIS) {
        match(s);
        TRY_WITH_ARG(expression, type);
        MATCH_AND_SYNC(RIGHT_PARENTHESIS, nonterm);
    }
    else if (s == IDENTIFIER) {
        TRY_WITH_ARG(variable_access, type);
    }
    else if (s == NUMERAL || s == TRUE_KEYWORD || s == FALSE_KEYWORD) {
        TRY_WITH_ARG(constant, type);
    }
    else if (s == NOT) {
        match(s);
        TRY_WITH_ARG(factor, type);
        if (!bool_type(type)) {
            type_error("Cannot take logical inverse of a non Boolean expression");
            type = PLType::UNDEFINED;
        }
        else {
            type = PLType::BOOL_CONST;

        }
    }
    else {
        return syntax_error(nonterm);
    }
	depth--;
    //return true;
}


void Parser::variable_access(PLType &type)
{
    std::string nonterm = "variable_access";
    print(nonterm);
	depth++;
    MATCH_AND_SYNC(IDENTIFIER, nonterm);
    auto id = matched_id.lexeme;
    type = get_type(id);
    TRY(variable_access_end);
	depth--;
    //return true;
}


void Parser::variable_access_end()
{
    std::string nonterm = "variable_access_end";
    print(nonterm);
	depth++;    
    auto s = next_token->symbol;
    if (s == LEFT_BRACKET) {
        TRY(indexed_selector);
    }
    // epsilon production 
    else if (!check_follow(nonterm)) {
        return syntax_error(nonterm);
    }
	depth--;
    //return true;
}


void Parser::indexed_selector()
{
    std::string nonterm = "indexed_selector";
    print(nonterm);
	depth++;
    MATCH_AND_SYNC(LEFT_BRACKET, nonterm);
    auto index_type = PLType::UNDEFINED;
    TRY_WITH_ARG(expression, index_type);
    if (!int_type(index_type)) {
        error_preamble();
        std::cout << "Array index must be integer type" << std::endl;
    }
    match(RIGHT_BRACKET);
	depth--;
    //return true;
}


void Parser::constant(PLType &type)
{
    std::string nonterm = "constant";
	print(nonterm);
    auto s = next_token->symbol;
    if (s == NUMERAL) {
        type = PLType::INT_CONST;
        match(s);
    }
    else if (s == TRUE_KEYWORD || s == FALSE_KEYWORD) {
        type = PLType::BOOL_CONST;
        match(s);
    }
    else if (s == IDENTIFIER) {

        auto id = next_token->lexeme;
        type = get_type(id);
        match(s);
    }
    else {
        return syntax_error(nonterm);
    }
    //return true;
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