#ifndef PL_PARSER_H
#define PL_PARSER_H

#include "token.h"
#include "block_table.h"
#include <vector>
#include <string>
#include <fstream>
#include <map>
#include <set>
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
    // If debug mode is enabled, output is written to command line (as well as to output string)
    Parser(bool debug=false);

    /*  Parse input and verify it can be derived from the PL language grammar.
        Returns the number of errors found, 0 on success. The contents of the resulting PLAM
        pseudo-code are stored in the output_program string
    */
    int verify_syntax(std::vector<Token> *input_tokens, std::string &output_program);

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

    // Next label returned by new_label
    int label_num;

    std::string output;
    bool debug_mode;

    // Produce labels to be used by assembler
    int new_label();

    // Write PLAM instruction and agruments to output string
    void emit(std::string instr, std::vector<int> args = {});

    // Move to next character in input. Throw eof_error if EOF reached
    void read_next();

    // Skip newline and comment tokens
    void skip_whitespace();

    // Check next token in input and advance if symbol matches s
    void match(Symbol s, std::string nonterminal);

    // Print error number and line on which error occurred
    void error_preamble();

    // Creates a new variables and catches any scope errors that might occur
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

    // Produce error message for type error
    void type_error(std::string err_mgs);

    // Produce error message and call synchronize to attempt to recover
    void syntax_error(std::string nonterm);

    // lookup next token in follow set of non-terminal. If not found, produce syntax error
    void check_follow(std::string non_terminal);

    // When in an error state, skip input tokens until one is found from which parsing can continue
    void synchronize(std::string non_terminal); 


    /*  Recursive procedures for nonterminals. 
    */

    void program();

    // Labels for storing size of vars, and address of first instruction in block
    void block(int var_label, int start_label);

    // Return length needed to store all variables - offset is used to set var displacements
    int definition_part(int offset);

    /*  Return length needed for variables defined - offset is updated according to size of 
        variables in subsequent calls
    */
    int definition(int &offset);

    void constant_definition();

    int variable_definition(int &offset);

    int variable_definition_type(PLType varlist_type, int &offset);

    // Return type of variable being defined
    PLType type_symbol();

    // var_list is filled with the identifiers of variables
    void variable_list(std::vector<std::string> &var_list);

    void variable_list_end(std::vector<std::string> &var_list);

    void procedure_definition();

    void statement_part();

    void statement();

    void empty_statement();

    void read_statement();

    // Vars filled with identifiers found in access list
    void variable_access_list(std::vector<std::string> &vars); 

    void variable_access_list_end(std::vector<std::string> &vars);

    void write_statement();

    // Store types of each expression in list in types
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

    // lhs_type is passed from expression to compare against type of following expressions
    PLType expression_end(PLType lhs_type);

    void primary_operator();

    PLType primary_expression();

    // lhs_type is passed from primary_expression to compare against type of following expressions
    PLType primary_expression_end(PLType lhs_type);

    void relational_operator();

    PLType simple_expression();

    // lhs_type is passed from simple_expression to compare against type of following expressions
    PLType simple_expression_end(PLType lhs_type);

    void adding_operator();

    PLType term();

    // lhs_type is passed from term confirm all parts are integers
    PLType term_end(PLType lhs_type);

    void multiplying_operator();

    PLType factor();

    // Return the identifier of the variable
    std::string variable_access();

    void variable_access_end();

    void indexed_selector();

    // Return type of constant (bool or int). Value of constant stored in value
    PLType constant(int &value);

    // Initialize first and follow sets
    void init_symbol_sets();    
};

#endif