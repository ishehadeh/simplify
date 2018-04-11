cmake_minimum_required(VERSION 3.0.0)
project(simplify C)

set(CMAKE_MODULE_PATH CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include_directories("${CMAKE_SOURCE_DIR}/src")

file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/simplify/*.c")

find_package(GMP)
find_package(MPFR)

option(INTEGERS_ONLY "Use integers instead of floating point values" FALSE)
option(DISABLE_GMP   "Don't use GMP, even if it is found" FALSE)
option(DISABLE_MPFR  "Don't use MPFR, even if it is found" FALSE)

if (INTEGERS_ONLY)
    add_definitions(-DSCALAR_INTEGER)
else()
    if(NOT FLOAT_PRECISION)
        set(FLOAT_PRECISION 32)
    endif()
    add_definitions(-DFLOAT_PRECISION=${FLOAT_PRECISION})
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

add_executable(simplify ${SOURCES})

if (GMP_FOUND AND NOT DISABLE_GMP)
    target_link_libraries(simplify ${GMP_LIBRARIES})
    add_definitions(-DHAVE_GMP)

    if (MPFR_FOUND AND NOT DISABLE_MPFR)
        target_link_libraries(simplify ${MPFR_LIBRARIES})
        add_definitions(-DHAVE_MPFR)
    endif()
endif()

target_link_libraries(simplify m)