#ifndef PL_SYMBOL_TABLE_H
#define PL_SYMBOL_TABLE_H

#include "token.h"
#include <vector>
#include <string>

/*  Unique pointers are used to create new table entries in order to avoid the need for explicit
    deletion or a destructor
*/
typedef std::unique_ptr<Token> token_ptr;

class SymbolTable
{
/*  A hash table used to store Tokens, hashed by their lexeme
*/

public:
    /*  Construct a symbol table of size 101 (prime number) with Tokens inserted for all reserved 
        words inserted initially
    */
    SymbolTable();

    // Check if table contains key
    bool contains(std::string key);

    /*  Return the token indexed by key. Should always call contains to check if entry exists. 
        Throws runtime error if key not found
    */
    Token get(std::string key);

    /*  Insert a new token with a key given by the token's lexeme. Should always call contains first
        to see if entry already exists. Throws a runtime error if entry with same lexeme found.
        If load exceeds 70% table will double in size.
    */
    void insert(Token token);

private:
    std::vector<token_ptr> tok_table;
    // The number of elements currently in the table
    int used;

    // The hash function. Linear probing is used for collisions.
    int hash_fn(std::string key);
};

#endif
