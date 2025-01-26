# Liblsqecc

[![Build & Tests](https://github.com/latticesurgery-com/liblsqecc/actions/workflows/build_and_test.yml/badge.svg)](https://github.com/latticesurgery-com/liblsqecc/actions/workflows/build_and_test.yml)
[![Unitary Fund](https://img.shields.io/badge/Supported%20By-Unitary%20Fund-FFFF00.svg)](https://unitary.fund)
[![arXiv](https://img.shields.io/badge/arXiv-2302.02459-b31b1b.svg)](https://arxiv.org/abs/2302.02459)

[![Lattice Surgery Compiler-fin-01](https://user-images.githubusercontent.com/46719079/150657000-8e83c649-84a8-431b-aab0-d44d847e5a24.png)](https://latticesurgery.com)

![](https://user-images.githubusercontent.com/36427091/193476068-eddfea28-3d91-4398-8de4-3a55bb43faa7.gif)

[Publication link for original compiler release](https://quantum-journal.org/papers/q-2024-05-22-1354/)
[Publication link for upgraded compiler and its usage in resource estimation](https://dl.acm.org/doi/abs/10.1145/3689826)

Home of a set of fast tools for compiling lattice surgery instructions. Part of the [Lattice Surgery Compiler](https://github.com/latticesurgery-com/lattice-surgery-compiler) family. The `liblsqecc` library contains the functionality used by the `lsqecc_slicer` executable. We are working on exposing its functionality as a Python API in the [Lattice Surgery Compiler](https://github.com/latticesurgery-com/lattice-surgery-compiler) package.

## Build
```shell
git clone --recursive https://github.com/latticesurgery-com/liblsqecc.git
cd liblsqecc
mkdir build
cd build
cmake ..
```

**Note**: The `lsqecc_slicer` executable will be at the top level of the `build` directory.

**Optional**: Install the [Boost](https://www.boost.org/) development headers for your platform. These are used for faster path finding in some cases.

## Using `lsqecc_slicer`

Found at the top level of the build directory. Produces [latticesurgery.com](https://latticesurgery.com) style slices from [LS Instructions](https://github.com/latticesurgery-com/lattice-surgery-compiler/issues/246), using a [layout spec](https://github.com/latticesurgery-com/lattice-surgery-compiler/issues/250).
 
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

### Parameters

```
Usage: lsqecc_slicer [options...]
Options:
    -i, --input            File with input. If not provided will read LS Instructions from stdin
    -q, --qasm             File name of file with QASM. When not provided will read as LLI (not QASM)
    -l, --layout           File name of file with layout spec, otherwise the layout is auto-generated (configure with -L)
    -o, --output           File name of output. When not provided outputs to stdout
    -f, --output-format    Requires -o, STDOUT output format: progress, noprogress, machine, stats
    -t, --timeout          Set a timeout in seconds after which stop producing slices
    -r, --router           Set a router: graph_search (default), graph_search_cached
    -P, --pipeline         pipeline mode: stream (default), wave, edpc, dag (deprecated)
    -g, --graph-search     Set a graph search provider: djikstra (default), astar, boost (not always available)
    --graceful             If there is an error when slicing, print the error and terminate
    --printlli             Output LLI instead of JSONs. options: before (default), sliced (prints lli on the same slice separated by semicolons)
    --printdag             Prints a dependency dag of the circuit. Modes: input (default), processedlli
    --noslices             Do the slicing but don't write the slices out
    --cnotcorrections      Add Xs and Zs to correct the the negative outcomes: never (default), always
    --layoutgenerator, -L  Automatically generates a layout for the given number of qubits. Incompatible with -l. Options:
                            - compact (default): Uses Litinski's Game of Surace Code compact layout (https://arxiv.org/abs/1808.02892)
                            - compact_no_clogging: same as compact, but fewer cells for ancillas and magic state queues
                            - edpc: Uses a family of layouts based upon the one specified in the EDPC paper by Beverland et. al. (https://arxiv.org/abs/2110.11493)
    --numlanes             Only compatible with -L edpc. Configures number of free lanes for routing.
    --condensed            Only compatible with -L edpc. Packs logical qubits more compactly.
    --explicitfactories    Only compatible with -L edpc. Explicitly specifies factories but clogs easily (otherwise, uses tiles reserved for magic state re-spawn).
    --nostagger            Turns off staggered distillation block timing
    --disttime             Set the distillation time (default 10)
    --local                Compile gates into a pair-wise local lattice surgery instruction set
    --notwists             Compile S gates using the catalytic teleportation circuit from Fowler, 2012 instead of using the twist-based Y state initialization and teleportation from Gidney, 2024
    -h, --help             Shows this page  
```
### OpenQASMmin: a OpenQASM dialect (Experimental)

LibLSQECC can parse a small subset of OpenQASM 2.0 instead of LLI. We call this type of assembly OpenQASMmin.

In general, OpenQASMmin should be valid OpenQASM, with the restrictions below:
 * Program must begin with `OPENQASM 2.0;` in the first line
 * Only one register is allowed (whether the names match will not be checked)
 * Max one gate per line
 * Single qubit gates must be in the form `g q[n];` where `g` is one of `h`,`x`,`z`,`s`,`t`, `sdg`, `tdg`, `reset` and `n` is a non-negative integer
 * `rz(expr)` and `crz(expr)` where `expr` has form `pi/m` or `n*pi/m` for n, m integers. No whitespace.
 * CNOTs must be in the form `cx q[n],q[m];` where `n` and `m` are non-negative. Target comes first, as per [OpenQASM convention (Fig 2)](https://arxiv.org/pdf/1707.03429.pdf)
 * No classical control is supported
 * No measurement operators are supported
 * Only inline comments, for example: `cx q[0],q[7]; // %ZXWithMBMTargetFirst,AncillaNextToTarget`
 

### Building with Gridsynth support for approximating rz angles

Our OpenQASM dialect, OpenQASMMin, does not support arbitrary single qubit rotations. These need to be decomposed into supported gates. [Gridsynth](https://www.mathstat.dal.ca/~selinger/newsynth/) (aka Newsynth) is a Haskell program that can approximate arbitrary rotations with Clifford+T gates.

We can take advantage of Gridsynth's functionality to decompose arbitrary `rz`'s in our input circuits. To do so we've created a [fork of Gridsynth](https://github.com/latticesurgery-com/rotation-decomposer/tree/main/newsynth) with a C API wrapper do that we can call Gridsynth from our C++ code.

However due to the Haskell platform's own portability challenges and some low level interfacing with C and C++ being required, at this point enabling Gridsynth is still manual:

 1. Go to `external/rotation/decomposer/newsynth`
 2. Get a Haskell environment compatible with the dependencies in `newsynth.cabal`
    * Use a user-level install of ghc-9.4.4 with [ghcup](https://www.haskell.org/ghcup/)
    * The `installghc` target in the`Makefile` in the `newsynth` folder contains the ghcup commands used to build
 3. Use `make buildhs` in the `newsynth` folder folder to build Gridsynth with the C API
    * Likeley to require some manual intevention like adjusting some versions and paths
    * Can use the `make runtests` to verify a succesful build of Gridsynth and the C API
4. Configure the CMake project with the `USE_GRIDSYNTH` variable defined (E.g. `cmake .. -DUSE_GRIDSYNTH:STRING=YES`)
    * If something goes wrong, the Makefile in the `newsynth` directory is readable and shows what's required to run `lsqecc` with Gridsynth
5. Run `lsqecc_slicer` as usual
    * `rz` and `crz` gates will be approximated using Gridsynth where applicable
    * can use the `--rzprecision` to set Gridsynth's precision `epsilon=10^(-rzprecision)` (equivalent to Gridsynth's default precision mode `-d`)

**Note**: Other ways are probably possible with Cabal or Stack

### Realistic Resource Estimates using Direct Clifford+T Compilation
![LSC2](https://github.com/latticesurgery-com/liblsqecc/assets/12632882/4e9a83c5-2933-4edf-9ad4-fbe63791aab9)

To generate results according to the compilation scheme written about in [our recent paper](https://arxiv.org/abs/2311.10686), use the following options:

``` shell
lsqecc_slicer -q -i {qasm_filename} -L edpc --disttime 1 --nostagger --notwists --local -P wave --printlli sliced -o {lli_filename} -f stats > {stats_filename}
```

Results in that paper were generated using [PR #106](https://github.com/latticesurgery-com/liblsqecc/pull/106), and should be reproducible using the current release.

### Example for compiling a simple T gate on a minimal layout
``` shell
./lsqecc_slicer -i ../examples/TgateInstructions.txt -l ../examples/minimal_layout.txt -o n_output.json
```

# Contributors
Liblsqecc was originally developed at Aalto University by [George Watkins](https://github.com/gwwatkin) under [Alexandru Paler](https://github.com/alexandrupaler)'s supervision and was later upgraded by [Tyler LeBlond (Oak Ridge National Laboratory)](https://github.com/tylerrleblond) and [Christopher Dean (Dalhousie University)](https://github.com/christopherjdean) in collaboration with [George Watkins](https://github.com/gwwatkin). The compiler is currently maintained primarily by [Tyler LeBlond](https://github.com/tylerrleblond) and ongoing development is co-led with [Alexandru Paler](https://github.com/alexandrupaler).

[Alex Nguyen](https://github.com/alexnguyenn) maintains the NPM package and associated infrastructure.

# Citing
The original release of the compiler should be cited as follows:
```
@article{watkins2024high,
  title={A high performance compiler for very large scale surface code computations},
  author={Watkins, George and Nguyen, Hoang Minh and Watkins, Keelan and Pearce, Steven and Lau, Hoi-Kwan and Paler, Alexandru},
  journal={Quantum},
  volume={8},
  pages={1354},
  year={2024},
  publisher={Verein zur F{\"o}rderung des Open Access Publizierens in den Quantenwissenschaften}
}
```

The upgraded compiler and its usage in resource estimation should be cited as follows:
```
@article{leblond2023realistic,
  title={Realistic Cost to Execute Practical Quantum Circuits using Direct Clifford+ T Lattice Surgery Compilation},
  author={LeBlond, Tyler and Dean, Christopher and Watkins, George and Bennink, Ryan},
  journal={ACM Transactions on Quantum Computing},
  year={2023},
  publisher={ACM New York, NY}
}
```
