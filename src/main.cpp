#include "symbol_table.h"
#include "scanner.h"
#include <fstream>
#include <iterator>
#include <iostream>
#include <string>
#include <compiler.h>

int main(int argc, char *argv[]) 
{
    std::ifstream file_in("test/error_test_program.pl");
    if (!file_in.good()) {
        std::cerr << "failed to open input file" << std::endl;
        return -1;
    }
    std::ofstream file_out("test/test_output");
    if (!file_out.good()) {
        std::cerr << "failed to open output file" << std::endl;
        return -1;
    }

    Compiler compiler(file_in);
    if (compiler.scan(file_out)) {
        std::cout << "Scan completed without errors." << std::endl;
    }
}