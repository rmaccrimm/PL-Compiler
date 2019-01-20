#include "symbol_table.h"
#include <cassert>
#include <iostream>
#include <stdexcept>

SymbolTable::SymbolTable() : used{0}
{
    tok_table.resize(100);
}

int SymbolTable::hash_fn(std::string key)
{
    // djb2 hash function - found online at http://www.cse.yorku.ca/~oz/hash.html
    unsigned long hash = 5381;
    for (char c: key) {
        hash = ((hash << 5) + hash) + (int)c; /* hash * 33 + c */
    }
    hash %= tok_table.size();

    // Linear probing for collisions
    if (tok_table[hash]) {
        while (tok_table[hash]->lexeme != key) {
            hash = (hash + 1) % tok_table.size();
        }
    }
    return hash;
}

bool SymbolTable::contains(std::string key)
{
    int hash = hash_fn(key);
    return tok_table[hash] != nullptr;
}

Token SymbolTable::get(std::string key) 
{
    int hash = hash_fn(key);
    if (tok_table[hash]) {
        return *tok_table[hash];
    }
    else {
        std::cerr << "No entry for key " + key + " in symbol table" << std::endl;
        throw std::runtime_error("Key not found");
    }
}

void SymbolTable::insert(Token tok)
{
    if (contains(tok.lexeme)) {
        std::cerr << "Symbol table already contains entry for key " + tok.lexeme << std::endl;
        throw std::runtime_error("Key already found");
    }
    double load = (double)used / (double)tok_table.size();
    if (load > 0.7) {
        tok_table.resize(tok_table.size() * 2);
    }
    int hash = hash_fn(tok.lexeme);
    tok_table[hash] = std::make_unique<Token>(Token(tok));
    used++;
}