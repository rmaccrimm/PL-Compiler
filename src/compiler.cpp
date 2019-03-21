#include "compiler.h"
#include "symbol.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

Compiler::Compiler(std::ifstream &input_file) : 
    scanner(input_file, sym_table), current_line{1}, error_count{0}, MAX_ERRORS{10} {}

bool Compiler::run()
{
    std::vector<Token> input_stream;    
    if (!scan(input_stream)) {
        std::cout << "Scan completed without errors" << std::endl;
    }
    if (!parser.verify_syntax(&input_stream)) {
        std::cout << "Parsing completed without errors" << std::endl;
    }
}

int Compiler::scan(std::vector<Token> &scanner_output)
{
    auto token_list = tokenize();
    scanner_output.resize(token_list.size());
    std::copy(token_list.begin(), token_list.end(), scanner_output.begin());
    // False if errors were found in tokenization
    return error_count;
}

std::vector<Token> Compiler::tokenize()
{
    Token tok;
    std::vector<Token> token_list;
    
    do {
        tok = scanner.get_token();
        if (error_token(tok)) {
            error_count++;
            print_error_msg(tok);
            // Only one error allowed per line, rest of line is ignored
            skip_line();
            if (error_count >= MAX_ERRORS) {
                std::cerr << "Number of errors reached maximum, aborting scan" << std::endl;
                break;
            }
        }
        else if (tok.symbol == NEWLINE) {
            current_line++;
        }
        token_list.push_back(tok);
    } while ((tok.symbol != END_OF_FILE) && (error_count <= MAX_ERRORS));

    return token_list;
}

void Compiler::skip_line()
{
    Token tok;
    do {
        tok = scanner.get_token();
    } while (tok.symbol != NEWLINE && tok.symbol != END_OF_FILE);
    current_line++;
}

bool Compiler::error_token(Token t)
{
    // Check if the token is one of the invalid symbols
    switch (t.symbol)
    {
        case INVALID_CHAR:
        case INVALID_NUMERAL:
        case INVALID_SYMBOL:
        case INVALID_WORD:
            return true;
        default:
            return false;
    }
}

void Compiler::print_error_msg(Token t)
{
    std::cerr << "Error " << error_count << " on line " << current_line << ": ";
    Symbol s = t.symbol;
    if (s == INVALID_CHAR) {
        std::cerr << "Unrecognized character ";
    }
    else if (s == INVALID_NUMERAL) {
        std::cerr << "Invalid numeral "; 
    }
    else if (s == INVALID_SYMBOL) {
        std::cerr << "Invalid symbol ";
    }
    else if (s == INVALID_WORD) {
        std::cerr << "Invalid identifier ";
    }
    else  {
        std::cerr << "No error -- Shouldn't be here " << std::endl;
    }
    std::cerr << "\"" << t.lexeme << "\"" << std::endl;
}