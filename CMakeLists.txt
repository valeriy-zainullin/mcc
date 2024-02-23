cmake_minimum_required(VERSION 3.12)

add_compile_options("-g")

project(mcc)

set(CMAKE_CXX_STANDARD 20)
add_compile_options(-fsanitize=address,undefined)
add_link_options(-fsanitize=address,undefined)

find_package(FLEX  2.6 REQUIRED)
find_package(BISON 2.6 REQUIRED)

set(
    HEADERS

    ${PascalInterpreter_SOURCE_DIR}/driver.hh
    ${PascalInterpreter_SOURCE_DIR}/parser.hh
)

BISON_TARGET(
    MyParser

    parser.y
    ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
    COMPILE_FLAGS --graph
    DEFINES_FILE     ${CMAKE_CURRENT_BINARY_DIR}/parser.hh
)

FLEX_TARGET(
    MyScanner
    scanner.l
    ${CMAKE_CURRENT_BINARY_DIR}/scanner.cpp
)

ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

add_executable(
    mcc

    main.cpp
    driver.cpp
    ast.cpp
    ${BISON_MyParser_OUTPUTS}
    ${FLEX_MyScanner_OUTPUTS}
)

add_custom_target(test COMMAND mcc ${CMAKE_CURRENT_LIST_DIR}/test.c)

target_include_directories(mcc PRIVATE ${CMAKE_CURRENT_LIST_DIR} -p -s -l ${CMAKE_CURRENT_BINARY_DIR})
