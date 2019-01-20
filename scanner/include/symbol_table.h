#ifndef SCANNER_SYMBOL_TABLE_H
#define SCANNER_SYMBOL_TABLE_H

#include "token.h"
#include <vector>
#include <string>

typedef std::unique_ptr<Token> token_ptr;

class SymbolTable
{
public:
    SymbolTable();

    // Check if table contains key
    bool contains(std::string key);

    // Return the token indexed by key or throw runtime error if not found
    Token get(std::string key);

    /*  Insert a new token with a key given by the token's lexeme or throw a runtime error if entry
        already exists
    */
    void insert(Token token);

private:
    std::vector<token_ptr> tok_table;
    int used;

    int hash_fn(std::string key);
};

#endif