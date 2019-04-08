#ifndef PL_COMPILER_H
#define PL_COMPILER_H

#include <fstream>
#include <vector>
#include "scanner.h"
#include "parser.h"
#include "symbol_table.h"

/*  An administration class that manages each of the separate compilation stages. Responsible for
    creating storing, and calling methods for each of the seperate classes and writing final output
*/
class Compiler
{
public:
    /*  Constructor takes the filepath of the PL source file to be compiled, the output file, and 
        a flag indicating debug mode
    */
    Compiler(std::ifstream &input_file, std::ofstream &output_file, bool debug=false);

    // Compile program. Returns true if errors occurred
    bool run();

private:
    /*  Perform tokenization on the input file. Return false if errors are detected, true otherwise. 
        After 10 errors, scanning is aborted. scanner_output is resized and fileed with results
    */
    int scan(std::vector<Token> &scanner_output);

    std::ofstream &output;

    SymbolTable sym_table;
    Scanner scanner;
    Parser parser;

    int current_line;
    int error_count;
    const int MAX_ERRORS = 10;

    // Construct the token list for input using. 
    std::vector<Token> tokenize();

    // Check if the token is one of the invalid types
    bool error_token(Token t);

    // Print an error message for invalid token types
    void print_error_msg(Token t);

    // Skip the rest of the current line (until next newline token)
    void skip_line();
};

#endif