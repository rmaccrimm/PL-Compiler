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
    /*  Construct a scanner which will read from the given input stream and store identifiers in 
        the given symbol table
    */
    Scanner(std::ifstream &program_file, SymbolTable &symbol_table);

    // Return the next token in input
    Token get_token(); 

private:
    // Store a reference since symbol table will be shared between components
    SymbolTable &sym_table;

    // Points to the next character to parse in the input
    std::istream_iterator<char> next_char;
    // End of stream iterator
    std::istream_iterator<char> eos_iter;

    // 3 Character types    
    bool letter(char c);
    bool digit(char c);
    bool special_symbol(char c);

    // True for any character that can separate words or numerals, i.e. white space or a symbol
    bool separator(char c);

    // Check if end of file has been reached, i.e. next_char is at the end of the input stream
    bool eof();

    // Advance next_char while pointing to space or tab
    void skip_whitespace();

    // Advance next_char until next newline char
    void skip_line();

    // Construct a token for a keyword or identifier (anything beginning with a letter)
    Token scan_word();

    // Construct a token for a numeral (begins with a number)
    Token scan_numeral();

    // Construct a token for a special symbol, e.g. +, -, >= etc., including newline char
    Token scan_symbol();
};

#endif