cmake_minimum_required(VERSION 3.6)
project(libstratum)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -pedantic -Wextra -Wall -Wno-unused-variable")

set(SOURCE_FILES main.c queue.c queue.h vstring.c vstring.h tests.c)
add_executable(libstratum ${SOURCE_FILES})
target_link_libraries(libstratum pthread)