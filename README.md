# CPSC 4600 Project Language (PL) Compiler
Roderick MacCrimmon (single person group, responsible for whole project)

Estimated time spent on project: 10-15 hours for part 1 (scanner), 10 for part 2 (parser), 15-20 for part 3 (type and scope checking), 10-15 for part 4 (code generation)

## Files & Directory Layout
(excluding CMake build files)

  - README.md
  - README.pdf
  - technical_doc.pdf
  - **include/** - header files
    - block_table.h
    - compiler.h
    - parser.h
    - scanner.h
    - symbol_table.h
    - symbol.h
    - token.h
  - **src/** - implementation files
    - block_table.cpp        
    - compiler.cpp
    - main.cpp 
    - parser.cpp
    - scanner.cpp
    - symbol_table.cpp
    - token.cpp
  - **test/**
    - catch.hpp - CATCH2 unit testing library header
    - main.cpp - defines main needed for running unit tests
    - test_scanner.cpp 
    - test_parser.cpp
    - **src_files/** - PL source code used for testing, subdirectories contain files used by unit tests
      - **scan/** 
      - **scope/**
      - **type/**
  - **docs/**
    - grammar.txt
    - technical_doc.tex
  - **interpreter/** - Not my own work - contains the assembler and interpreter source code
    provided on Moodle with minor revisions
  - **demos/** - example programs written in PL, code contains comments explaining purpose
    - add_procedure.txt
    - algebra.txt
    - boolean.txt
    - bubble_sort.txt
    - comparisonst.txt
    - Fibonacci_numbers.txt
    - recursion.txt
    - reverse_list.txt

## Build Instructions
This project uses CMake to automatically generate makefiles. It has been tested to ensure it builds on the linux lab computers without any compilation errors.

From the project root directory run the following commands:
```
mkdir build
cd build
cmake ..
make
```
This will produce three separate executables:
  - build/src/plc - compiler
  - build/test/run_tests - automatic unit tests
  - build/interpreter/plinterp - interpreter

## Usage Instructions
The compiler can be run with
```p
./plc src-file [-o output-file] [-d]
```
The -o flag is used to specify the output file, which will otherwise be a.out by default.

The -d flag enables debug mode, which will print the resulting intermediate code to the command
line, as well as to the output file. Note that if errors occurr, no output will be written to file.

The output file can then be passed as the input for the interpreter, which will produce an output 
file called assembly.out and load and run this file with the interpreter
```
./plinterp input-file
```

Note: All programs included in the demos directory and the unit tests have been confirmed prior to submission to run on the linux lab computers without any run-time errors.