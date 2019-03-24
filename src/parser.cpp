#include "parser.h"
#include <iostream>
#include <cassert>
#include <set>

#define PRINT 0
#define INDENT 1

/*  Two macros are used to reduce some of the typing in the nonterminal functions.

    All nonterminal functions that call another nonterminal continue only if true is returned, 
    returning false otherwise.
*/
#define TRY(func) if(!func()) { return false; }

/*  When matching, continue only if successful, otherwise attempt to synchronize and skip the
    rest of the current noterminal function
*/
#define MATCH_AND_SYNC(symbol, nonterminal) if (!match(symbol)) { return synchronize(nonterminal); }

// Slightly simpler syntax for looking up an element of a set 
bool sfind(std::set<Symbol> set, Symbol s) { return set.find(s) != set.end(); }


Parser::Parser(): line{1}, depth{0}, num_errors{0}
{
    init_symbol_sets();
}


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
    next_token = input_tokens->begin();
    skip_whitespace();
    program();
    return num_errors;
}


bool Parser::read_next()
{
    if (next_token->symbol == END_OF_FILE) {
        num_errors++;
        std::cerr << "Fatal Error: Reached end of file while parsing" << std::endl;
        return false;
    }
    next_token++;
    skip_whitespace();
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


bool Parser::match(Symbol s)
{
    if (s != next_token->symbol) {
        num_errors++;
        std::cerr << "Error " << num_errors << " on line " << line << ": Expected " 
                  << SYMBOL_STRINGS.at(s) << ", found " << SYMBOL_STRINGS.at(next_token->symbol) 
                  << std::endl;
        return false;
    }
    return read_next();
}


bool Parser::check_follow(std::string non_terminal)
{
    return sfind(follow[non_terminal], next_token->symbol);
}


void Parser::error_preamble()
{
    num_errors++;
    std::cerr << "Error " << num_errors << " on line " << line << ": ";
}


bool Parser::syntax_error(std::string non_terminal)
{
    error_preamble();
    std::cerr << "Unexpected " << SYMBOL_STRINGS.at(next_token->symbol) << " symbol" << std::endl; 
    return synchronize(non_terminal);
}


void Parser::scope_error(std::string id)
{
    error_preamble();
    std::cerr << "Identifier " << id << " already defined" << std::endl;
}


bool Parser::synchronize(std::string non_terminal)
{
    auto sync = follow[non_terminal];
    while (!sfind(sync, next_token->symbol)) {
        if (!read_next()) {
            return false;
        }
    }
    std::cerr << "-- Resuming from " << SYMBOL_STRINGS.at(next_token->symbol) << " on line " << line
              << std::endl; 
    return true;
}


bool Parser::program()
{
    std::string nonterm = "program";
    print(nonterm);

    depth++;
    block_table.push(Block());
    TRY(block)
    depth--;
    block_table.pop();

    MATCH_AND_SYNC(PERIOD, nonterm);
    if (next_token->symbol != END_OF_FILE) {
        num_errors++;
        std::cerr << "Error " << num_errors << " on line " << line << ": Expected " 
                  << SYMBOL_STRINGS.at(END_OF_FILE) << ", found " 
                  << SYMBOL_STRINGS.at(next_token->symbol) << std::endl;
    }
    return true;
}


bool Parser::block()
{
    std::string nonterm = "block";
	print(nonterm);
    MATCH_AND_SYNC(BEGIN, nonterm)
    depth++;
    TRY(definition_part)
    TRY(statement_part)
    depth--;
    MATCH_AND_SYNC(END, nonterm)
    return true;
}


bool Parser::definition_part()
{
    std::string nonterm = "definition_part";
	print(nonterm);
    depth++;
    std::set<Symbol> first{CONST, INT, BOOL, PROC};
    auto s = next_token->symbol;
    if (sfind(first, s)) {
        TRY(definition)
        MATCH_AND_SYNC(SEMICOLON, nonterm)        
        depth--;
        TRY(definition_part)
    }
    // epsilon production
    else if (!check_follow(nonterm)) {
        depth--;
        return synchronize(nonterm);
    }
    return true;
}


bool Parser::definition()
{
    std::string nonterm = "definition";
	print(nonterm);
    depth++;
    auto s = next_token->symbol;
    if (s == CONST) {
        TRY(constant_definition)
    }
    else if (s == INT || s == BOOL) {
        TRY(variable_definition)
    }
    else if (s == PROC) {
        TRY(procedure_definition)
    }
    else {
        return syntax_error(nonterm);
    }
    depth--;
    return true;
}


bool Parser::check_scope(std::string id) 
{
    Block& current_block = block_table.top();
    if (current_block.find(id) != current_block.end()) {
        scope_error(id);
        return false;
    }
    return true;
}


bool Parser::constant_definition()
{
    std::string nonterm = "constant_definition";
	print(nonterm);

    depth++;
    MATCH_AND_SYNC(CONST, nonterm)    
    MATCH_AND_SYNC(IDENTIFIER, nonterm);
    MATCH_AND_SYNC(EQUALS, nonterm);
    TRY(constant)
    depth--;

    /*  We have now succesfully matched the whole sequence, const ID = CONST, so know what 
        preceding tokens are
    */
    auto id_tok = *(next_token - 3);
    auto id = id_tok.lexeme;

    if (check_scope(id)) {
        // Determine type and insert
        auto s = (next_token - 1)->symbol;
        auto type = (s == NUMERAL) ? PLType::INT_CONST : PLType::BOOL_CONST;
        block_table.top()[id] = {type, 1};
    }
    return true;
}


bool Parser::variable_definition()
{
	print("variable_definition");
    depth++;
    PLType type;
    if (!type_symbol(type)) {
        return false;
    }
    if (!variable_definition_type(type)) {
        return false;
    }
    depth--;
    return true;
}


bool Parser::variable_definition_type(PLType type)
{
    assert(type == PLType::BOOL_VAR || type == PLType::INT_VAR);

    std::string nonterm = "variable_definition_type";
    print(nonterm);
	depth++;

    std::vector<std::string> vars;
    int size = 1;

    auto s = next_token->symbol;
    if (s == IDENTIFIER) {
        if (!variable_list(vars)) {
            return false;
        }
    }
    else if (s == ARRAY) { 
        MATCH_AND_SYNC(ARRAY, nonterm)
        if (!variable_list(vars)) {
            return false;
        }
        MATCH_AND_SYNC(LEFT_BRACKET, nonterm)
        TRY(constant)
        MATCH_AND_SYNC(RIGHT_BRACKET, nonterm)
        // After matching sequence look back 2 tokens to get size of the array(s)
        size = (next_token - 2)->value;
    }
    else {
        return syntax_error(nonterm);
    }
	depth--;
    for (auto &id: vars) {
        if (check_scope(id)) {
            block_table.top()[id] = {type, size};
        }
    }
    return true;
}


bool Parser::type_symbol(PLType &type)
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
    return true;
}


bool Parser::variable_list(std::vector<std::string> &var_list)
{
    std::string nonterm = "variable_list";
    print(nonterm);
	depth++;
    MATCH_AND_SYNC(IDENTIFIER, nonterm);
    var_list.push_back((next_token-1)->lexeme);
    if (!variable_list_end(var_list)) {
        return false;
    }
	depth--;
    return true;
}


bool Parser::variable_list_end(std::vector<std::string> &var_list)
{
    std::string nonterm = "variable_list_end";
    print(nonterm);
	depth++;
    auto s = next_token->symbol;
    if (s == COMMA) {
        MATCH_AND_SYNC(s, nonterm);
        MATCH_AND_SYNC(IDENTIFIER, nonterm);
        var_list.push_back((next_token-1)->lexeme);
        if (!variable_list_end(var_list)) {
            return false;
        }
    }
    // epsilon production
    else if (!check_follow(nonterm)) {
        return syntax_error(nonterm);
    }
    depth--;
    return true;
}


bool Parser::procedure_definition()
{
    std::string nonterm = "procedure_definition";
	print(nonterm);
	depth++;
    MATCH_AND_SYNC(PROC, nonterm)    
    MATCH_AND_SYNC(IDENTIFIER, nonterm);

    auto id = (next_token - 1)->lexeme;
    if (check_scope(id)) {
        block_table.top()[id] = {PLType::PROCEDURE, 1};
    }
    /*  Push a new block onto the stack that is initially identical to the previous block. This 
        allows the procedure to access variables in the outer scope, but does not allow later 
        statements to use variables defined in the procedure 
    */
    Block proc_block = block_table.top();
    block_table.push(proc_block);
    bool success = block();
    block_table.pop();
	depth--;
    if (!success) {
        return false;
    }
    return true;
} 


bool Parser::statement_part()
{
    std::string nonterm = "statement_part";
	print(nonterm);
	depth++;
    std::set<Symbol> first{SKIP, READ, WRITE, IDENTIFIER, CALL, IF, DO};
    auto s = next_token->symbol;
    if (sfind(first, s)) {
        TRY(statement)
        MATCH_AND_SYNC(SEMICOLON, nonterm)        
        depth--;
        TRY(statement_part)
    }
    // epsilon production
    else if (!check_follow(nonterm)) {
        depth--;
        return syntax_error(nonterm);
    }
    return true;
}


bool Parser::statement()
{
    std::string nonterm = "statement";
	print(nonterm);
	depth++;
    auto s = next_token->symbol;
    if (s == SKIP) {
        TRY(empty_statement)
    }
    else if (s == READ) {
        TRY(read_statement)
    }
    else if (s == WRITE) {
        TRY(write_statement)
    }
    else if (s == IDENTIFIER) {
        TRY(assignment_statement)
    }
    else if (s == CALL) {
        TRY(procedure_statement)
    }
    else if (s == IF) {
        TRY(if_statement)
    }
    else if (s == DO) {
        TRY(do_statement)
    }
    else {
        syntax_error(nonterm);
    }
	depth--;
    return true;
}


bool Parser::empty_statement()
{
    std::string nonterm = "empty_statement";
	print(nonterm);
	depth++;
    MATCH_AND_SYNC(SKIP, nonterm)
	depth--;
    return true;
}


bool Parser::read_statement()
{
    std::string nonterm = "read_statement";
	print(nonterm);
	depth++;
    MATCH_AND_SYNC(READ, nonterm)
    TRY(variable_access_list)
	depth--;
    return true;
}


bool Parser::variable_access_list()
{
	print("variable_access_list");
	depth++;
    TRY(variable_access)
    TRY(variable_access_list_end)
	depth--;
    return true;
}


bool Parser::variable_access_list_end()
{
    std::string nonterm = "variable_access_list_end";
    print(nonterm);
	depth++;
    std::set<Symbol> first{SEMICOLON, ASSIGN};
    auto s = next_token->symbol;
    if (s == COMMA) {
        MATCH_AND_SYNC(COMMA, nonterm);
        TRY(variable_access)
        TRY(variable_access_list_end)
    }
    // epsilon production 
    else if (!check_follow(nonterm)) {
        return syntax_error(nonterm);
    }
    depth--;
    return true;
}


bool Parser::write_statement()
{
    std::string nonterm = "write_statement";
	print(nonterm);
	depth++;
    MATCH_AND_SYNC(WRITE, nonterm)
    TRY(expression_list)
	depth--;
    return true;
}


bool Parser::expression_list()
{
	print("expression_list");
	depth++;
    TRY(expression)
    TRY(expression_list_end)
	depth--;
    return true;
}


bool Parser::expression_list_end()
{
    std::string nonterm = "expression_list_end";
    print(nonterm);
	depth++;
    auto s = next_token->symbol;
    if (s == COMMA) {
        MATCH_AND_SYNC(s, nonterm);
        TRY(expression)
        TRY(expression_list_end)
    }
    // epsilon production 
    else if (!check_follow(nonterm)) {
        return syntax_error(nonterm);
    }
    depth--;
    return true;
}


bool Parser::assignment_statement()
{
    std::string nonterm = "assignment_statement";
    print(nonterm);
	depth++;
    TRY(variable_access_list)
    MATCH_AND_SYNC(ASSIGN, nonterm);
    TRY(expression_list)
	depth--;
    return true;
}

bool Parser::procedure_statement()
{
    std::string nonterm = "procedure_statement";
	print(nonterm);
	depth++;
    MATCH_AND_SYNC(CALL, nonterm);
    MATCH_AND_SYNC(IDENTIFIER, nonterm);
	depth--;
    return true;
}


bool Parser::if_statement()
{
	std::string nonterm = "if_statement";
    print(nonterm);
	depth++;
    MATCH_AND_SYNC(IF, nonterm);
    TRY(guarded_command_list)
    MATCH_AND_SYNC(FI, nonterm);
	depth--;
    return true;
}


bool Parser::do_statement()
{
	std::string nonterm = "do_statement";
    print(nonterm);
	depth++;
    MATCH_AND_SYNC(DO, nonterm);
    TRY(guarded_command_list)
    MATCH_AND_SYNC(OD, nonterm);
	depth--;
    return true;
}


bool Parser::guarded_command_list()
{
	print("guarded_command_list");
	depth++;
    TRY(guarded_command)
    TRY(guarded_command_list_end)
	depth--;
    return true;
}


bool Parser::guarded_command_list_end()
{
    std::string nonterm = "guarded_command_list_end";
    print(nonterm);
	depth++;
    auto s = next_token->symbol;
    if (s == DOUBLE_BRACKET) {
        MATCH_AND_SYNC(s, nonterm);
        TRY(guarded_command)
        depth--;
        TRY(guarded_command_list_end)
    }
    // epsilon production 
    else if (!check_follow(nonterm)) {
        depth--;
        return syntax_error(nonterm);
    }
    return true;
}


bool Parser::guarded_command()
{
    std::string nonterm = "guarded_command";
    print(nonterm);
	depth++;
    TRY(expression)
    MATCH_AND_SYNC(RIGHT_ARROW, nonterm);
    TRY(statement_part)
	depth--;
    return true;
}


bool Parser::expression()
{
	print("expression");
	depth++;
    TRY(primary_expression)
    TRY(expression_end)
	depth--;
    return true;
}


bool Parser::expression_end()
{
    std::string nonterm = "expression_end";
    print(nonterm);
	depth++;
    auto s = next_token->symbol;
    if (s == AND || s == OR) {
        TRY(primary_operator)
        TRY(primary_expression)
        TRY(expression_end)
    }
    // epsilon production 
    else if (!check_follow(nonterm)) {
        return syntax_error(nonterm);
    }
    depth--;
    return true;
}


bool Parser::primary_operator()
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
    return true;
}


bool Parser::primary_expression()
{
	print("primary_expression");
	depth++;
    TRY(simple_expression)
    TRY(primary_expression_end)
	depth--;
    return true;
}


bool Parser::primary_expression_end()
{
    std::string nonterm = "primary_expression_end"; 
    print(nonterm);
	depth++;
    auto s = next_token->symbol;
    if (s == LESS_THAN || s == GREATER_THAN || s == EQUALS) {
        TRY(relational_operator)
        TRY(simple_expression)
    }
    // epsilon production 
    else if (!check_follow(nonterm)) {
        return syntax_error(nonterm);
    }
	depth--;
    return true;
}


bool Parser::relational_operator()
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
    return true;
}


bool Parser::simple_expression()
{
    std::string nonterm = "simple_expression";
    print(nonterm);
	depth++;
    std::set<Symbol> first{NOT, LEFT_PARENTHESIS, NUMERAL, TRUE_KEYWORD, FALSE_KEYWORD, IDENTIFIER};
    auto s = next_token->symbol;
    if (s == SUBTRACT) {
        match(s);
        TRY(term)
        TRY(simple_expression_end)
    }
    else if (sfind(first, s)) {
        TRY(term)
        TRY(simple_expression_end)
    }
    else {
        return syntax_error(nonterm);
    }
	depth--;
    return true;
}


bool Parser::simple_expression_end()
{
    std::string nonterm = "simple_expression_end";
    print(nonterm);
	depth++;
    auto s = next_token->symbol;
    if (s == ADD || s == SUBTRACT) {
        TRY(adding_operator)
        TRY(term)
        depth--;
        TRY(simple_expression_end)
    }
    // epsilon production 
    else if (!check_follow(nonterm)) {
        depth--;
        return syntax_error(nonterm);
    }
    return true;    
}


bool Parser::adding_operator()
{
    std::string nonterm = "adding_operator";
    print(nonterm);
	depth++;
    auto s = next_token->symbol;
    if (s == ADD || s == OR) {
        match(s);
    }
    else {
        return syntax_error(nonterm);
    }
	depth--;
    return true;
}


bool Parser::term()
{
    std::string nonterm = "term";
	print(nonterm);
	depth++;
    TRY(factor)
    TRY(term_end)
	depth--;
    return true;
}


bool Parser::term_end()
{
    std::string nonterm = "term_end";
    print(nonterm);
	depth++;
    auto s = next_token->symbol;
    if (s == MULTIPLY || s == DIVIDE || s == MODULO) {
        TRY(multiplying_operator)
        TRY(factor)
        depth--;
        TRY(term_end)
    }
    // epsilon production 
    else if (!check_follow(nonterm)) {
        depth--;
        return syntax_error(nonterm);
    }
    return true;
}


bool Parser::multiplying_operator()
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
    return true;
}


bool Parser::factor()
{
    std::string nonterm = "factor";
    print(nonterm);
	depth++;
    // LL(1) issue - does not correctly decide between VarAccess and Constant
    auto s = next_token->symbol;
    if (s == LEFT_PARENTHESIS) {
        match(s);
        TRY(expression)
        MATCH_AND_SYNC(RIGHT_PARENTHESIS, nonterm);
    }
    else if (s == IDENTIFIER) {
        TRY(variable_access)
    }
    else if (s == NUMERAL || s == TRUE_KEYWORD || s == FALSE_KEYWORD || s == IDENTIFIER) {
        TRY(constant)
    }
    else if (s == NOT) {
        match(s);
        TRY(factor)
    }
    else {
        return syntax_error(nonterm);
    }
	depth--;
    return true;
}


bool Parser::variable_access()
{
    std::string nonterm = "variable_access";
    print(nonterm);
	depth++;
    MATCH_AND_SYNC(IDENTIFIER, nonterm)
    TRY(variable_access_end)
	depth--;
    return true;
}


bool Parser::variable_access_end()
{
    std::string nonterm = "variable_access_end";
    print(nonterm);
	depth++;    
    auto s = next_token->symbol;
    if (s == LEFT_BRACKET) {
        MATCH_AND_SYNC(s, nonterm)
        TRY(expression)
        MATCH_AND_SYNC(RIGHT_BRACKET, nonterm)
    }
    // epsilon production 
    else if (!check_follow(nonterm)) {
        return syntax_error(nonterm);
    }
	depth--;
    return true;
}


bool Parser::indexed_selector()
{
    std::string nonterm = "indexed_selector";
    print(nonterm);
	depth++;
    MATCH_AND_SYNC(LEFT_BRACKET, nonterm);
    TRY(expression)
    match(RIGHT_BRACKET);
	depth--;
    return true;
}


bool Parser::constant()
{
    std::string nonterm = "constant";
	print(nonterm);
    auto s = next_token->symbol;
    if (s == NUMERAL || s == TRUE_KEYWORD || s == FALSE_KEYWORD || s == IDENTIFIER) {
        match(s);
    }
    else {
        return syntax_error(nonterm);
    }
    return true;
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