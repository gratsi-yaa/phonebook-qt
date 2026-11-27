cmake_minimum_required(VERSION 3.16)
project(phonebook_console LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(phonebook src/main.cpp)
target_include_directories(phonebook PRIVATE include)
