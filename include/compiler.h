#ifndef PL_COMPILER_H
#define PL_COMPILER_H

#include <fstream>
#include "scanner.h"
#include "symbol_table.h"

/*  An administration class that manages each of the separate compilation stages. Responsible for
    creating storing, and calling methods for each of the seperate classes. Currnetly only the 
    scanner is implemented
*/
class Compiler
{
public:
    // Constructor takes the filepath of the PL source file to be compiled
    Compiler(std::ifstream &input_file);

    /*  Create a list of tokens from the input file and write to the given output file. If errors
        are detected, returns false and no output is written.
    */
    bool scan(std::ofstream &output_file);

private:
    SymbolTable sym_table;
    Scanner scanner;

    int current_line;
    int error_count;
    const int MAX_ERRORS;

    bool error_token(Token t);

    void print_error_msg(Token t);

    void skip_line();
};

#endif