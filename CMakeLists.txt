cmake_minimum_required(VERSION 3.0.0)
project(simplify C)

set(CMAKE_MODULE_PATH CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_EXPORT_COMPILE_COMMAND ON)

include(CTest)

include_directories("${CMAKE_SOURCE_DIR}/src" "${CMAKE_SOURCE_DIR}")
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/simplify/*.c" "${CMAKE_SOURCE_DIR}/src/simplify/*/*.c")
file(GLOB HEADERS "${CMAKE_SOURCE_DIR}/src/simplify/*.h" "${CMAKE_SOURCE_DIR}/src/simplify/*/*.h")
string(REPLACE ";" "\t" HEADERS_STR "${HEADERS}")


find_package(GMP REQUIRED)
find_package(MPFR REQUIRED)

if (MSVC)
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} /W4 /WX /Za /std:c99")
else()
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wall -Wextra -std=c99 -Werror -O3")
endif()

add_custom_target("doc" COMMAND cldoc generate --std=c99 -I${CMAKE_SOURCE_DIR}/src -- --language c --output ${CMAKE_SOURCE_DIR}/docs "${HEADERS_STR}")
add_library(simplify STATIC ${SOURCES})


add_executable(simplify-bin ${CMAKE_SOURCE_DIR}/src/simplify.c)
set_target_properties(simplify-bin PROPERTIES OUTPUT_NAME simplify)

target_link_libraries(simplify ${GMP_LIBRARIES})
target_link_libraries(simplify ${MPFR_LIBRARIES})
target_link_libraries(simplify m)

target_link_libraries(simplify-bin simplify)

if(BUILD_TESTING)
    add_executable(test_rbtree ${CMAKE_SOURCE_DIR}/test/rbtree.c)
    add_executable(test_lexer ${CMAKE_SOURCE_DIR}/test/lexer.c)
    add_executable(test_parser ${CMAKE_SOURCE_DIR}/test/parser.c)

    target_link_libraries(test_rbtree  simplify)
    target_link_libraries(test_lexer   simplify)
    target_link_libraries(test_parser  simplify)

    add_test(NAME rbtree COMMAND test_rbtree)
    add_test(NAME lexer  COMMAND test_lexer)
    add_test(NAME parser COMMAND test_parser)
endif()