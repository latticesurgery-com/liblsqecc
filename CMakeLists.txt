cmake_minimum_required(VERSION 3.12)


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
        src/gates/gates.cpp
        src/gates/gate_approximator.cpp
        src/gates/clifford_plus_t_conversion_stream.cpp
        src/patches/dense_slice.cpp
        src/patches/dense_patch_computation.cpp
        src/patches/patches.cpp
        src/patches/sparse_slice.cpp
        src/patches/slices_to_json.cpp
        src/pipelines/slicer.cpp
        src/layout/dynamic_layouts/compact_layout.cpp
	src/layout/dynamic_layouts/edpc_layout.cpp
        src/layout/dynamic_layouts/determine_exposed_operators.cpp
        src/layout/graph_search/boost_based_graph_search.cpp
        src/layout/graph_search/custom_graph_search.cpp
        src/layout/router.cpp
        src/layout/ascii_layout_spec.cpp
        src/layout/layout.cpp
        src/ls_instructions/boundary_rotation_injection_stream.cpp
        src/ls_instructions/ls_instructions_from_gates.cpp
        src/ls_instructions/ls_instructions_parse.cpp
        src/ls_instructions/ls_instruction_stream.cpp
        src/ls_instructions/ls_instructions.cpp
        src/ls_instructions/parse_utils.cpp
        src/ls_instructions/teleported_s_gate_injection_stream.cpp
       )

set(LSQECCLIB_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include")

target_include_directories(
    lsqecclib
    PUBLIC
    ${LSQECCLIB_INCLUDE_DIRS}
)

set_property(TARGET lsqecclib PROPERTY POSITION_INDEPENDENT_CODE ON)


if(USE_GRIDSYNTH) 
    message("USE_GRIDSYNTH defined, building with it")

    set(GHC_VERSION 9.4.4)
    set(CABAL_VERSION 3.8.1.0)
    set(HASKELL_RTS_VERSION 1.0.2)
    
    
    execute_process(COMMAND $ENV{HOME}/.ghcup/bin/ghc --print-libdir OUTPUT_VARIABLE GHC_LIB_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
    message(GHC_LIB_DIR: ${GHC_LIB_DIR})
    
    add_compile_definitions(USE_GRIDSYNTH)
    
    set(RTS_LIBS_DIR ${GHC_LIB_DIR}/x86_64-linux-ghc-${GHC_VERSION}/)
    message(RTS_LIBS_DIR: ${RTS_LIBS_DIR})
    
    set(GRIDSYNTH_LIBS_DIR ${PROJECT_SOURCE_DIR}/external/rotation-decomposer/newsynth/)
    message(GRIDSYNTH_LIBS_DIR: ${GRIDSYNTH_LIBS_DIR})
    
    target_include_directories(
            lsqecclib
            PUBLIC AFTER
            ${GRIDSYNTH_LIBS_DIR}/include
            ${GHC_LIB_DIR}/x86_64-linux-ghc-${GHC_VERSION}/rts-${HASKELL_RTS_VERSION}/include # for HsFFI.h
    )
    
    
    target_link_options(lsqecclib INTERFACE
            -L${GRIDSYNTH_LIBS_DIR} -lgridsynth_ccals -lHSnewsynth-0.4.0.0-inplace-ghc${GHC_VERSION}
            -L${GHC_LIB_DIR}/x86_64-linux-ghc-${GHC_VERSION}/ -lHSrts-${HASKELL_RTS_VERSION}_debug-ghc${GHC_VERSION}
            -Wl,-rpath,${GRIDSYNTH_LIBS_DIR}
            -Wl,-rpath,${RTS_LIBS_DIR}
    )
else()
   message("USE_GRIDSYNTH not defined, building without Gridsynth")
endif()

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



if(NOT DEFINED CMAKE_CROSSCOMPILING_EMULATOR)

    ###################################################
    # Tests
    add_subdirectory(external/googletest)


    enable_testing()


    # lib
    add_executable(
            lsqecc_tests
            tests/main.cpp
            tests/lstk/lstk.cpp
            tests/gates/gate_approximator.cpp
            tests/gates/parse_gates.cpp
    )

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

    add_custom_command(
            TARGET lsqecc_tests
            COMMENT "Run lsqecc_tests"
            POST_BUILD
            COMMAND ${PROJECT_BINARY_DIR}/lsqecc_tests
    )

endif()
