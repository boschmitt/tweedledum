********************
01 : Basics
********************

In this tutorial we will go from knowing nothing about tweedledum to creating and printing a quantum
circuit.  Note that this tutorial is *not* a quantum computing 101 tutorial, we assume familiarity
of quantum computing at about the level of the textbook "Quantum Computation and Quantum 
Information" by Nielsen and Chuang.

To begin, please follow the instructions for :ref:`install`.

An empty circuit
-------------------

The basic elements needed for building your first circuit are a `circuit type` and an 
`operation type`.  The combination of those two elements is what defines a circuit in tweedledum. 
You can look at a list of both types in :ref:`implementations`.

To keep this tutorial simple, we shall use the combination ``netlist`` and ``w3_op``.  
Let’s instantiate an empty circuit:

.. code-block:: c++

  #include <tweedledum/tweedledum.hpp>

  int main()
  {
      using namespace tweedledum;
      netlist<w3_op> circuit;
  }

Creating a wires
-------------------

Once you have a circuit, you can create wires in it.  The wire can be either *quantum* or 
*classical*.  In the end, you will need both to do anything useful.  Creating wires is quite
straightforward:

.. code-block:: c++

  wire::id q0 = circuit.create_qubit("q0");
  wire::id c0 = circuit.create_cbit("c0");

.. note::

  In a circuit all wires are named.  However, explicitly naming the wires is optional.  When the
  user do not provide a name, the circuit will use a default naming convention to name the wire.

Operations
-------------------

An ``operation`` is ``gate`` that is applied to a collection of ``wires``, i.e., objects with a
``wire::id`` given by a ``circuit``.  Now that you have wires in your circuit, you can create
operations to manipulate the wires states.

A ``gate`` can be applied to wire(s) by directly constructing a ``operation`` object, or by calling
one of the ``create_op`` methods from a  ``circuit``---this will also create an ``operation``
object. For example:

.. code-block:: c++

  // directly constructing an operation object and emplacing it in the circuit
  w3_op h_op(gate_lib::h, q0);
  circuit.emplace_op(h_op);

  // directly creating an operation in the circuit
  circuit.create_op(gate_lib::h, q0);

You can look at a list of gates available in the ``gate_lib`` in :ref:`gate_lib`.  After applying 
all unitary operations you want, you will need to to measure a the state of quantum wires:

.. code-block:: c++

  circuit.create_op(gate_lib::measure_z, q0, c0);

Printing
-------------------

Finally, to print the circuit to the terminal we just need to call a function:

.. code-block:: c++

  write_utf8(circuit);


TLDR
-------------------

.. code-block:: c++

  #include <tweedledum/tweedledum.hpp>

  int main()
  {
      using namespace tweedledum;
      netlist<w3_op> circuit;
      wire::id q0 = circuit.create_qubit("q0");
      wire::id c0 = circuit.create_cbit("c0");
      circuit.create_op(gate_lib::h, q0);
      circuit.create_op(gate_lib::measure_z, q0, c0);
      write_utf8(circuit);
  }

The output::

  c0 : ══════════■═══
                 ║   
          ┌───┐┌─╨──┐
  q0 : ───┤ H ├┤ Mz ├
          └───┘└────┘