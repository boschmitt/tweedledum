<p align="center">
  <img src="https://cdn.rawgit.com/boschmitt/tweedledum/master/tweedledum.svg" width="128" height="128" style="margin-right: 12pt"/>
</p>
<p align="center">
  <img src="https://img.shields.io/badge/license-MIT-000000.svg">
  <a href="https://github.com/boschmitt/tweedledum/actions/workflows/macos.yml">
    <img src="https://github.com/boschmitt/tweedledum/workflows/MacOS/badge.svg">
  </a>
  <a href="https://github.com/boschmitt/tweedledum/actions/workflows/ubuntu.yml">
    <img src="https://github.com/boschmitt/tweedledum/workflows/Ubuntu/badge.svg">
  </a>
  <a href="https://github.com/boschmitt/tweedledum/actions/workflows/windows.yml">
    <img src="https://github.com/boschmitt/tweedledum/workflows/Windows/badge.svg">
  </a>
  <a href="https://github.com/boschmitt/tweedledum/actions/workflows/build_wheels.yml.yml">
    <img src="https://github.com/boschmitt/tweedledum/actions/workflows/build_wheels.yml/badge.svg">
  </a>
</p>

    /!\ (Warning) If you have used tweedledum before: the master branch history is broken.
    /!\ The new master branch is a completely rewrite of the library. The old version can be found
    /!\ on **alpha** branch. (Sorry for the inconvenience!!---but it is for a great cause)

**tweedledum** is a library for synthesis, compilation, and optimization of
quantum circuits.  The library is written to be scalable up to problem sizes in
which quantum circuits outperform classical ones. Also, it is meant to be used
both independently and alongside established tools.


Its design is guided by three mantras:

- __Gotta run fast__: execution-time performance is a priority.

- __Your compiler, your rules__.  You know better. At least, Tweedledum 
hopes so! The library provides a standard set of operators that can be easily 
extended (thanks to some type-erasure black magic).  However, the library will 
leave your operators completely alone if you don't write passes that 
specifically manipulate them.  Furthermore, Tweedledum will rarely take any
decision in your behalf, i.e., it does not provide generic methods to optimize
or synthesize circuits, you need to specifically call the algorithms you want.

- __Opinionated, but not stubborn__.  Many passes and synthesis algorithms have
many configuration parameters.  Tweedledum comes with reasonable defaults and 
curated opinions of what value such parameters should take.  But in the end,
it all up to you.

__Corollary__:  Because of it's flexibility, Tweedledum is capable of accepting
gates/operators that are defined as python classes.  Indeed, any pythonic 
framework can use the library as a circuit manager.  Meaning that the library 
can be used to slowly transition the core and performance sensitive parts of a 
pythonic framework to C++, while maintaining the capability of users to develop
passes in python.

# Installation

Tweedledum has two python packages that can be installed using `pip`.  For both,
you will at least __Python 3.6__.  The `tweedledum` package contains the latest
stable release.  You can install it from PyPI using:

* Latest stable release (Linux/Mac/Windows)

```
pip install tweedledum
```

For the developers, users or researchers who are comfortable living on the 
absolute bleeding edge, `tweedledum-dev` contains that latest developments 
merged into the master branch.

* Latest (Linux/Mac/Windows)

```
pip install tweedledum-dev
```

__Warning__: The two packages cannot be installed together.

# Installation from source (Development)

Installing `tweedledum` from the source, instead of using the Python Package 
Index (PyPI) repository version, allows you to extend the latest version of the
code. In the following, I will explain two workflows I personally use for
development. Choose one that best suits your needs.

Alright, both workflows start the same way. You clone the repository:
```
git clone https://github.com/boschmitt/tweedledum.git
```

## C++
The first workflow is pure C++. We start by creating a directory to hold the 
build output:

```
mkdir build      
cd build
```

Note that the library has a directory named `examples/`.  If we set the
``TWEEDLEDUM_EXAMPLES`` CMake variable to ``TRUE``.  Any `.cpp` file in this
directory will be compiled to its own executable. 

So, lets assume you have a file named `hello_world.cpp` in the `examples/`.
First we configure our project and enable the examples:

```
cmake -DTWEEDLEDUM_EXAMPLES=TRUE ..
```

If you are on a \*nix system, you should now see a Makefile in the current
directory. Now you can build the library by running `make`. At this point you
can build the `hello_world` executable by calling

```
make hello_world
```

Once the examples have been built you can run it:

```
./examples/hello_world
```

## C++ and Python

The second workflow is a bit of a hack.  In Python we can install libraries in
editable mode, meaning that code changes to the _Python code_ in the project
don't require a reinstall to be applied. 

If you want to install it in editable mode, you can do this with:
```
pip install -e .
```

The only problem now, is that if we change the C++ code, we will need to 
reinstall the library.  Fortunately, there is a way to circumvent this 
annoyance.  

After installing in editable mode, you will see that in `python/tweedledum/` 
there is a cpython shared library `_tweedledum.cpython-...` Remove this file:

```sh
rm python/tweedledum/_tweedledum.cpython-...
```

Now, we create a build directory as we did with the C++ workflow:
```sh
mkdir build      
cd build
```

We can manually build the cpython shared library using:
```sh
make _tweedledum
```

This will create the library in the `build/` directory. Now, all we need to
create a symbolic link in `python/tweedledum/` that points the library in 
`build/`:

```sh
ln -s _tweedledum.cpython-39-darwin.so ../python/tweedledum/
```

Now, whenever we change the C++ code and rebuild the python library, the changes
won't require a reinstall of the library to be available.

# Used third-party tools

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

## License

This software is licensed under the MIT licence (see 
[LICENSE](https://github.com/boschmitt/tweedledum/blob/master/LICENSE)).

## EPFL logic synthesis libraries

tweedledum is part of the
[EPFL logic synthesis](https://lsi.epfl.ch/page-138455-en.html) libraries.
The other libraries and several examples on how to use and integrate the 
libraries can be found in the 
[logic synthesis tool showcase](https://github.com/lsils/lstools-showcase).
