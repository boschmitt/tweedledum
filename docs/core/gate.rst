Gate interface API
==================

A ``Gate`` is an ``gate_base`` that is applied to a collection of qubits. Those qubits are identified
by a ``qid`` given by a ``network``.

This page describes the interface of a quantum gate data structure in *tweedledum*.

.. warning::

   This part of the documentation makes use of a class called ``gate``. This class has been
   created solely for the purpose of creating this documentation and is not meant to be used in
   code. 

Mandatory types and constants
-----------------------------

A gate must expose the following compile-time constants:

.. code-block:: c++

   static constexpr uint32_t max_num_qubits;
   static constexpr uint32_t network_max_num_qubits;

The struct ``is_gate_type`` can be used to check at compile time whether a given type contains all
required types and constants to implement a network type. It should be used in the beginning of
an algorithm that expects a gate type:

.. code-block:: c++

   template<typename Gate>
   class network {
          static_assert(is_gate_type_v<Gate>, "Gate is not a gate type");
   };

Methods
-------

Constructors
~~~~~~~~~~~~

.. doxygenclass:: tweedledum::gate
   :members: gate
   :no-link:

Properties
~~~~~~~~~~

.. doxygenclass:: tweedledum::gate
   :members: num_controls, num_targets, op, rotation_angle
   :no-link:

Iterators
~~~~~~~~~

.. doxygenclass:: tweedledum::gate
   :members: foreach_control, foreach_target
   :no-link: