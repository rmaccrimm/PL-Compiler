#ifndef PL_COMPILER_H
#define PL_COMPILER_H

#include <fstream>
#include <vector>
#include "scanner.h"
#include "symbol_table.h"

/*  An administration class that manages each of the separate compilation stages. Responsible for
    creating storing, and calling methods for each of the seperate classes. Currnetly only  
    scanning is implemented
*/
class Compiler
{
public:
    // Constructor takes the filepath of the PL source file to be compiled
    Compiler(std::ifstream &input_file);

    /* Two versions of scan which perform tokenization on the input file. Return false if errors 
       are detected, true otherwise. When an error is detected, the rest of the line is ignored.
       After 10 errors, scanning is aborted. 

        The first writes to the given output file with the format:  
            symbol-name lexeme value (space-separaterd, one token per line)        

        The second simply fills the vector with the tokens rather than write to file 
    */
    bool scan(std::ofstream &output_file);
    bool scan(std::vector<Token> &scanner_output);

private:
    SymbolTable sym_table;
    Scanner scanner;

    int current_line;
    int error_count;
    const int MAX_ERRORS;

    // Construct the token list for input. Called by both scan methods
    std::vector<Token> tokenize();

    // Check if the token is one of the invalid types
    bool error_token(Token t);

    // Print an error message for invalid token types
    void print_error_msg(Token t);

    // Skip the rest of the current line (until next newline token)
    void skip_line();
};

#endif