# Liblsqecc

Home of a set of fast tools for compiling lattice surgery instructions. Part of the [Lattice Surgery Compiler](https://github.com/latticesurgery-com/lattice-surgery-compiler) family.

![](https://user-images.githubusercontent.com/36427091/193476068-eddfea28-3d91-4398-8de4-3a55bb43faa7.gif)

## Targets
### The `lsqecc_slicer` executable

Found at the top level of the build directory. Produces [latticesurgery.com](https://latticesurgery.com) style slices from [LS Instructions](https://github.com/latticesurgery-com/lattice-surgery-compiler/issues/246), using a [layout spec](https://github.com/latticesurgery-com/lattice-surgery-compiler/issues/250).
 
Example usage: 

```shell
# Basic example of using LLI instructions generated from LSC
lsqecc_slicer -i instructions.txt -l 10_by_10_layout.txt -o output.json

# Litinski's compact layout
lsqecc_slicer -q examples/qasm/compact_layout_demo.qasm -o out.json --compactlayout --graceful
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
    -l, --layout           File name of file with layout spec. Defaults to simple layout if none is provided
    -o, --output           File name of output. When not provided outputs to stdout
    -f, --output-format    Requires -o, STDOUT output format: progress, noprogress, machine
    -t, --timeout          Set a timeout in seconds after which stop producing slices
    -r, --router           Set a router: graph_search (default), graph_search_cached
    -g, --graph-search     Set a graph search provider: custom (default), boost (not always available)
    --graceful             If there is an error when slicing, print the error and terminate
    --printlli             Output LLI instead of JSONs
    --noslices             Do the slicing but don't write the slices out
    --cnotcorrections      Add Xs and Zs to correct the the negative outcomes: never (default), always
    --compactlayout        Uses Litinski's compact layout, incompatible with -l
    -h, --help             Shows this page           
```
#### QASM Support (Highly experimental)
LibLSQECC can parse a small subset of OpenQASM 2.0 instead of LLI, with restrictions below. We call this type of assembly OpenQASM--. In general OpenQASM-- should be valid OpenQASM, up to implementation defects. The rules are 
 * No classical control
 * Only one register is allowed (whether the names match will not be checked)
 * Max one gate per line, with only inline comments
 * Single qubit gates must be in the form `g q[n];` where `g` is one of `h`,`x`,`z`,`s`,`t` and `n` is a non-negative integer
 * CNOTs must be in the form `cx q[n],q[m];` where `n` and `m` are non-negative. Target comes first, as per [OpenQASM convention (Fig 2)](https://arxiv.org/pdf/1707.03429.pdf).
 * Supports some basic annotations such as: `cx q[0],q[7]; // %ZXWithMBMTargetFirst,AncillaNextToTarget`

 * Working on adding support for `rz` and `crz`. Needs integration with a Solovay-Kitaev decomposer.

### The `liblsqecc` library

Contains the functionality used by the `lsqecc_slicer` executable. We are working on exposing its functionality as a Python API in the [Lattice Surgery Compiler](https://github.com/latticesurgery-com/lattice-surgery-compiler) package.

### Build
Clone:
```shell
$ git clone --recursive git@github.com:latticesurgery-com/liblsqecc.git 
```

Install the [Boost](https://www.boost.org/) development headers for your platform.

Standard CMake build:
```shell
$ mkdir build
$ cd build
$ cmake ..
```

The `lsqecc_slicer` executable will be at the top level of the `build` directory.
