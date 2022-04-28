# Liblsqecc

A collection of C++ tools to speed up the [Lattice Surgery Compiler](https://github.com/latticesurgery-com/lattice-surgery-compiler).

### Targets
#### The `lsqecc_slicer` executable

Found at the top level of the build directory. Produces [latticesurgery.com](https://latticesurgery.com) style slices from [LS Instructions](https://github.com/latticesurgery-com/lattice-surgery-compiler/issues/246), using a [layout spec](https://github.com/latticesurgery-com/lattice-surgery-compiler/issues/250).
 
Example usage: 

```shell
lsqecc_slicer -i instructions.txt -l 10_by_10_layout.txt -o output.json
```
Where:
 * `instructions.txt` contains [LS Instructions](https://github.com/latticesurgery-com/lattice-surgery-compiler/issues/246)
 * `10_by_10_layout.txt` is a [layout spec](https://github.com/latticesurgery-com/lattice-surgery-compiler/issues/250) file.
 * `output.json` is a file containing the 3D assembly of slices. Those can just be uploaded [latticesurgery.com](https://latticesurgery.com) for viewing

Full usage:
```
Usage: cmake-build-debug/lsqecc_slicer [options...]
Options:
    -i, --instructions     File name of file with LS Instructions. If not provided will read LS Instructions from stdin
    -q, --qasm             File name of file with QASM. If not provided will read LS Instructions (not QASM) from stdin
    -l, --layout           File name of file with layout spec. Defaults to simple layout if none is provided
    -o, --output           File name of output file to which write a latticesurgery.com JSON of the slices. By default outputs to stdout
    -f, --output-format    Requires, STDOUT output format: slices (default), progress , noprogress, machine,
    -t, --timeout          Set a timeout in seconds after which stop producing slices
    -r, --router           Set a router: graph_search (default), graph_search_cached
    -g, --graph-search     Set a graph search provider: custom (default), boost (not allways available)
    -a, --slice-repr       Set how slices are represented: dense (default), sparse
    --graceful             If there is an error when slicing, print the error and terminate
    -h, --help             Shows this page   
```

#### The `liblsqecc` library

Contains the functionality used by the `lsqecc_slicer` executable. One day we hope to expose it's functionality as a Python API in the [Lattice Surgery Compiler](https://github.com/latticesurgery-com/lattice-surgery-compiler) package.

#### Build
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


###