.. _network:

Network API
===========

This page describes the interface of a quantum network data structure in *tweedledum*.

.. warning::

   This part of the documentation makes use of a class called ``network``. This class has been
   created solely for the purpose of creating this documentation and is not meant to be used in
   code. Custom network implementation do **not** have to derive from this class, but only need to
   ensure that, if they implement a function of the interface, it is implemented using the same
   signature.

Mandatory types and constants
-----------------------------

The interaction with a network data structure is performed using four types for which no application
details are assumed. The following four types must be defined within the network data structure.
They can be implemented as nested type, but may also be exposed as type alias.

.. doxygenclass:: tweedledum::network
   :members: base_type, gate_type, node_type, storage_type
   :no-link:

Further, a network must expose the following compile-time constants:

.. code-block:: c++

   static constexpr uint32_t min_fanin_size;
   static constexpr uint32_t max_fanin_size;

The struct ``is_network_type`` can be used to check at compile time whether a given type contains
all required types and constants to implement a network type. It should be used in the beginning
of an algorithm that expects a network type:

.. code-block:: c++

   template<typename Network>
   void algorithm(Network const& ntk) {
          static_assert(is_network_type_v<Network>, "Network is not a network type");
   }

Methods
-------

Constructors
~~~~~~~~~~~~

.. doxygenclass:: tweedledum::network
   :members: network
   :no-link:

Qubits and Ancillae
~~~~~~~~~~~~~~~~~~~~~

.. doxygenclass:: tweedledum::network
   :members: add_qubit
   :no-link:

Structural properties
~~~~~~~~~~~~~~~~~~~~~

.. doxygenclass:: tweedledum::network
   :members: size, num_qubits, num_gates
   :no-link:

Node iterators
~~~~~~~~~~~~~~

.. doxygenclass:: tweedledum::network
   :members: foreach_qubit, foreach_input, foreach_output, foreach_gate, foreach_node
   :no-link:
