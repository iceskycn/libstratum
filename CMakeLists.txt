cmake_minimum_required(VERSION 3.6)
project(libstratum)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -fPIC -pedantic -Wextra -Wall -Wno-unused-variable -Wno-gnu-zero-variadic-macro-arguments -Wno-unused-function -fno-omit-frame-pointer")

set(SOURCE_FILES main.c queue.c queue.h vstring.c vstring.h tests.c protocol.c hash_table.c mem.h hash_table.h binary_tree.h.c types.h graph.c graph.h json_parser.c stack.h log.h log.c json_parser.h)
add_executable(libstratum ${SOURCE_FILES})
target_link_libraries(libstratum pthread)