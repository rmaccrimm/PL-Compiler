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

    if (next_char == eof) {
        /*  Once the end of the file is reached, any subsequent calls to get_token will continue to 
            return an end of file token
        */
        return Token(END_OF_FILE);
    }
    else if (*next_char == '$') {
        // $ starts a comment and the rest of the line following it is ignored 
        skip_line();
    }
    else if (letter(*next_char)) {
        return scan_word();
    }
    else if (digit(*next_char)) {
        return scan_numeral();
    }
    else if (special_symbol(*next_char)) {
        return scan_symbol();
    }
    else {
        // unrecognized character
        std::string c(1, *next_char);
        next_char++;
        return Token(INVALID_CHAR, c);
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
    /*  A.k.a whitespace characters. Newline is considered a symbol since we want a newline token 
        for debugging purposes */
    return (c == ' ') || (c == '\t');
}

bool Scanner::special_symbol(char c)
{
    // All symbols allowed in 
    char symbols[] = { '.', ',', ';', ':', '(', ')', '[', ']', '&', '|', '~', '<', '>', '+', '-', 
                       '=', '/', '\\', '\n' };
    for (auto s: symbols) {
        if (c == s) {
            return true;
        }
    }
    return false;
}

void Scanner::skip_whitespace()
{
    while (separator(*next_char)) {
        next_char++;
    }
}

void Scanner::skip_line()
{
    /*  Skip up to the next newline, but not the newline itself. This is for the case of statements
        followed by comments where we still want to retain the line information for the statement
    */
    while (*next_char != '\n') {
        next_char++;
    }
}

Token Scanner::scan_word()
{
    std::string word;
    bool invalid_word = false;
    while (!(separator(*next_char) || special_symbol(*next_char))) {
        // will remain true if at any point an invalid character is found
        invalid_word |= !(letter(*next_char) || digit(*next_char) || *next_char == '_');
        word += *next_char;
        next_char++;
    }
    if (invalid_word) {
        return Token(INVALID_WORD, word);
    }
    if (sym_table.contains(word)) {
        // If the word is already stored in the symbol table, just return that token
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
    while (!(separator(*next_char) || special_symbol(*next_char))) {
        numeral += *next_char;
        next_char++;
    }
    // A stringstream can be used to easily determine if a string represents a valid integer
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
            // The only valid case for a : character is when followed by =
            if (*next_char == '=') {
                lex += *next_char;
                next_char++;
                return Token(ASSIGN, lex);
            }
            else {
                return Token(INVALID_SYMBOL, lex);
            }
        case '(':
            return Token(LEFT_PARENTHESIS, lex);
        case ')':
            return Token(RIGHT_PARENTHESIS, lex);
        case '[':
            /*  A [ character can either be a left parenthesis for array indexing or the [] for 
                separating guarded statements
            */
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
            /*  Can be either a part of the conditional operator -> in guarded statements or 
                subtraction
            */
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
            return Token(DIVIDE, lex);
        case '\\':
            return Token(MODULO, lex);
        case '\n':
            // No lexeme avoids printing extra newline when debugging
            return Token(NEWLINE, "");
        default:
            return Token(INVALID_CHAR, lex);
    }
}

