<p align="center">
  <img src="https://cdn.rawgit.com/boschmitt/tweedledum/master/tweedledum.svg" width="128" height="128" style="margin-right: 12pt"/>
</p>
<p align="center">
  <img src="https://img.shields.io/badge/license-MIT-000000.svg">
  <img src="https://github.com/boschmitt/tweedledum/workflows/MacOS/badge.svg">
  <img src="https://github.com/boschmitt/tweedledum/workflows/Ubuntu/badge.svg">
  <img src="https://github.com/boschmitt/tweedledum/workflows/Windows/badge.svg">
  <img src="https://github.com/boschmitt/tweedledum/workflows/Build%20wheels/badge.svg?event=release">
</p>

    /!\ (Warning) If you have used tweedledum before: the master branch history is broken.
    /!\ The new master branch is a completely rewrite of the library. The old version can be found
    /!\ on **alpha** branch. (Sorry for the inconvenience!!---but it is for a great cause)

**tweedledum** is a library for synthesis, compilation, and optimization of
quantum circuits.  The library is written to be scalable up to problem sizes in
which quantum circuits outperform classical ones. Also, it is meant to be used
both independently and alongside established tools.


Its design is guided by three mantras:

- Gotta run fast: run-time performance is a priority.
- Opinionated, but not stubborn.
- Your system, your rules.

## Used third-party tools

The library it is built, tested, bind to python, and whatnot using many
third-party tools and services. Thanks a lot!

- [**abc**](https://github.com/berkeley-abc/abc) - ABC: System for Sequential Logic Synthesis and Formal Verification
- [**bill**](https://github.com/lsils/bill) - C++ header-only reasoning library
- [**Catch2**](https://github.com/catchorg/Catch2) test framework for unit-tests, TDD and BDD
- [**CMake**](https://cmake.org) for build automation
- [**Eigen**](https://gitlab.com/libeigen/eigen) template library for linear algebra
- [**{fmt}**](https://github.com/fmtlib/fmt) - A modern formatting library
- [**kitty**](https://github.com/msoeken/kitty) - truth table library 
- [**lorina**](https://github.com/hriener/lorina) - C++ parsing library for simple formats used in logic synthesis and formal verification 
- [**mockturtle**](https://github.com/lsils/mockturtle) - C++ logic network library
- [**nlohmann/json**](https://github.com/nlohmann/json) - JSON for Modern C++
- [**parallel_hashmap**](https://github.com/greg7mdp/parallel-hashmap) - A family of header-only, very fast and memory-friendly hashmap and btree containers.
- [**percy**](https://github.com/lsils/percy) - C++ header-only exact synthesis library
- [**pybind11**](https://github.com/pybind/pybind11) - Seamless operability between C++11 and Python
- [**rang**](https://github.com/agauniyal/rang) - A Minimal, Header only Modern c++ library for terminal goodies

## Beta disclaimer

**Tweedledum is in version *Beta***.  Hence, the software is still under active
development and not feature complete, meaning the API is subject to changes.
This is released for developers or users who are comfortable living on
the absolute bleeding edge.

## License

This software is licensed under the MIT licence (see 
[LICENSE](https://github.com/boschmitt/tweedledum/blob/master/LICENSE)).

## EPFL logic synthesis libraries

tweedledum is part of the
[EPFL logic synthesis](https://lsi.epfl.ch/page-138455-en.html) libraries.
The other libraries and several examples on how to use and integrate the 
libraries can be found in the 
[logic synthesis tool showcase](https://github.com/lsils/lstools-showcase).
