cmake_minimum_required(VERSION 3.16)
project(phonebook_console LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
    add_compile_options(/utf-8)
endif()

add_library(core src/Validator.cpp)
target_include_directories(core PUBLIC include)

add_executable(phonebook src/main.cpp)
target_link_libraries(phonebook PRIVATE core)

enable_testing()
add_executable(validator_tests tests/test_validator.cpp)
target_link_libraries(validator_tests PRIVATE core)
add_test(NAME validator COMMAND validator_tests)