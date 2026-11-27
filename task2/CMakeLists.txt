cmake_minimum_required(VERSION 3.16)
project(phonebook_qt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Business logic shared with task 1 (no Qt dependency)
set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../task1)
add_library(core STATIC
    ${CORE_DIR}/src/Validator.cpp
    ${CORE_DIR}/src/FileStorage.cpp
    ${CORE_DIR}/src/PhoneBook.cpp)
target_include_directories(core PUBLIC ${CORE_DIR}/include)

add_executable(phonebook_qt
    src/main.cpp
    src/MainWindow.cpp
    src/ContactDialog.cpp
    src/ContactTableModel.cpp
    src/ContactFilterProxy.cpp)
target_include_directories(phonebook_qt PRIVATE src)
target_link_libraries(phonebook_qt PRIVATE core Qt6::Widgets)