<img src="https://cdn.rawgit.com/boschmitt/tweedledum/master/tweedledum.svg" width="64" height="64" align="left" style="margin-right: 12pt"/>

# Tweedledum

A C++-17 header-only library for synthesizing, manipulating, and optimizing
quantum circuits.

The library is written to be scalable up to problem sizes in which quantum
circuits outperform classical ones. Also, it is meant to be used both
independently and alongside established tools for example compilers or more
general and high level frameworks, e.g., IBM's QISKit, Rigetti's Forest,
ProjectQ.

**NOTE: The library is currently going through a major redesign**.  It is fair
to say that it would be a completely new library by the time I am done with it,
but I like the name.

The idea is to make it much more extensible.  User will be able to come up with
their own gates and mix them with the ones provided by the library.  A circuit
will be able to be a mixture of gates, unitary matrices, other circuits, and
whatever you feel like adding to it.  A truly multilevel representation. 

## Disclaimer

**Tweedledum is in version *Alpha***.  Hence, the software is still under active
development and not feature complete, meaning the API is subject to big
changes.  This is released for developers or users who are comfortable living on
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
