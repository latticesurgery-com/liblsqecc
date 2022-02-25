cmake_minimum_required(VERSION 3.5)


project(lsqecc)


####################################################
# General configuration

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g -Wfatal-errors -Wall -pedantic -D_LIBCPP_DEBUG=1 -Wno-c++11-narrowing")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)


####################################################
# Dependencies

find_package(Boost REQUIRED COMPONENTS filesystem graph)

# set(PYTHON_VERSION 3.9)
# set(Python_ROOT_DIR /usr/lib/python3.9)
# set(PYTHON_LIBRARIES /usr/lib/libpython3.10.so)
#find_package(PythonLibs)
#include_directories(/usr/include/python3.10)

include_directories(external/ordered-map/include)
add_subdirectory(external/pybind11)

set(ABSL_PROPAGATE_CXX_STD ON)
add_subdirectory(external/abseil-cpp)
include_directories(external/abseil-cpp/include)

include_directories(external/json/single_include)

include_directories(external/include)

###################################################
# Library setup

add_library(
        lsqecclib
        src/patches/fast_patch_computation.cpp
        src/logical_lattice_ops/logical_lattice_ops.cpp
        src/logical_lattice_ops/ls_instructions_parse.cpp
        src/logical_lattice_ops/parse_utils.cpp
        src/patches/patches.cpp
        src/patches/slice.cpp
        src/patches/slices_to_json.cpp
        src/layout/graph_search/boost_based_graph_search.cpp
        src/layout/router.cpp
        src/layout/ascii_layout_spec.cpp
        src/layout/layout.cpp)

set(INCLUDE_DIRS
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/external/pybind11/include")

target_include_directories(
        lsqecclib
        PUBLIC
        ${INCLUDE_DIRS}
)

set_property(TARGET lsqecclib PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(lsqecclib
    ${Boost_FILESYSTEM_LIBRARY}
)

target_link_libraries(lsqecclib absl::strings absl::str_format)

###################################################
# Command line executable
add_executable(
        lsqecc_slicer
        src/slicer.cpp
)

target_link_libraries(
        lsqecc_slicer PUBLIC lsqecclib ${Boost_FILESYSTEM_LIBRARY}
)

###################################################
# Python Interface

pybind11_add_module(lsqecclib_bybind ${PROJECT_SOURCE_DIR}/src/lsqecclib_pybind.cpp)

target_include_directories(
        lsqecclib_bybind
        PUBLIC
        ${INCLUDE_DIRS}
)

target_link_libraries(
        lsqecclib_bybind PUBLIC lsqecclib
)


###################################################
# Tests

enable_testing()


# lib
add_executable(
        lsqecc_tests
        tests/main.cpp
        tests/patches/fast_patch_computation.cpp
        tests/logical_lattice_ops/logical_lattice_ops.cpp tests/logical_lattice_ops/ls_instructions_parse.cpp)
target_link_libraries(lsqecc_tests
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
