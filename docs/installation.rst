Installation
============

tweedledum is a header-only C++-17 library. Just add the include directory of tweeldedum to your
include directories, and you can integrate it into your source files using

.. code-block:: c++

  #include <tweedledum/tweedledum.hpp>

Requirements
------------

We tested building tweedledum on Mac OS and Linux using:

* Clang 6.0.0
* Clang 7.0.0
* GCC 7.3.0
* GCC 8.1.0.

If you experience that the system compiler does not suffice the requirements, you can manually
pass a compiler to CMake using::

  cmake -DCMAKE_CXX_COMPILER=/path/to/c++-compiler ..

Building the examples
---------------------

The included `CMake build script` can be used to build the tweedledum library examples on a wide
range of platforms. CMake is freely available for download from http://www.cmake.org/download/.

CMake works by generating native makefiles or project files that can be used in the compiler
environment of your choice. The typical workflow starts with::

  mkdir builds      # Create a directory to hold the build output.
  cd build

To build the `examples` set the ``TWEEDLEDUM_EXAMPLES`` CMake variable to ``TRUE``::

  cmake -DTWEEDLEDUM_EXAMPLES=TRUE <path/to/tweedledum>

where :file:`{<path/to/tweedledum>}` is a path to the ``tweedledum`` repository. 

If you are on a \*nix system, you should now see a Makefile in the current directory. Now you can
build the library by running :command:`make`.

All :file:`*.cpp` files in the :file:`{examples/}` directory will be compiled to its own executable
which will have the same name. For example, the file :file:`examples/hello_world.cpp` will generate
the executable :file:`hello_world`.

Once the examples have been built you can invoke :command:`./examples/<name>` to run it::

  ./examples/hello_world


Building the documentation
--------------------------

To build the documentation you need the following software installed on your system:

* `Python <https://www.python.org/>`_ with pip and virtualenv
* `Doxygen <http://www.stack.nl/~dimitri/doxygen/>`_

First generate makefiles or project files using CMake as described in the previous section.
Then compile the ``doc`` target/project, for example::

  make doc

This will generate the HTML documentation in ``doc/html``.
