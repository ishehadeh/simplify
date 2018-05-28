cmake_minimum_required(VERSION 3.0.0)
project(simplify C)

set(CMAKE_MODULE_PATH CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_EXPORT_COMPILE_COMMAND ON)

include(CTest)

file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/simplify/*.c" "${CMAKE_SOURCE_DIR}/src/simplify/*/*.c")
file(GLOB HEADERS "${CMAKE_SOURCE_DIR}/src/simplify/*.h" "${CMAKE_SOURCE_DIR}/src/simplify/*/*.h")
file(GLOB MANUALS "${CMAKE_SOURCE_DIR}/docs/simplify.*.md")
string(REPLACE ";" "\t" HEADERS_STR "${HEADERS}")
string(REPLACE ";" "\t" SOURCES_STR "${SOURCES}")
string(REPLACE ";" "\t" MANUALS_STR "${MANUALS}")
set(CLDOC_FLAGS ${CLDOC_FLAGS} --language c --output ${CMAKE_SOURCE_DIR}/docs/libsimplify ${HEADERS_STR})

if(EXISTS "${CMAKE_SOURCE_DIR}/deps/root/usr/local")
    set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/deps/root/usr/local")
endif()

if (SIMPLIFY_LINK_STATIC)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
endif()

if (SIMPLIFY_MINIMIZE_ALLOCATIONS)
    add_definitions(-DRBTREE_USE_CHUNKS=1)
endif()

find_package(GMP REQUIRED)
find_package(MPFR REQUIRED)

find_program(CLDOC cldoc cldoc.py DOC "c/c++/obj-c documentation generator")
find_program(RONN  ronn  ronn.rb  DOC "markdown to man converter")
find_program(GCOV  gcov           DOC "GNU test coverage tool")

include_directories("${CMAKE_SOURCE_DIR}/src" "${CMAKE_SOURCE_DIR}" "${GMP_INCLUDE_DIR}" "${MPFR_INCLUDES}")
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wall -Wextra -std=c99 -Werror")

add_custom_target("docs")

if (CLDOC STREQUAL "CLDOC-NOTFOUND")
    message(WARNING "cldoc was not found, the docs.lib.* targets will not be available")
else()
    add_custom_target("docs.lib.html" COMMAND ${CLDOC} generate -std=c99 -I${CMAKE_SOURCE_DIR}/src -- --type html ${CLDOC_FLAGS})
    add_custom_target("docs.lib.xml"  COMMAND ${CLDOC} generate -std=c99 -I${CMAKE_SOURCE_DIR}/src -- --type xml  ${CLDOC_FLAGS})

    add_custom_target("docs.lib" DEPENDS "docs.lib.html" "docs.lib.xml")

    add_dependencies("docs" "docs.lib")
endif()

if (RONN STREQUAL "RONN-NOTFOUND")
    message(WARNING "ronn was not found, the docs.man.* targets will not be available")
else()
    add_custom_target("docs.man.html" COMMAND ${RONN}  -5 "${MANUALS_STR}" )
    add_custom_target("docs.man.roff" COMMAND ${RONN}  -r "${MANUALS_STR}" )

    add_custom_target("docs.man" DEPENDS "docs.man.html" "docs.man.roff")

    add_dependencies("docs" "docs.man")
endif()

if (GCOV STREQUAL "GCOV-NOTFOUND")
    message(WARNING "gcov was not found, the coverage target will not be available")
    set(GCOV "")
    set(HAVE_GCOV FALSE)
else()
    add_custom_target("coverage" COMMAND ${GCOV} ${SOURCES_STR} ${CMAKE_BINARY_DIR}/CMakeFiles/simplify.dir/src/simplify/**/*.gcno)
    set(HAVE_GCOV TRUE)
endif()

add_library(simplify STATIC ${SOURCES})

add_executable(simplify-bin ${CMAKE_SOURCE_DIR}/src/simplify.c)
set_target_properties(simplify-bin PROPERTIES OUTPUT_NAME simplify)

target_link_libraries(simplify ${MPFR_LIBRARIES})
target_link_libraries(simplify ${GMP_LIBRARIES})
target_link_libraries(simplify m)

target_link_libraries(simplify-bin simplify)

if(BUILD_TESTING)
    if (HAVE_GCOV)
        set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -O0 -g --coverage")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    endif()

    add_executable(test_rbtree     ${CMAKE_SOURCE_DIR}/test/rbtree.c)
    add_executable(test_lexer      ${CMAKE_SOURCE_DIR}/test/lexer.c)
    add_executable(test_parser     ${CMAKE_SOURCE_DIR}/test/parser.c)
    add_executable(test_expression ${CMAKE_SOURCE_DIR}/test/expression.c)
    add_executable(test_compare    ${CMAKE_SOURCE_DIR}/test/compare.c)
    add_executable(test_stringify  ${CMAKE_SOURCE_DIR}/test/stringify.c)

    target_link_libraries(test_rbtree     simplify)
    target_link_libraries(test_lexer      simplify)
    target_link_libraries(test_parser     simplify)
    target_link_libraries(test_expression simplify)
    target_link_libraries(test_compare    simplify)
    target_link_libraries(test_stringify  simplify)

    add_test(NAME rbtree     COMMAND test_rbtree)
    add_test(NAME lexer      COMMAND test_lexer)
    add_test(NAME parser     COMMAND test_parser)
    add_test(NAME expression COMMAND test_expression)
    add_test(NAME compare    COMMAND test_compare)
    add_test(NAME stringify  COMMAND test_stringify)
endif()