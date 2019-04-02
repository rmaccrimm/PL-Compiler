#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "block_table.h"
#include <vector>
#include <string>
#include <map>
#include <set>
#include <stack>



/*  A block table Entry stores:
    - the type of the variable, defined above
    - the size of the variable (greater than 1 only for arrays)
*/
// typedef std::pair<PLType, int> BlockData;

// A block is a set of names of identifiers, and the associated data
// typedef std::map<std::string, BlockData> Block;


// Recursive descent parser which perform syntax, type and scope checking
class Parser
{
public:
    // Construct a parser to verify the given input represents a syntactically correct PL program
    Parser();

    /*  Parse input and verify it can be derived from the PL language grammar.
        Returns the number of errors found, 0 on success.
    */
    int verify_syntax(std::vector<Token> *input_tokens);

private:
    // Nonterminal follow sets
    std::map<std::string, std::set<Symbol>> follow;

    // The "lookahead" token for LL(1) parsing
    std::vector<Token>::iterator next_token;

    // The Block Table, for scope and type checking
    BlockTable block_table;

    // Always save the last identifier token matched
    Token matched_id;

    // Used to track height of call stack for printing function calls with indentation
    int depth;
    int num_errors;
    int line;

    // Move to next character in input. Return false if EOF read
    bool read_next();

    // Skip newline and comment tokens
    void skip_whitespace();

    // Check next token in input and advance if symbol matches s
    bool match(Symbol s);

    // Print error number and line on which error occurred
    void error_preamble();

    // Catch potential scope_errors 
    void define_var(std::string id, PLType type, int size, bool constant);
    PLType get_type(std::string id);

    // Types of error messages
    bool syntax_error(std::string non_terminal);
    void type_error(std::string err_mgs);

    // lookup next token in follow set of non-terminal  
    bool check_follow(std::string non_terminal);

    // When in an error state, skip input tokens until one is found from which parsing can continue
    bool synchronize(std::string non_terminal); 

    // Used to print the function call stack 
    void print(std::string msg);

    /*  Recursive procedures for nonterminals. True is returned if the nonterminal was either parsed
        correctly, or was able to recover from an error. The only situation in which false is 
        returned is when the end of file was reach prematurely, in which case tha caller returns
        false as well.

        Reference parameters in functions are used to return additional values
    */
    bool program();
    bool block();
    bool definition_part();
    bool definition();
    bool constant_definition();
    bool variable_definition();
    bool variable_definition_type(PLType varlist_type);
    bool type_symbol(PLType &type);
    bool variable_list(std::vector<std::string> &var_list);
    bool variable_list_end(std::vector<std::string> &var_list);
    bool procedure_definition();
    bool statement_part();
    bool statement();
    bool empty_statement();
    bool read_statement();
    bool variable_access_list(std::vector<PLType> &types);
    bool variable_access_list_end(std::vector<PLType> &types);
    bool write_statement();
    bool expression_list(std::vector<PLType> &types);
    bool expression_list_end(std::vector<PLType> &types);
    bool assignment_statement();
    bool procedure_statement();
    bool if_statement();
    bool do_statement();
    bool guarded_command_list();
    bool guarded_command_list_end();
    bool guarded_command();
    bool expression(PLType &type);
    bool expression_end(PLType &lhs_type);
    bool primary_operator();
    bool primary_expression(PLType &type);
    bool primary_expression_end(PLType &lhs_type);
    bool relational_operator();
    bool simple_expression(PLType &type);
    bool simple_expression_end(PLType &lsh_type);
    bool adding_operator();
    bool term(PLType &type);
    bool term_end(PLType &lhs_type);
    bool multiplying_operator();
    bool factor(PLType &type);
    bool variable_access(PLType &type);
    bool variable_access_end();
    bool indexed_selector();
    bool constant(PLType &type);

    // Initialize first and follow sets
    void init_symbol_sets();    
};

#endif