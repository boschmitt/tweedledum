The tweedledum architecture
=========================

Introduction
------------

The most commonly used notation for representing quantum algorithms is the
quantum circuit model introduced by Deutsch, which describes the computation as
a sequence of elementary quantum logic gates acting on a collection of qubits.
While such representation has the advantage of simplicity, it lacks canonicity,
i.e., there are many different ways of representing a given computation with an
available set of universal elementary operations. Finding an implementation
that uses the fewest resources is not only advantageous but imperative given
the stringent resource constraints in quantum NISQ hardware.

Gates representations (Gates)
-----------------------------

.. todo:: Finish writing

Quantum circuits representations (Networks)
-------------------------------------------

- A **netlist** represents the circuit as a list of gates to be applied
  sequentially. It is convenient because each range in the array represents a
  valid sub-circuit.

- **Directed acyclic graph (DAG)** representation. The vertices of the DAG are
  the gates of the circuit and the edges encode their relationships. The DAG
  representation has the advantage of making adjacency between gates easy to
  access.

- **Phase polynomial** representation. (work-in-progress)

- **ZX-diagrams**. (work-in-progress)

(Apart for *ZX-diagrams*) Any of these circuit representations is efficiently
convertible to the other in linear time with respect to the number of gates in
the circuit. For example, given a netlist, it is possible to straightforwardly
construct a directed acyclic graph by iterating over all gates. A simple
topological order of the directed acyclic graph nodes is a valid netlist.

A circuit in tweedledum is defined by combining a gate representation with
a network representation.

Architecture
------------

.. image:: /_static/layers.svg
   :align: right

The tweedledum architecture is based on a principle of four layers that depend on
each other as depicted in the figure on the right-hand side. The fundament is
provided by the API layer which is devided in :ref:`network` and :ref:`gate` .
These classes define naming conventions for types and methods for classes that
implement networks and gates, some of which are mandatory while others are
optional. The API layer does *not* provide any implementations.

Algorithms are implemented in terms of generic functions that takes as input an
instance of hypothetical network type and require it to implement the mandatory
and some optional interfaces. The algorithms do not make any assumption on the
internal implementation of the input network and gate types.

The third layer consists of actual network and gate implementations. The
combination of a network type with a gate type defines a circuit. Algorithms
can be called on instances of circuit if they implement the required API.
Static compile time assertions are guaranteeing that compilation succeeds only
for those network implementations that do provide all required types and
methods.

Finally, in order to improve the performance some algorithmic details may be
specialized for some network types based on their internal implementation.
This can be done for each network individually, without affecting the generic
algorithm implementation nor the implementation of other network types.