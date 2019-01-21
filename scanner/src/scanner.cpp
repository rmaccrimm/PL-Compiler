#include "scanner.h"
#include <cassert>
#include <sstream>

Scanner::Scanner(std::ifstream &program_file, SymbolTable &symbol_table) : 
    next_char{program_file}, sym_table{symbol_table} 
{
    // Make sure whitespace is not skipped when iterating over input stream
    program_file >> std::noskipws;
}

Token Scanner::get_token()
{
    skip_whitespace();
    if (*next_char == '$') {
        skip_line();
    }
    skip_whitespace();

    if (next_char == eof) {
        return Token(END_OF_FILE);
    }
    else if (letter(*next_char)) {
        return scan_word();
    }
    else if (digit(*next_char)) {
        return scan_numeral();
    }
    else {
        return scan_symbol();
    }
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

void Scanner::skip_whitespace()
{
    while (separator(*next_char)) {
        next_char++;
    }
}

void Scanner::skip_line()
{
    while (*next_char != '\n') {
        next_char++;
    }
    next_char++;
}

Token Scanner::scan_word()
{
    std::string word;
    bool invalid_word = false;
    char c = *next_char;
    while (!separator(c = *next_char)) {
        // will remain true if at any point an invalid character is found
        invalid_word |= !(letter(*next_char) || digit(*next_char) || *next_char == '_');
        word += *next_char;
        next_char++;
    }
    if (invalid_word) {
        return Token(INVALID_WORD, word);
    }
    if (sym_table.contains(word)) {
        return sym_table.get(word);
    }
    else {
        /*  Symbol table is preinitialized with all keywords, so if word is not found, we
            have an identifier
        */
        Token t(IDENTIFIER, word);
        sym_table.insert(t);
        return t;
    }
}

Token Scanner::scan_numeral()
{
    std::string numeral;
    bool invalid_numeral = false;
    while (!separator(*next_char)) {
        numeral += *next_char;
        next_char++;
    }
    std::stringstream ss(numeral);
    int val;
    if (ss >> val) {
        return Token(NUMERAL, numeral, val);
    }
    else {
        return Token(INVALID_NUMERAL, numeral);
    }
}

Token Scanner::scan_symbol()
{
    char sym = *next_char;
    next_char++;

    std::string lex(1, sym);

    switch (sym)
    {
        case '.':
            return Token(PERIOD, lex);
        case ',':
            return Token(COMMA, lex);
        case ';':
            return Token(SEMICOLON, lex);
        case ':':
            if (*next_char == '=') {
                lex += *next_char;
                next_char++;
                return Token(ASSIGN, lex);
            }
            else {
                lex += *next_char;
                next_char++;
                return Token(INVALID_SYMBOL, lex);
            }
        case '(':
            return Token(LEFT_PARENTHESIS, lex);
        case ')':
            return Token(RIGHT_PARENTHESIS, lex);
        case '[':
            if (*next_char == ']') {
                lex += *next_char;
                next_char++;
                return Token(DOUBLE_BRACKET, lex);
            }
            else {
                return Token(LEFT_BRACKET, lex);
            }
        case ']':
            return Token(RIGHT_BRACKET, lex);
        case '&':
            return Token(AND, lex);
        case '|':
            return Token(OR, lex);
        case '~':
            return Token(NOT, lex);
        case '<':
            return Token(LESS_THAN, lex);
        case '>':
            return Token(GREATER_THAN, lex);
        case '+':
            return Token(ADD, lex);
        case '-':
            if (*next_char == '>') {
                lex += *next_char;
                next_char++;
                return Token(RIGHT_ARROW, lex);
            }
            else {
                return Token(SUBTRACT, lex);
            }
            break;
        case '=':
            return Token(EQUALS, lex);
        case '/':
            return Token(MODULO, lex);
        case '\\':
            return Token(DIVIDE, lex);
        default:
            return Token(INVALID_CHAR, lex);
    }
}

