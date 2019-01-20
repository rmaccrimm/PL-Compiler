#include "token.h"

Token::Token(Symbol tok_symbol, std::string tok_lexeme, int tok_value) :
    symbol{tok_symbol}, lexeme{tok_lexeme}, value{tok_value} {}

Token::Token(const Token &tok) : symbol{tok.symbol}, lexeme{tok.lexeme}, value{tok.value} {}

void Token::operator=(const Token &tok)
{
    symbol = tok.symbol;
    lexeme = tok.lexeme;
    value = tok.value;
}