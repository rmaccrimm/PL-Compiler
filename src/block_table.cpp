#include "block_table.h"
#include <cassert>

bool equals(const PLType &lhs, const PLType &rhs)
{
    // UNDEFINED is a universal type that compares equal to everything, avoiding cascading errors
    if ((int)lhs == 0 || (int)rhs == 0) {
        return true;
    }
    else return (int)lhs == (int)rhs;
}


BlockTable::BlockTable(): curr_level{0} {}

void BlockTable::push_new() {
    curr_level++;
    table.push_back(Block());
}

void BlockTable::pop()
{
    assert(table.size() > 0);
    table.erase(table.end() - 1);
    curr_level--;
}

BlockData& BlockTable::find(std::string id) 
{
    assert(table.size() > 0);
    for (auto it = table.rbegin(); it != table.rend(); it++) {
        if (it->find(id) != it->end()) {
            return it->at(id);
        }
    }
    throw scope_error("Identifier " + id + " is undefined");
}

void BlockTable::insert(std::string id, BlockData new_block)
{
    assert(table.size() > 0);
    auto last = table.rbegin();
    if (last->find(id) != last->end()) {
        throw scope_error("Identifier " + id + " already defined in currents scope");
    }
    // automatically set the level
    new_block.level = curr_level;
    last->emplace(id, new_block);
}
