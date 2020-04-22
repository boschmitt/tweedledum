Mapping
***************************

In the quantum circuit model of computation, circuits manipulate qubits. These qubits exist as
abstractions within a circuit and shall be called *virtual* (or logical). Mapping is the task of
finding a one-to-one relation between virtual qubits and *physical* qubits, which denote the actual
hardware units that store quantum bits, such that allow the execution of all gates.

In the circuit model, any pair of virtual qubits can interact, i.e., we can perform a two-qubit
operation between any pair. The physical qubits in most quantum hardware, however, are not fully
connected, meaning that not every pair of physical qubits can participate in the same quantum
operation. These physical connectivity restrictions are known as coupling constraints.

The library divides mapping into two subtasks: **placement** and **routing**.

Placement
----------

Placement tries to find a bijection between the set of virtual qubits and the set of physical qubits,
such that the quantum device can execute all gates. Such perfect placement, however, might not exist.
In this case, a placement algorithm might return an initial placement. When the placement is not
enough, it is necessary to transform the circuit.

+---------------------------+----------------------------------------------------------------------+
| Function                  | Description                                                          |
+===========================+======================================================================+
| :ref:`hsat-place`         | Yet to be written.                                                   |
+---------------------------+----------------------------------------------------------------------+
| :ref:`line-place`         | Yet to be written.                                                   |
+---------------------------+----------------------------------------------------------------------+
| :ref:`random-place`       | yet to be written.                                                   |
+---------------------------+----------------------------------------------------------------------+
| :ref:`sat-place`          | yet to be written.                                                   |
+---------------------------+----------------------------------------------------------------------+

.. toctree::
   :maxdepth: 2
   :hidden:

   mapping/place-hsat
   mapping/place-line
   mapping/place-random
   mapping/place-sat


Routing
--------

Given an initial placement, a router uses SWAP operations to move the virtual qubits around the
device topology whenever it finds an operation that acts on qubits that are physically separated.
It routes the involved virtual qubits to adjacent physical qubits. 

+---------------------------+----------------------------------------------------------------------+
| Function                  | Description                                                          |
+===========================+======================================================================+
| :ref:`astar-router`       | Yet to be written.                                                   |
+---------------------------+----------------------------------------------------------------------+
| :ref:`jit-router`         | yet to be written.                                                   |
+---------------------------+----------------------------------------------------------------------+
| :ref:`sabre-router`       | yet to be written.                                                   |
+---------------------------+----------------------------------------------------------------------+
| :ref:`sat-router`         | yet to be written.                                                   |
+---------------------------+----------------------------------------------------------------------+

.. toctree::
   :maxdepth: 2
   :hidden:

   mapping/router-astar
   mapping/router-jit
   mapping/router-sabre
   mapping/router-sat