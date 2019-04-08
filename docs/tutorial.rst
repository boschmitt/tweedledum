Tutorial
========

In this tutorial we will go from knowing nothing about tweedledum to solving a
satisfiability problem using Grover's search algorithm. Note that this tutorial
is *not* a quantum computing 101 tutorial, we assume familiarity of quantum
computing at about the level of the textbook "Quantum Computation and Quantum
Information" by Nielsen and Chuang.

To begin, please follow the instructions for :ref:`install`.

Background: Boolean satisfiability problem
------------------------------------------

The Boolean Satisfiability Problem (or SAT) is the problem of determining if a
propositional formula representing a Boolean function is satisfiable. A formula
is satisfiable (SAT) when there is an assignment true/false values for the
variablesin the formaula such that the formula evaluates to *True*. Otherwise
the formula is unsatisfiable (UNSAT).

Usually, the propositional formula is expressend in so-called "conjunctive normal
form" (CNF) as an AND of ORs. For example,

:math:`F(x_1, x_2, x_3) = 
(\bar{x}_1 + \bar{x}_2 + \bar{x}_3)
(x_1 + \bar{x}_2 + x_3)
(x_1 + x_2 + \bar{x}_3)
(x_1 + \bar{x}_2 + \bar{x}_3)
(\bar{x}_1 + x_2 + x_3)`

Where the addition symbol :math:`+` denote a logical OR, and the omitted
multiplication symbols denote a logical AND. The :math:`x_i`'s that
appear on the right-hand side are called literals. A literal is either
a variable :math:`x_i` or the complement of a variable :math:`\bar{x}_i`.
For example, if :math:`x_1 = 1`, :math:`x_2 = 1` and :math:`x_3 = 0`,
then

:math:`F(1, 1, 0) = 
(0 + 0 + 1)
(1 + 0 + 0)
(1 + 1 + 1)
(1 + 0 + 1)
(0 + 1 + 0)`

Clearly, the propositional formula :math:`F` is satisfiable. Moreover, 
there are more satisfying assignments as one can see when we explicity
represent :math:`F`'s truth table:

=========== =========== =========== ==========
:math:`x_1` :math:`x_2` :math:`x_3`  :math:`F`
=========== =========== =========== ==========
:math:`0`    :math:`0`   :math:`0`  :math:`1`
:math:`0`    :math:`0`   :math:`1`  :math:`0`
:math:`0`    :math:`1`   :math:`0`  :math:`0`
:math:`0`    :math:`1`   :math:`1`  :math:`0`
:math:`1`    :math:`0`   :math:`0`  :math:`0`
:math:`1`    :math:`0`   :math:`1`  :math:`1`
:math:`1`    :math:`1`   :math:`0`  :math:`1`
:math:`1`    :math:`1`   :math:`1`  :math:`0`
=========== =========== =========== ==========

Most SAT solvers take as input CNF in a simplified version of the DIMACS format::

  c Example DIMACS 3-SAT file with 3 satisfying solutions: -1 -2 -3 0,  1 2 -3 0, 1 -2 3 0
  p cnf 3 5
  -1 -2 -3 0
  1 -2 3 0
  1 2 -3 0
  1 -2 -3 0
  -1 2 3 0

Background: Grover's algorithm
------------------------------

The purpose of Grover's algorithm is usually described as "searching an
unstructured database" or "searching an unordered list of items," however such
descriptions are misleading. In reality, Grover's algorithm does not search
through lists or databases; it searches through function inputs. It takes an
unknown function, searches the implicit list of its possible inputs, and (with
high probability) returns the single input that causes the function to return a
particular output. The aim here is not explain in detail how/why the algorithm
works, for those intrested in a good explanation refer to 
`this excellently written blog post 
<http://twistedoakstudios.com/blog/Post2644_grovers-quantum-search-algorithm>`_
by Craig Gidney.

Creating a circuit
-------------------

The basic elements needed for building your first circuit are a network type
and gate type. The combination of those two elements is what defines a circuit
in tweedledum. 

To keep this tutorial simple, we shall use the combination ``netlist`` and
``mcmt_gate``. Let’s instantiate an empty circuit.

.. code-block:: c++

  #include <tweedledum/tweedledum.hpp>

  int main(int argc, char** argv)
  {
      using namespace tweedledum;
      netlist<mcst_gate> network;
  }

After you create the circuit, it is necessary to add qubits to it.

.. code-block:: c++

  io_id q1 = network.add_qubit("x1");
  io_id q2 = network.add_qubit("x2");
  io_id q3 = network.add_qubit("x3");
  io_id qf = network.add_qubit("F");

Note: Naming the qubits is optional. ()

Once you have a circuit with qubits, you can add gates ("operations") to
manipulate the qubits states. As you proceed through the documentation you will
find more gates and circuits;

.. code-block:: c++

	network.add_gate(gate_set::pauli_x, "x3");
	network.add_gate(gate_set::pauli_x, "F");
	network.add_gate(gate_set::hadamard, q1);
	network.add_gate(gate_set::hadamard, q2);
	network.add_gate(gate_set::hadamard, q3);
	network.add_gate(gate_set::hadamard, qf);

Synthesizing the Oracle
-----------------------

Optimizing the Oracle
---------------------

.. todo:: Finish writing