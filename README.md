# CPSC 4600 Project Language (PL) Compiler
Roderick MacCrimmon (single person group, responsible for whole project)

Estimated time spent on project: 10-15 hours for part 1 (scanner), 10 for part 2 (parser), 15-20 for part 3

## Files & Directory Layout
(excluding CMake build files)

  - README.pdf
  - README.md
  - technical_doc.pdf
  - **include/** - header files
    - compiler.h
    - parser.h
    - scanner.h
    - symbol_table.h
    - symbol.h
    - token.h
  - **src/** - implementation files
    - main.cpp - contains main function for plc
    - parser.cpp
    - compiler.cpp
    - scanner.cpp
    - symbol_table.cpp
    - token.cpp
  - **test/**
    - catch.hpp - CATCH2 unit testing library header
    - main.cpp - defines main needed for running unit tests
    - test_scanner.cpp - unit tests
    - **src_files/** - PL source code used for testing
      - valid_test_program.pl
      - **scan** - subdirectories contain files used by unit tests
      - **scope**
      - **type**
  - **docs/**
    - grammar.txt
    - technical_doc.tex
    - pl_grammar.tex

## Build Instructions
This project uses CMake to automatically generate makefiles. It has been tested to ensure it builds
on the linux lab computers without any compilation errors.

From the project root directory run the following commands:
```
mkdir build
cd build
cmake ..
make
```
This will produce two separate executables - one for the main compiler at build/src/plc, and a 
second for the automated unit tests at build/test/run_tests

## Usage Instructions
The compiler can be run with
```
./plc src_file 
```

One valid test program is provided to see parser output at test/src_files/valid_test_program.pl. The
program has been confirmed to run correctly for this file on the lab computers.

The scanner unit tests can be run from the project root directory with
```
./build/test/run_tests
```
Because the input filepaths are currently hard-coded, this must be run from the project root directory.
