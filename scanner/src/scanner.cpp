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

Symbol Scanner::scan_word()
{
    assert(letter(*next_char));

    std::string word;

    while (!separator(*next_char)) {
        bool valid_word_char = letter(*next_char) || digit(*next_char) || *next_char != '_';
        assert(valid_word_char);
        word += *next_char;
        next_char++;
    }
    
}

Symbol Scanner::scan_numeral()
{

}

Symbol next_symbol()
{

}
