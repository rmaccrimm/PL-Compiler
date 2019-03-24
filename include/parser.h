#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include <vector>
#include <string>
#include <map>
#include <set>
#include <stack>

// All of the valid types in PL
enum class PLType 
{
    INT_VAR,
    INT_CONST,
    BOOL_VAR,
    BOOL_CONST,
    PROCEDURE,
    UNDEFINED
};

/*  A block table Entry stores:
    - the type of the variable, defined above
    - the size of the variable (greater than 1 only for arrays)
*/
typedef std::pair<PLType, int> BlockData;

// A block is a set of names of identifiers, and the associated data
typedef std::map<std::string, BlockData> Block;


// Recursive descent parser which perform syntax, type and scope checking
class Parser
{
public:
    // Construct a parser to verify the given input represents a syntactically correct PL program
    Parser();

    /*  Parse input and verify it can be derived from the PL language grammar. The call stack is 
        printed as it runs, showing the structure of the parse tree. 
        
        When an error is encountered -- TBD
        Returns the number of errors found, 0 on success.
    */
    int verify_syntax(std::vector<Token> *input_tokens);

private:
    // Nonterminal follow sets
    std::map<std::string, std::set<Symbol>> follow;

    // The "lookahead" token for LL(1) parsing
    std::vector<Token>::iterator next_token;

    // The Block Table, for scope and type checking
    std::stack<Block> block_table;

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

    void error_preamble();

    // When none of the predict symbols are found, print error message and attempt to synchronize
    bool syntax_error(std::string non_terminal);

    void scope_error(std::string identifier, bool define=true);

    void type_error(Token t);

    bool check_follow(std::string non_terminal);

    // Check if identifier has already been defined in current scope
    bool in_scope(std::string id);

    // When in an error state, skip input symbols until one is found from which parsing can continue
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
    bool variable_definition_type(PLType type);
    bool type_symbol(PLType &type);
    bool variable_list(std::vector<std::string> &var_list);
    bool variable_list_end(std::vector<std::string> &var_list);
    bool procedure_definition();
    bool statement_part();
    bool statement();
    bool empty_statement();
    bool read_statement();
    bool variable_access_list();
    bool variable_access_list_end();
    bool write_statement();
    bool expression_list();
    bool expression_list_end();
    bool assignment_statement();
    bool procedure_statement();
    bool if_statement();
    bool do_statement();
    bool guarded_command_list();
    bool guarded_command_list_end();
    bool guarded_command();
    bool expression();
    bool expression_end();
    bool primary_operator();
    bool primary_expression();
    bool primary_expression_end();
    bool relational_operator();
    bool simple_expression();
    bool simple_expression_end();
    bool adding_operator();
    bool term();
    bool term_end();
    bool multiplying_operator();
    bool factor();
    bool variable_access();
    bool variable_access_end();
    bool indexed_selector();
    bool constant(PLType &type);

    // Initialize first and follow sets
    void init_symbol_sets();

    
};

#endif