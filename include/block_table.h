#ifndef PL_BLOCK_TABLE_H
#define PL_BLOCK_TABLE_H

#include <stdexcept>
#include <map>
#include <vector>

// All of the valid types of identifiers
enum class PLType 
{
    UNDEFINED     = 0,
    INTEGER       = 1,
    BOOLEAN       = 2,
    PROCEDURE     = 3
};

bool equals(const PLType &lhs, const PLType &rhs);

// Allows scope error messages to be produced by block table and caught in parser
class scope_error : public std::runtime_error 
{
public:
    scope_error(std::string err_msg): std::runtime_error(err_msg) {}
};  

// Data stored in individual blocks of block table
struct BlockData
{
    PLType type;
    int size;           // 1 for regular variables, larger for arrays
    bool constant;      
    int displacement;   // used for variable access
    int start_addr;     // label for address of first instruction for procedures 
    bool array;
    int value;          // used for constant instructions
    int level;          // current block level, set automatically in block table
};

// A block maps identifiers to the corresponding data
typedef std::map<std::string, BlockData> Block;


class BlockTable
{
public:
    BlockTable();

    // Create a new block
    void push_new();

    // Remove a block
    void pop();

    // If id not found throws scope_error
    BlockData& find(std::string id);

    // If already exists in same block, throw scope_error
    void insert(std::string id, BlockData new_block);

    // The level for top block on the stack
    int curr_level;

private:
    std::vector<Block> table;
};

#endif