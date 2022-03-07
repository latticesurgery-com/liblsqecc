# Liblsqecc

A collection of C++ tools to speed up the [Lattice Surgery Compiler](https://github.com/latticesurgery-com/lattice-surgery-compiler).

Currently, its main goal is to produce a 3D assembly of "slices" representing routed LS Instructions.

#### Structure
This package builds two main targets, an executable and a library.

#### The `lsqecc_slicer` executable

Found at the top level of the build directory. Produces [latticesurgery.com](https://latticesurgery.com) style slices from [LS Instructions](https://github.com/latticesurgery-com/lattice-surgery-compiler/issues/246), using a [layout spec](https://github.com/latticesurgery-com/lattice-surgery-compiler/issues/250).
 
Example usage: 

```shell
lsqecc -i instructions.txt -l 10_by_10_layout.txt -o qft.json -t 5
```

Full usage:
```
Usage: lsqecc_slicer [options...]
Options:
    -i, --instructions     File name of file with LS Instructions (Required)
    -l, --layout           File name of file with layout spec. Defaults to simple layout if none is provided
    -o, --output           File name of output file to which write a latticesurgery.com JSON of the slices
    -t, --timeout          Set a timeout in seconds after which stop producing slices
    -r, --router           Set a router: naive_cached (default), naive
    -h, --help             Shows this page
```

#### The `liblsqecc` library

Contains the functionality used by the `lsqecc_slicer` executable. One day we hope to expose it's functionality as a Python API in the [Lattice Surgery Compiler](https://github.com/latticesurgery-com/lattice-surgery-compiler) package.

#### Build
Clone:
```shell
$ git clone --recursive git@github.com:latticesurgery-com/liblsqecc.git 
```

Standard CMake build:
```shell
$ mkdir build
$ cd build
$ cmake ..
```

###