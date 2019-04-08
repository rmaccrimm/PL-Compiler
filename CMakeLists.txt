project(PL_COMPILER)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
   set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(interpreter)
