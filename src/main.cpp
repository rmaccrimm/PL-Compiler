#include "symbol_table.h"
#include "scanner.h"
#include "compiler.h"
#include "parser.h"
#include <fstream>
#include <iterator>
#include <iostream>
#include <string>

#include <algorithm>

const std::string usage_info = "Usage:\n\tplc src_file [-o output_file]";

int main(int argc, char *argv[]) 
{
    /*  Parse command line arguments
    */
    if (argc < 2) {
        std::cout << "Missing source file" << std::endl << usage_info << std::endl;
        return -1;
    }
    std::string input_file(argv[1]);
    std::string output_file = "scanner.out";

    auto it = std::find(argv, argv + argc, std::string("-o"));
    if (it != argv + argc) {
        if (it == argv + (argc - 1)) {
            std::cout << "-o option must be followed by output file" << std::endl 
                      << usage_info << std::endl;
            return -1;
        }
        output_file = std::string(*(it + 1));
    }

    /* Open input/output files
    */
    std::ifstream file_in(input_file);
    if (!file_in.good()) {
        std::cerr << "failed to open input file " + input_file << std::endl 
                  << usage_info << std::endl;
        return -1;
    }
    std::ofstream file_out(output_file);
    if (!file_out.good()) {
        std::cerr << "failed to open output file " + output_file << std::endl;
        return -1;
    }

    /* Compilation
    */
    std::vector<Token> tokens;
    Compiler compiler(file_in);
    compiler.run();
}