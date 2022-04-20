cmake_minimum_required(VERSION 3.21)
project(IJC C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -pedantic -Wall -lm -Wextra")

add_executable(proj2 proj2.c)