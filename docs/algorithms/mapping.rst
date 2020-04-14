Mapping
***************************

In the quantum circuit model of computation, circuits manipulate qubits.  These qubits exisit as
abstractions within a circuit and shall be called *virtual* (or logical).  Mapping is the taks of 
finding a one-to-one relation between *virtual* qubits and *physical* qubits, which denote the
actual hardware units that store quantum bits.

In the circuit model, any pair of *virtual* qubits can interact, i.e., we can perform a two-qubit
operation between any pair.  The *physical* qubits in most quantum hardware, however, are not fully
connected, meaning that not every paif of *physical* qubits can participate on the same quantum
operation.  These connectivity restrictions imposed by the hardware are known as
*coupling constraints*.

Mapping is not always possible without transforming the quantum circuit.  

In tweedledum, mapping is devided in two subtaks:

- **Placement**:

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

- **Routing**:

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

   mapping/place-hsat
   mapping/place-line
   mapping/place-random
   mapping/place-sat
   mapping/router-astar
   mapping/router-jit
   mapping/router-sabre
   mapping/router-sat