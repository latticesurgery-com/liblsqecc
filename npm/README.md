# NPM distribution of the the Lattice Surgery Compiler's C++ edition

[![arXiv](https://img.shields.io/badge/arXiv-2302.02459-b31b1b.svg)](https://arxiv.org/abs/2302.02459)
[![Unitary Fund](https://img.shields.io/badge/Supported%20By-Unitary%20Fund-FFFF00.svg)](https://unitary.fund)

[![Lattice Surgery Compiler-fin-01](https://user-images.githubusercontent.com/46719079/150657000-8e83c649-84a8-431b-aab0-d44d847e5a24.png)](https://latticesurgery.com)

This package allows calling the Lattice Surgery Compiler's C++ edition, aka [Liblsqecc](https://github.com/latticesurgery-com/liblsqecc) from Javascript.

## Background on liblsqecc

The Liblsqecc package is part of the Lattice Surgery compiler project. It's a tool for quantum error correction, that translates an arbitrary quantum circuit into surface code operations based on lattice surgery. These instruction are expressed as a 3D array in JSON format. A viewer is available [here](https://latticesurgery.com/online-compiler). 

![](https://user-images.githubusercontent.com/36427091/193476068-eddfea28-3d91-4398-8de4-3a55bb43faa7.gif)

Read more about the project in [its paper](https://arxiv.org/abs/2302.02459) and on https://latticesurgery.com, or contribute [on github](https://github.com/latticesurgery-com/).


## How to use this package

This package provides an interface to the Liblsqecc's executable target `lsqecc_slicer` though the `Slicer` class. The type interface describes which options are available. 

```typescript
import { Slicer } from "@lattice-surgery/liblsqecc";

const slicer = await Slicer.load();

const result = slicer.run(YOUR_CIRCUIT_AS_STRING, 'qasm');
```

For an overview of what the options do, please consult [executable's readme](https://github.com/latticesurgery-com/liblsqecc#the-lsqecc_slicer-executable).
