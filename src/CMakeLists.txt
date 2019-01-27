include_directories(${PROJECT_SOURCE_DIR}/include)
add_executable(main 
    main.cpp 
    symbol_table.cpp
    token.cpp
    scanner.cpp
    compiler.cpp
)