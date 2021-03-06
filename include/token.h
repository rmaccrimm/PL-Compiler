#ifndef PL_TOKEN_H
#define PL_TOKEN_H

#include "symbol.h"
#include <string>
#include <memory>

class Token
{
public:
    Token();

    Token(Symbol tok_symbol, std::string tok_lexeme = "", int tok_value = 0);

    Token(const Token &tok);

    void operator=(const Token &tok);

    Symbol symbol;
    std::string lexeme;
    int value;
};

#endif