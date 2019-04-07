#ifndef BLOCK_TABLE_H
#define BLOCK_TABLE_H

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

class scope_error : public std::runtime_error 
{
public:
    scope_error(std::string err_msg): std::runtime_error(err_msg) {}
};  

struct BlockData
{
    PLType type;
    int size;
    bool constant;
    int displacement;
    int var_start;
    bool array;
    int value;
    int level;
};

typedef std::map<std::string, BlockData> Block;

class BlockTable
{
public:
    BlockTable();

    void push_new();

    void pop();

    // If not found throws scope_error
    BlockData& find(std::string id);

    // If already exists in same block, throw scope_error
    void insert(std::string id, BlockData new_block);
    //     std::string id, 
    //     PLType t, 
    //     int size,
    //     int value,
    //     bool constant,
    //     int displacement,
    //     int start,
    //     bool array
    // );

    int curr_level;

private:
    std::vector<Block> table;
};

#endif