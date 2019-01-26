#ifndef SCANNER_H
#define SCANNER_H

#include "token.h"
#include "symbol_table.h"
#include <vector>
#include <fstream>
#include <iterator>

class Scanner
{
public:
    // Construct scanner for input program
    Scanner(std::ifstream &program_file, SymbolTable &symbol_table);

    // Return the next token in input
    Token get_token(); 

// private:
    SymbolTable &sym_table;
    std::istream_iterator<char> next_char, eof;

    // Character types    
    bool letter(char c);
    bool digit(char c);
    bool special_symbol(char c);
    bool separator(char c);


    // Advance next_char while pointing to separator
    void skip_whitespace();

    // Advance next_char until after newline encountered
    void skip_line();

    // Construct a token for a keyword or identifier (anything beginning with a letter)
    Token scan_word();

    // Construct a token for a numeral (begins with a number)
    Token scan_numeral();

    // Construct a token for a special symbol, e.g. +, -, >= etc.
    Token scan_symbol();
};

#endif