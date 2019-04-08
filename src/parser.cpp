#include "parser.h"
#include "symbol.h"
#include <iostream>
#include <cassert>
#include <set>

#define PRINT 1

using std::cout;
using std::cerr;
using std::endl;

// Slightly simpler syntax for looking up an element of a set, return true if found
bool sfind(std::set<Symbol> set, Symbol s) { return set.find(s) != set.end(); }


Parser::Parser(bool debug):
    line{1}, num_errors{0}, label_num{1}, output{output}, debug_mode{debug}
{
    init_symbol_sets();
}


int Parser::verify_syntax(std::vector<Token> *input_tokens, std::string &output_prog)
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
    output_prog = output;
    return num_errors;
}


int Parser::new_label()
{
    return label_num++;
}


void Parser::emit(std::string instr, std::vector<int> args)
{
    output +=  instr + ' ';
    for (auto a: args) {
        output += std::to_string(a) + ' ';
    }
    output += '\n';

    if (debug_mode) {
        cout << instr << ' ';
        for (auto a: args) {
            cout << a << ' ';
        }
        cout << endl;        
    }
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
    std::cout << msg << std::endl;
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


void Parser::define_var(
    std::string id,
    PLType type,
    int size,
    int value,
    bool constant,
    bool array,
    int displacement, // number of words relative to start of block - starts at 3 
    int start_addr
    ) {
    BlockData b;
    b.type = type;
    b.size = size;
    b.value = value;
    b.constant = constant;
    b.array = array;
    b.displacement = displacement;
    b.start_addr = start_addr; 
    try {
        block_table.insert(id, b);
        // DEBUG
        // cout << "---\ndefine " << id << "\nsize: " << size << "\nvalue: " << value << "\noffset: "
        //      << displacement << endl;
    }
    catch (const scope_error &e) {
        error_preamble();
        cerr << e.what() << endl;
    }
}


void Parser::program()
{
    std::string nonterm = "program";
    int var_label = new_label();
    int start_label = new_label();
    emit("PROG", {var_label, start_label});
    block_table.push_new();
    block(var_label, start_label);
    block_table.pop();
    match(PERIOD, nonterm);
    emit("ENDPROG");
    // Matching an eof token would normally produce an error message, so handle in special case here
    if (next_token->symbol != END_OF_FILE) {
        num_errors++;
        std::cout << "Error " << num_errors << " on line " << line << ": Expected " 
                  << SYMBOL_STRINGS.at(END_OF_FILE) << ", found " 
                  << SYMBOL_STRINGS.at(next_token->symbol) << std::endl;
    }
}


void Parser::block(int var_label, int start_label)
{
    std::string nonterm = "block";
    match(BEGIN, nonterm);
    // Vars start at offset 3, following static link, dynamic link, and return address
    int var_len = definition_part(3);
    emit("DEFARG", {var_label, var_len});
    emit("DEFADDR", {start_label});
    statement_part();
    match(END, nonterm);
}


int Parser::definition_part(int offset)
{
    std::string nonterm = "definition_part";
    std::set<Symbol> first{CONST, INT, BOOL, PROC};
    auto s = next_token->symbol;
    if (sfind(first, s)) {
        // Definition will update offset
        int len = definition(offset);
        match(SEMICOLON, nonterm);        
        return len + definition_part(offset);
    }
    // epsilon production
    else {
        check_follow(nonterm);        
        return 0;
    }
}


int Parser::definition(int &offset)
{
    std::string nonterm = "definition";
    auto s = next_token->symbol;
    if (s == CONST) {
        // Constants don't get space allocated in activation record, so don't have displacement
        constant_definition();
        return 0;
    }
    else if (s == INT || s == BOOL) {
        return variable_definition(offset);
    }
    else if (s == PROC) {
        // Procs not included as a variable, sizes is handled by a label
        procedure_definition();
        return 0;
    }
    else {
        syntax_error(nonterm);
        return 0;
    }
}


void Parser::constant_definition()
{
    std::string nonterm = "constant_definition";
    match(CONST, nonterm);    
    match(IDENTIFIER, nonterm);
    auto id = matched_id.lexeme;
    match(EQUALS, nonterm);
    int value;
    auto type = constant(value);
    define_var(
        id,     // identifier
        type,   // type
        1,      // size
        value,  // value
        true,   // constant
        false,  // array
        0,      // displacement - doesn't apply to constants
        0       // start_addr, for procedures, doesn't apply to constants
    );
}


int Parser::variable_definition(int &offset)
{
	std::string nonterm = "variable_definition";
    auto type = type_symbol();
    return variable_definition_type(type, offset);
}


int Parser::variable_definition_type(PLType varlist_type, int &offset)
{
    std::string nonterm = "variable_definition_type";
    std::vector<std::string> vars;
    int size = 1;
    auto s = next_token->symbol;
    bool arr = false;
    if (s == IDENTIFIER) {
        variable_list(vars);
    }
    else if (s == ARRAY) { 
        arr = true;
        match(ARRAY, nonterm);
        variable_list(vars);
        match(LEFT_BRACKET, nonterm);
        auto arr_size_type = constant(size);
        if (!equals(arr_size_type, PLType::INTEGER)) {
            type_error("Array bounds must be of type integer");
        }
        match(RIGHT_BRACKET, nonterm);
    }
    else {
        syntax_error(nonterm);
    }
    int len = 0;
    for (auto &id: vars) {
        define_var(
            id,             // id
            varlist_type,   // type
            size,           // size
            0,              // value
            false,          // constant
            arr,            // array
            offset,         // displacement
            0               // start_addr - for procedures, doesn't apply to vars 
        );
        offset += size;
        len += size;
    }
    return len;
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

    // Label for the start of the procedure
    int proc_label = new_label();
    // Label for the size of the variables in the procedure
    int var_label = new_label();
    // Label for the first instruction address of the procedure
    int start_label = new_label();
    define_var(
        id,                 // identifier
        PLType::PROCEDURE,  // type
        0,                  // size
        0,                  // value
        false,              // constant
        false,              // array
        0,                  // displacement - doesn't apply to procedures
        start_label         // start_addr 
    );

    block_table.push_new();
    emit("DEFADDR", {proc_label});
    emit("PROC", {var_label, start_label});
    block(var_label, start_label);
    emit("ENDPROC");
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
    emit("READ", {static_cast<int>(vars.size())});
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
            // Error messages already issued in variable_access            
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
    emit("WRITE", {static_cast<int>(types.size())});
    for (auto t: types) {
        if (equals(t, PLType::PROCEDURE)) {
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
    emit("ASSIGN", {static_cast<int>(expr_types.size())});
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
                else if ((data.type == PLType::PROCEDURE) || (e_type == PLType::PROCEDURE)) {
                    type_error("Procedure type cannot be used in assignment statement");
                }
                else if (!equals(data.type, e_type)) {
                    type_error("Mismatch between types of LHS and RHS of assignment statement");
                }
            }
            catch (const scope_error &e) {
                // These errors will have already been issued in variable_access
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
    try {
        BlockData &data = block_table.find(id);        
        if (!equals(data.type, PLType::PROCEDURE)) {
            type_error("Cannot call a non procedure type");
        }
        emit("CALL", {block_table.curr_level - data.level, data.start_addr});
    }
    catch (const scope_error &e) {
        error_preamble();
        cerr << e.what() << endl;
    }
}


void Parser::if_statement()
{
	std::string nonterm = "if_statement";
    int lstart = new_label(), ldone = new_label();
    match(IF, nonterm);
    guarded_command_list(lstart, ldone);
    emit("DEFADDR", {lstart});
    emit("FI", {line});
    emit("DEFADDR", {ldone});
    match(FI, nonterm);
}


void Parser::do_statement()
{
	std::string nonterm = "do_statement";
    int lstart = new_label(), lloop = new_label();
    match(DO, nonterm);
    emit("DEFADDR", {lloop});
    guarded_command_list(lstart, lloop);
    emit("DEFADDR", {lstart});
    match(OD, nonterm);
}


void Parser::guarded_command_list(int &start_label, int &goto_label)
{
	std::string nonterm = "guarded_command_list";
    guarded_command(start_label, goto_label);
    guarded_command_list_end(start_label, goto_label);
}


void Parser::guarded_command_list_end(int &start_label, int &goto_label)
{
    std::string nonterm = "guarded_command_list_end";
    auto s = next_token->symbol;
    if (s == DOUBLE_BRACKET) {
        match(s, nonterm);
        guarded_command(start_label, goto_label);
        guarded_command_list_end(start_label, goto_label);
    }
    // epsilon production 
    else {
		check_follow(nonterm);
	}
}


void Parser::guarded_command(int &start_label, int &goto_label)
{
    std::string nonterm = "guarded_command";
    emit("DEFADDR", {start_label});
    auto guard_type = expression();
    start_label = new_label();
    emit("ARROW", {start_label});
    if (!equals(guard_type, PLType::BOOLEAN)) {
        type_error("Guarded command must evaluate to Boolean type");
    }
    match(RIGHT_ARROW, nonterm);
    statement_part();
    emit("BAR", {goto_label});
}


PLType Parser::expression()
{
    std::string nonterm = "expression";
    auto lhs_type = primary_expression();
    return expression_end(lhs_type);
}


PLType Parser::expression_end(PLType lhs_type)
{
    std::string nonterm = "expression_end";
    auto s = next_token->symbol;
    if (s == AND || s == OR) {
        emit((s == AND ? "AND" : "OR"));
        primary_operator();
        auto rhs_type = expression();
        if (!equals(lhs_type, PLType::BOOLEAN) || !equals(rhs_type, PLType::BOOLEAN)) {
            type_error("Both operands for logical operator must be Boolean");
            return PLType::UNDEFINED;
        }
        return lhs_type;
    }
    // epsilon production 
    else {
		check_follow(nonterm);
        return lhs_type;
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
        switch (s) {
            case LESS_THAN: emit("LESS"); break;
            case GREATER_THAN: emit("GREATER"); break;
            case EQUALS: emit("EQUAL"); break;
        }
        if (!(equals(lhs_type, PLType::INTEGER) && equals(PLType::INTEGER, rhs_type))) {
            type_error("Both operands of a comparison must be integers");
            return PLType::UNDEFINED;
        }
        return PLType::BOOLEAN;
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
        emit("MINUS");
        if (!equals(lhs_type, PLType::INTEGER)) {
            type_error("Cannot negate a non integer value");
            lhs_type = PLType::UNDEFINED;
        }
        return simple_expression_end(lhs_type);
    }
    else if (sfind(first, s)) {
        auto lhs_type = term();
        return simple_expression_end(lhs_type);
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
        emit((s == ADD ? "ADD" : "SUBTRACT"));
        return simple_expression_end(lhs_type);
    }
    // epsilon production 
    else {
        check_follow(nonterm); 
        return lhs_type;
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
        switch (s) {
            case MULTIPLY: emit("MULTIPLY"); break;
            case DIVIDE: emit("DIVIDE"); break;
            case MODULO: emit("MODULO"); break;
        }
        if (!equals(lhs_type, PLType::INTEGER) || !equals(rhs_type, PLType::INTEGER)) {
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
        auto id = next_token->lexeme;
        BlockData data;
        try {
            data = block_table.find(id);
        }
        catch (const scope_error &e) {
            match(IDENTIFIER, nonterm); // Need to get rid of id from input to continue
            error_preamble();
            cerr << e.what() << endl;
            return PLType::UNDEFINED;
        }
        if (data.constant) {
            int value;
            auto type = constant(value);
            emit("CONSTANT", {value});
            return type;    
        }
        else {
            id = variable_access();
            data = block_table.find(id);
            emit("VALUE");
            return data.type;
        }
    }
    else if (s == NUMERAL || s == TRUE_KEYWORD || s == FALSE_KEYWORD) {
        int value;
        auto type = constant(value);
        emit("CONSTANT", {value});
        return type;
    }
    else if (s == NOT) {
        match(s, nonterm);
        auto type = factor();
        emit("NOT");
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
    try {
        auto data = block_table.find(id);
        emit("VARIABLE", {block_table.curr_level - data.level, data.displacement});
    }
    catch (const scope_error &e) {
        error_preamble();
        cerr << e.what() << endl;
    }
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
    // Last matched id before the [] was the array identifier
    auto id = matched_id.lexeme;
    match(LEFT_BRACKET, nonterm);
    auto ind_type = expression();
    try {
        BlockData data = block_table.find(id);
        emit("INDEX", {data.size, line});
    }
    catch (const scope_error &e) {
        // This error message will have been issued already by variable_access
    }
    
    if (!equals(ind_type, PLType::INTEGER)) {
        error_preamble();
        std::cout << "Array index must be integer type" << std::endl;
    }
    match(RIGHT_BRACKET, nonterm);
}


PLType Parser::constant(int &value)
{
    std::string nonterm = "constant";
    auto s = next_token->symbol;
    if (s == NUMERAL) {
        value = next_token->value;
        match(s, nonterm);
        return PLType::INTEGER;
    }
    else if (s == TRUE_KEYWORD || s == FALSE_KEYWORD) {
        value = (s == TRUE_KEYWORD ? 1 : 0);
        match(s, nonterm);
        return PLType::BOOLEAN;
    }
    else if (s == IDENTIFIER) {
        auto id = next_token->lexeme;
        match(s, nonterm);
        try {
            BlockData data = block_table.find(id);
            if (!data.constant) {
                error_preamble();
                cerr << "Found non-constant value where constant expected" << endl;
                return PLType::UNDEFINED;
            }
            value = data.value;
            return data.type;
        }
        catch (const scope_error &e) {
            error_preamble();
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