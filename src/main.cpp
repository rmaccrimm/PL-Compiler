#include "symbol_table.h"
#include "scanner.h"
#include <fstream>
#include <iterator>
#include <iostream>
#include <string>
#include <compiler.h>

int main(int argc, char *argv[]) 
{
    std::ifstream file_in("error_test_program");
    if (!file_in.good()) {
        std::cerr << "failed to input open file" << std::endl;
        return -1;
    }
    std::ofstream file_out("test_output");
    if (!file_out.good()) {
        std::cerr << "failed to open output file" << std::endl;
        return -1;
    }

    Compiler compiler(file_in);
    if (compiler.scan(file_out)) {
        std::cout << "Scan completed successfully. Wrote output to file" << std::endl;
    }
    else {
        std::cout << "Scanner errors detected. No output written." << std::endl;
    }
}