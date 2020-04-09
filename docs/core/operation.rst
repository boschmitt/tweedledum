.. _operation:
**************
Operation API
**************

An ``operation`` is an ``gate`` that is applied to a collection of `wires`.  The wires are
identified by a ``wire_id`` s given by a ``network``.

This page describes the interface of a operation data structure in *tweedledum*.

.. warning::
   This part of the documentation makes use of a class called ``operation``. This class has been
   created solely for the purpose of creating this documentation and is not meant to be used in
   code. 

.. important::
   An ``peration`` implementation **must** derive from the ``gate`` class.

Gate
============
.. doxygenclass:: tweedledum::gate

Properties
-------------

.. doxygenclass:: tweedledum::gate
   :members: id, is, is_adjoint, is_meta, is_one_qubit, is_two_qubit, is_r1, is_measurement, axis
   :no-link:

Parameters
-------------

.. doxygenclass:: tweedledum::gate
   :members: rotation_angle, theta, phi, lambda
   :no-link:

Operation
============

Types and constants
-------------------

An operation must expose the following compile-time constants:

.. code-block:: c++

   static constexpr uint32_t max_num_wires;
   static constexpr uint32_t network_max_num_wires;

The struct ``is_gate_type`` can be used to check at compile time whether a given type contains all
required types and constants to implement a network type. It should be used in the beginning of
an algorithm that expects a gate type:

.. code-block:: c++

   template<typename Gate>
   class network {
          static_assert(is_gate_type_v<Gate>, "Gate is not a gate type");
   };

Constructors
--------------

.. doxygenclass:: tweedledum::operation
   :members: operation
   :no-link:

Properties
--------------

.. doxygenclass:: tweedledum::operation
   :members: num_wires, num_controls, num_targets, control, target, position, wire, is_adjoint, is_dependent
   :no-link:

Iterators
--------------

.. doxygenclass:: tweedledum::operation
   :members: foreach_control, foreach_target
   :no-link: