#include "scanner.h"
#include <cassert>

Scanner::Scanner(const std::string &program_text) : input{program_text} 
{
    next_char = input.begin();
}

Token Scanner::get_token()
{

}

bool Scanner::letter(char c)
{
    // Alphabetic ASCII characters are sequential
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Scanner::digit(char c)
{
    // ASCII digits ordered 0 - 9
    return c >= '0' && c <= '9';
}

bool Scanner::separator(char c)
{
    return (c == ' ') || (c == '\n') || (c == '\t');
}

void Scanner::skip_separators()
{
    while (separator(*next_char)) {
        next_char++;
    }
}

Token Scanner::scan_word()
{
    // Just checks that we have not called this function incorrectly
    assert(letter(*next_char));

    std::string word;

    bool invalid_word = false;
    while (!separator(*next_char)) {
        // will remain true if at any point an invalid character is found
        invalid_word |= !(letter(*next_char) || digit(*next_char) || *next_char == '_');
        word += *next_char;
        next_char++;
    }
    if (invalid_word) {
        return Token(INVALID_WORD, word);
    }
    else {
        Token *lookup_tok = symbol_table.search(word);
        if (lookup_tok != nullptr) {
            return *lookup_tok;
        }
        else {
            /*  Symbol table is preinitialized with all keywords, so if word is not found, we
                have an identifier
            */
            return Token(IDENTIFIER, word);
        }
    }
    
}

Token Scanner::scan_numeral()
{

}

Symbol next_symbol()
{

}
