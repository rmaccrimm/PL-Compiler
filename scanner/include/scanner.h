#ifndef SCANNER_H
#define SCANNER_H

#include "token.h"
#include "symbol_table.h"
#include <vector>
#include <string>
#include <iterator>

class Scanner
{
public:
    // Construct scanner for input program
    Scanner(const std::string &program_text);

    // Return the next token in input
    Token get_token(); 

private:
    SymbolTable symbol_table;
    std::istream_iterator<char> next_char;

    // Character types
    bool letter(char);
    bool digit(char);
    bool separator(char);

    // Advance next_char while pointing to separator
    void skip_separators();

    // Construct a token for a keyword or identifier (anything beginning with a letter)
    Token scan_word();

    // Construct a token for a numeral (begins with a number)
    Token scan_numeral();

};

#endif