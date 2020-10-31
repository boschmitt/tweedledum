[![License](https://img.shields.io/badge/license-MIT-000000.svg)](https://opensource.org/licenses/MIT)

<img src="https://cdn.rawgit.com/boschmitt/tweedledum/master/tweedledum.svg" width="64" height="64" style="margin-right: 12pt"/>

**tweedledum** is a library for synthesis, compilation, and optimization of
quantum circuits.  The library is written to be scalable up to problem sizes in
which quantum circuits outperform classical ones. Also, it is meant to be used
both independently and alongside established tools.

## Used third-party tools

The library it is built, tested, bind to python, and whatnot using a many
third-party tools and services. Thanks a lot!

- [**Catch2**](https://github.com/catchorg/Catch2) test framework for unit-tests, TDD and BDD
- [**CMake**](https://cmake.org) for build automation
- [**Eigen**](https://gitlab.com/libeigen/eigen) template library for linear algebra
- [**{fmt}**](https://github.com/fmtlib/fmt) - A modern formatting library
- [**kitty**](https://github.com/msoeken/kitty) - truth table library 
- [**nlohmann/json**](https://github.com/nlohmann/json) - JSON for Modern C++
- [**pybind11**](https://github.com/pybind/pybind11) - Seamless operability between C++11 and Python

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
