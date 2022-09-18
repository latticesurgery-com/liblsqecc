cmake_minimum_required(VERSION 3.5)


project(lsqecc)


####################################################
# General configuration

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfatal-errors -Wall -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-O0 -g3 -DNDEBUG -D_LIBCPP_DEBUG=1 ")
set(CMAKE_CXX_FLAGS_RELEASE_INIT  "-O3")
set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_BUILD_TYPE Debug)

if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    message("Setting stdlib=libc++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
endif ()


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
        src/gates/parse_gates.cpp
        src/patches/sparse_patch_computation.cpp
        src/ls_instructions/ls_instructions.cpp
        src/ls_instructions/ls_instructions_parse.cpp
        src/ls_instructions/ls_instruction_stream.cpp
        src/ls_instructions/parse_utils.cpp
        src/patches/patches.cpp
        src/patches/sparse_slice.cpp
        src/patches/slices_to_json.cpp
        src/layout/graph_search/boost_based_graph_search.cpp
        src/layout/graph_search/custom_graph_search.cpp
        src/layout/router.cpp
        src/layout/ascii_layout_spec.cpp
        src/layout/layout.cpp
        src/pipelines/slicer.cpp
        src/gates/gates.cpp
        src/patches/dense_slice.cpp
        src/patches/dense_patch_computation.cpp
        src/ls_instructions/ls_instructions_from_gates.cpp src/layout/dynamic_layouts/compact_layout.cpp)

set(LSQECCLIB_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include")

target_include_directories(
        lsqecclib
        PUBLIC
        ${LSQECCLIB_INCLUDE_DIRS}
)

set_property(TARGET lsqecclib PROPERTY POSITION_INDEPENDENT_CODE ON)


###################################################
# Command line executable
if(NOT DEFINED CMAKE_CROSSCOMPILING_EMULATOR)

    add_executable(
            lsqecc_slicer
            src/lsqecc_slicer_main.cpp)

    target_link_libraries(
            lsqecc_slicer PUBLIC lsqecclib
    )

endif()

###################################################
# Emscripten interface

if(DEFINED CMAKE_CROSSCOMPILING_EMULATOR)
    
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")
    
    add_executable(
            lsqecc_emscripten
            src/emscripten_bindings.cpp
            src/pipelines/slicer.cpp)

    target_link_libraries(
            lsqecc_emscripten PUBLIC lsqecclib
    )

    target_include_directories(
            lsqecc_emscripten
            PUBLIC
            #TODO
    )
    
    set_target_properties(lsqecclib PROPERTIES LINK_FLAGS "-fexceptions -s MODULARIZE=1 -s TOTAL_MEMORY=1024MB -s DEMANGLE_SUPPORT=1  --bind")
    set_target_properties(lsqecc_emscripten PROPERTIES LINK_FLAGS "-fexceptions -s MODULARIZE=1 -s TOTAL_MEMORY=1024MB -s DEMANGLE_SUPPORT=1 -s EXPORT_NAME='LsqeccModule' --bind")

    #target_link_options(lsqecclib PUBLIC "-s TOTAL_MEMORY=1024MB ")
    #target_link_options(lsqecc_emscripten PUBLIC "-s TOTAL_MEMORY=1024MB --bind")

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

endif()
