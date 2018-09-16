cmake_minimum_required(VERSION 3.12)
project(myshell)

set(CMAKE_CXX_STANDARD 17)

add_executable(myshell main.cpp myshell.cpp myshell.h)