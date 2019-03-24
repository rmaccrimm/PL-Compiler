include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/test)

add_library(compiler
    ../src/token.cpp
    ../src/symbol_table.cpp
    ../src/scanner.cpp
    ../src/parser.cpp
)

add_executable(run_tests
    main.cpp
    test_scanner.cpp
    test_parser.cpp
)

target_link_libraries(run_tests compiler)