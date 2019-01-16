#ifndef SCANNER_H
#define SCANNER_H

#include "token.h"
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
    const std::string &input;
    std::string::const_iterator next_char;

    // Character types
    bool letter(char);
    bool digit(char);
    bool separator(char);

    // Advance next_char while pointing to separator
    void skip_separators();

    Symbol scan_word();
    Symbol scan_numeral();

    Symbol next_symbol();
};

#endif