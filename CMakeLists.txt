cmake_minimum_required(VERSION 3.5)


project(lsqecc)


####################################################
# General configuration

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g3 -Wfatal-errors -Wall -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g3 -D_LIBCPP_DEBUG=1 -DNDEBUG")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)


####################################################
# Dependencies

find_package(Boost COMPONENTS graph)
if(Boost_FOUND)
    add_definitions(-DENABLE_BOOST_GRAPH_SEARCH)
endif()

# set(PYTHON_VERSION 3.9)
# set(Python_ROOT_DIR /usr/lib/python3.9)
# set(PYTHON_LIBRARIES /usr/lib/libpython3.10.so)
#find_package(PythonLibs)
#include_directories(/usr/include/python3.10)

include_directories(external/ordered-map/include)

include_directories(external/json/include)


# Packages that have their headers at the top level
include_directories(external/include)

###################################################
# Library setup

add_library(
        lsqecclib
        src/patches/fast_patch_computation.cpp
        src/ls_instructions/ls_instructions.cpp
        src/ls_instructions/ls_instructions_parse.cpp
        src/ls_instructions/parse_utils.cpp
        src/patches/patches.cpp
        src/patches/slice.cpp
        src/patches/slices_to_json.cpp
        src/layout/graph_search/boost_based_graph_search.cpp
        src/layout/graph_search/custom_graph_search.cpp
        src/layout/router.cpp
        src/layout/ascii_layout_spec.cpp
        src/layout/layout.cpp
        src/ls_instructions/ls_instruction_stream.cpp)

set(LSQECCLIB_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include")

target_include_directories(
        lsqecclib
        PUBLIC
        ${LSQECCLIB_INCLUDE_DIRS}
)

set_property(TARGET lsqecclib PROPERTY POSITION_INDEPENDENT_CODE ON)


###################################################
# Command line executable
add_executable(
        lsqecc_slicer
        src/slicer.cpp
)

target_link_libraries(
        lsqecc_slicer PUBLIC lsqecclib
)

###################################################
# Emscripten interface

if(DEFINED CMAKE_CROSSCOMPILING_EMULATOR)

    add_executable(
            lsqecc_emscripten
            src/emscripten_bindings.cpp
    )

    target_link_libraries(
            lsqecc_emscripten PUBLIC lsqecclib
    )

    target_include_directories(
            lsqecc_emscripten
            PUBLIC
            #TODO
    )
endif()



###################################################
# Python Interface


if(NOT DEFINED CMAKE_CROSSCOMPILING_EMULATOR)
    add_subdirectory(external/pybind11)

    pybind11_add_module(lsqecclib_bybind ${PROJECT_SOURCE_DIR}/src/lsqecclib_pybind.cpp)

    target_include_directories(
            lsqecclib_bybind
            PUBLIC
            ${INCLUDE_DIRS}
    )

    target_link_libraries(
            lsqecclib_bybind PUBLIC lsqecclib
    )
endif()

###################################################
# Tests

enable_testing()


# lib
add_executable(
        lsqecc_tests
        tests/main.cpp
        tests/patches/fast_patch_computation.cpp
        tests/logical_lattice_ops/ls_instructions.cpp
        tests/logical_lattice_ops/ls_instructions_parse.cpp)
target_link_libraries(
        lsqecc_tests
        lsqecclib
        gtest
        gtest_main
)
add_test(
        NAME unit
        COMMAND ${PROJECT_BINARY_DIR}/lsqecc_tests
)
#add_custom_command(
#        TARGET lsqecc_tests
#        COMMENT "Run lsqecc_tests"
#        POST_BUILD
#        COMMAND valgrind ${PROJECT_BINARY_DIR}/lsqecc_tests
#)
