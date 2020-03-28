********************
Conceptual overview
********************

The most commonly used notation for representing quantum algorithms is the quantum circuit model
introduced by Deutsch.  The model describes the computation as a sequence of elementary quantum
logic gates acting on a collection of qubits.  

In tweedledum, the representation of quantum programs is a *quantum network*, hereinafter called
``network`` (see ).  A ``network`` is a collection of ``operations``.  An ``operation`` is some 
effect that operates on a specific subset ``wires``.  A ``gate`` describes the effect of the
operation. Let's analyze a small quantum circuit to illustrate these concetps:

.. image:: /_static/concept_back.svg
   :width: 50%
   :align: center

Wires
======

At the very bottom of the layers of abstraction rest concepts of *quantum bit*, ``qubit``, and  
*classical bit*, ``cbit``. I expect the reader to be familiar with those.  So I will start
describing the ``wires``:

.. image:: /_static/concept_wires.svg
   :width: 50%
   :align: center

A ``wire`` can be either a *quantum* or *classical*.  A quantum wire holds the state of a ``qubit``,
and it is represented by a line in quantum circuit diagrams.  In tweedledum, a quantum wire is 
equivalent to a qubit.  Similarly, a classical wire holds the state of a ``cbit``, and it is
represented by a double line in quantum circuit diagrams.

In a quantum network, each wire has a ``wire_id``.  The ``wire_id`` is used to uniquely identify a 
wire, and to it indicate whether the wire is *quantum* or *classical*.  Wires are by calling one 
of the ``create_qubit()`` or ``create_cbit()`` methods from a ``network``.  We can also directly 
instantiate a ``wire_id`` object.  A wire created by direct instantiation, however, won't be part of
a quantum network. Indeed, even it the API is weird:

.. code-block:: c++

  #include <tweedledum/tweedledum.hpp>

  int main(int argc, char** argv)
  {
      wire_id q0(0, /* is_qubit = */ true);
      wire_id q1(1, /* is_qubit = */ true);
      wire_id c0(2, /* is_qubit = */ false);
  }

Operations
===========

An ``operation`` is some effect that operates on a specific subset ``wires``:

.. image:: /_static/concept_ops.svg
   :width: 50%
   :align: center


Gates 
======

A ``gate`` describes the effect of the operation on the wires.  Most often this effect is an unitary
evolution, hence the gate is a *quantum gate*.  In our small example, we have two 'pure' quantum
gates: the Hadamard gate :math:`\mathrm{H}`, and the :math:`\mathrm{CNOT}` gate :math:`\mathrm{CX}`.

.. image:: /_static/concept_gates.svg
   :width: 50%
   :align: center

The weird looking 'meter gate' is actually a *measurement gate*.  As measurement is irreversible, 
it is not a quantum gate. Finaly, the last gate is NOT gate :math:`\mathrm{\oplus}` that is applied
whenever the state of the `cbit` is `true`.

.. code-block:: c++

  #include <tweedledum/tweedledum.hpp>

  int main(int argc, char** argv)
  {
      wire_id q0(0, /* is_qubit = */ true);
      wire_id q1(1, /* is_qubit = */ true);
      wire_id c0(2, /* is_qubit = */ false);
      operation h_op(gate_lib::h, q1);
      operation cx_op(gate_lib::cx, q1, q0);
      operation m_op(gate_lib::measure_z, q0, c0);
  }

Networks
=========

- A **netlist** represents the circuit as a list of gates to be applied sequentially. It is
  convenient because each range in the array represents a valid sub-circuit.

- **Directed acyclic graph (DAG)** representation, ``op_dag``.  The vertices of the DAG are the
  operations of the circuit and the edges encode their relationships. The DAG representation has the
  advantage of making adjacency between gates easy to access.

- **Mapped DAG** representation, ``mapped_dag``.  The same as ``op_dag`` but mapped to a particular
  device architecture.

- **Unitary** representation.  A unitary matrix representation of the circuit.  Not scalable at all,
  the unitary is literally represented as a :math:`2^n \times 2^n` matrix, where :math:`n` is the
  number of qubits.

- **Phase polynomials** representation. (work-in-progress)

- **Path polynomials** representation. (work-in-progress)

- **Exponents of Pauli** representation. (work-in-progress)
