# Liblsqecc

[![Build & Tests](https://github.com/latticesurgery-com/liblsqecc/actions/workflows/build_and_test.yml/badge.svg)](https://github.com/latticesurgery-com/liblsqecc/actions/workflows/build_and_test.yml)
[![Unitary Fund](https://img.shields.io/badge/Supported%20By-Unitary%20Fund-FFFF00.svg)](https://unitary.fund)
[![arXiv](https://img.shields.io/badge/arXiv-2302.02459-b31b1b.svg)](https://arxiv.org/abs/2302.02459)

[![Lattice Surgery Compiler-fin-01](https://user-images.githubusercontent.com/46719079/150657000-8e83c649-84a8-431b-aab0-d44d847e5a24.png)](https://latticesurgery.com)

![](https://user-images.githubusercontent.com/36427091/193476068-eddfea28-3d91-4398-8de4-3a55bb43faa7.gif)

Home of a set of fast tools for compiling lattice surgery instructions. Part of the [Lattice Surgery Compiler](https://github.com/latticesurgery-com/lattice-surgery-compiler) family.

## Targets
### The `lsqecc_slicer` executable

Found at the top level of the build directory. Produces [latticesurgery.com](https://latticesurgery.com) style slices from [LS Instructions](https://github.com/latticesurgery-com/lattice-surgery-compiler/issues/246), using a [layout spec](https://github.com/latticesurgery-com/lattice-surgery-compiler/issues/250).
 
Example usage: 

```shell
# Basic example of using LLI instructions generated from LSC
lsqecc_slicer -i instructions.txt -l 10_by_10_layout.txt -o output.json

# Litinski's compact layout
lsqecc_slicer -q -i examples/qasm/compact_layout_demo.qasm -o out.json --compactlayout --graceful
```
Where:
 * `instructions.txt` contains [LS Instructions](https://github.com/latticesurgery-com/lattice-surgery-compiler/issues/246)
 * `10_by_10_layout.txt` is a [layout spec](https://github.com/latticesurgery-com/lattice-surgery-compiler/issues/250) file.
 * `output.json` is a file containing the 3D assembly of slices. Those can just be uploaded [latticesurgery.com](https://latticesurgery.com) for viewing

Full usage:
```
Usage: lsqecc_slicer [options...]
Options:
    -i, --input            File with input. If not provided will read LS Instructions from stdin
    -q, --qasm             File name of file with QASM. When not provided will read as LLI (not QASM)
    -l, --layout           File name of file with layout spec, otherwise the layout is auto-generated (configure with -L)
    -o, --output           File name of output. When not provided outputs to stdout
    -f, --output-format    Requires -o, STDOUT output format: progress, noprogress, machine
    -t, --timeout          Set a timeout in seconds after which stop producing slices
    -r, --router           Set a router: graph_search (default), graph_search_cached
    -g, --graph-search     Set a graph search provider: custom (default), boost (not always available)
    --graceful             If there is an error when slicing, print the error and terminate
    --printlli             Output LLI instead of JSONs
    --noslices             Do the slicing but don't write the slices out
    --cnotcorrections      Add Xs and Zs to correct the the negative outcomes: never (default), always
    --layoutgenerator, -L  Automatically generates a layout for the given number of qubits. Incompatible with -l. Options:
                            - compact (default): Uses Litinski's Game of Surace Code compact layout (https://arxiv.org/abs/1808.02892)
                            - edpc: Uses a layout specified in the EDPC paper by Beverland et. al. (https://arxiv.org/abs/2110.11493)
    --nostagger            Turns off staggered distillation block timing
    -h, --help             Shows this page
```
#### QASM Support (Experimental)
LibLSQECC can parse a small subset of OpenQASM 2.0 instead of LLI, with restrictions below. We call this type of assembly OpenQASM--. In general OpenQASM-- should be valid OpenQASM, up to implementation defects. The rules are 
 * No classical control
 * Only one register is allowed (whether the names match will not be checked)
 * Max one gate per line, with only inline comments
 * Single qubit gates must be in the form `g q[n];` where `g` is one of `h`,`x`,`z`,`s`,`t` and `n` is a non-negative integer
 * CNOTs must be in the form `cx q[n],q[m];` where `n` and `m` are non-negative. Target comes first, as per [OpenQASM convention (Fig 2)](https://arxiv.org/pdf/1707.03429.pdf).
 * `rz(expr)` and `crz(expr)` where `expr` has form `pi/m` or `n*pi/m` for n,m integers. No whitespace.
 * Supports some basic annotations such as: `cx q[0],q[7]; // %ZXWithMBMTargetFirst,AncillaNextToTarget`
 * Program must begin with `OPENQASM 2.0;` in the first line

### The `liblsqecc` library

Contains the functionality used by the `lsqecc_slicer` executable. We are working on exposing its functionality as a Python API in the [Lattice Surgery Compiler](https://github.com/latticesurgery-com/lattice-surgery-compiler) package.

### Build
Clone:
```shell
$ git clone --recursive git@github.com:latticesurgery-com/liblsqecc.git 
```

Install the [Boost](https://www.boost.org/) development headers for your platform. (optional, used for faster path finding in some cases)

Standard CMake build:
```shell
$ mkdir build
$ cd build
$ cmake ..
```

The `lsqecc_slicer` executable will be at the top level of the `build` directory.

#### Building with Gridsynth support for approximating rz angles

[Gridsynth](https://www.mathstat.dal.ca/~selinger/newsynth/) (aka Newsynth) is a Haskell program that can approximate arbitrary rotations with Clifford gates and T gates.

We can take advantage of gridsynth's functionality to decompose arbitrary rz's in our input circuits. To do so we've created a [fork of Gridsynth](https://github.com/latticesurgery-com/rotation-decomposer/tree/main/newsynth) with a C API wrapper do that we can call Gridsynth from our C++ code.

However due to the Haskell platform's own portability challenges and some low level interfacing with C and C++ being required, at this point enabling Gridsynth is still manual:

 1. Go to `external/rotation/decomposer/newsynth`
 2. Get a Haskell environment compatible with the dependencies in `newsynth.cabal`
    * I used a user-level install of ghc-9.4.4 with [ghcup](https://www.haskell.org/ghcup/). The `installghc` target in the`Makefile` in the `newsynth` folder contains the ghcup commands I used to create my build.
    * Other ways are probably possible with Cabal or Stack
 3. Use `make buildhs` in the `newsynth` folder folder to build gridsynth with the C API
    * Likeley to require some manual intevention like adjusting some versions and paths
    * Can use the `make runtests` to verify a succesful build of Gridsynth and the C API
4. Configure the CMake project with the `USE_GRIDSYNTH` variable defined (E.g. `cmake .. -DUSE_GRIDSYNTH:STRING=YES`)
    * If something goes wrong, the makefile in the `newsynth` disectory is readble and shows what's required to run lsqecc with gridsynth
5. Run lsqecc_slicer as usual. RZ and CRZ gates will be approximated using gridsynth where applicable
    * can use the `--rzprecision` to set gridsynth's precision epsilon=10^(-rzprecision) (Equivalent to gridsynth's default precsion mode `-d`)

## Contributors

Liblsqecc was primarily developed at Aalto University by [George Watkins](https://github.com/gwwatkin) under Dr. [Alexandru Paler](https://github.com/alexandrupaler)'s supervision, and is now maintained by George Watkins.

A special thanks to [Tyler LeBlond](https://github.com/tylerrleblond) for adding the EDPC layout and other contributions.
