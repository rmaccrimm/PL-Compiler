#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "block_table.h"
#include <vector>
#include <string>
#include <map>
#include <set>
#include <stack>
#include <stdexcept>


class eof_error: public std::runtime_error
{
public:
    eof_error(): std::runtime_error("Reached end of file while parsing") {}
};

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

    int num_errors;
    int line;
    int label_num;


    // Code generations functions
    int new_label();
    void emit(std::string instr, std::vector<int> args = {});

    // Move to next character in input. Throw eof_error if EOF reached
    void read_next();

    // Skip newline and comment tokens
    void skip_whitespace();

    // Check next token in input and advance if symbol matches s
    void match(Symbol s, std::string nonterminal);

    // Print error number and line on which error occurred
    void error_preamble();

    // Catch potential scope_errors 
    void define_var(
        std::string id,
        PLType type,
        int size,
        int value,
        bool constant,
        bool array,
        int displacement,
        int start_addr
    );

    // Types of error messages
    void type_error(std::string err_mgs);

    void syntax_error(std::string nonterm);

    // lookup next token in follow set of non-terminal  
    void check_follow(std::string non_terminal);

    // When in an error state, skip input tokens until one is found from which parsing can continue
    void synchronize(std::string non_terminal); 

    /*  Recursive procedures for nonterminals. True is returned if the nonterminal was either parsed
        correctly, or was able to recover from an error. The only situation in which false is 
        returned is when the end of file was reach prematurely, in which case tha caller returns
        false as well.

        Reference parameters in functions are used to return additional values
    */
    void program();
    // Labels for storing size of vars, and first instruction address
    void block(int var_label, int start_label);
    // Return length needed to store all variables - not sure how procs work yet
    int definition_part(int first_var_start);
    // Return length needed for vars defined 
    int definition(int &offset);

    void constant_definition();
    int variable_definition(int &offset);

    int variable_definition_type(PLType varlist_type, int &var_start);
    PLType type_symbol();
    void variable_list(std::vector<std::string> &var_list);
    void variable_list_end(std::vector<std::string> &var_list);
    void procedure_definition();
    void statement_part();
    void statement();
    void empty_statement();
    void read_statement();
    void variable_access_list(std::vector<std::string> &vars); 
    void variable_access_list_end(std::vector<std::string> &vars);
    void write_statement();
    void expression_list(std::vector<PLType> &types);
    void expression_list_end(std::vector<PLType> &types);
    void assignment_statement();
    void procedure_statement();
    void if_statement();
    void do_statement();
    void guarded_command_list(int &start_label, int &goto_label);
    void guarded_command_list_end(int &start_label, int &goto_label);
    void guarded_command(int &start_label, int &goto_label);
    PLType expression();
    PLType expression_end(PLType lhs_type);
    void primary_operator();
    PLType primary_expression();
    PLType primary_expression_end(PLType lhs_type);
    void relational_operator();
    PLType simple_expression();
    PLType simple_expression_end(PLType lhs_type);
    void adding_operator();
    PLType term();
    PLType term_end(PLType lhs_type);
    void multiplying_operator();
    PLType factor();
    std::string variable_access();
    void variable_access_end();
    void indexed_selector();
    PLType constant(int &value);
    // PLType constant();

    // Initialize first and follow sets
    void init_symbol_sets();    
};

#endif