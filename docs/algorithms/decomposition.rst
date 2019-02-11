Decomposition
-------------

**Decomposition**: is the process of breaking down larger parts or elements into smaller ones.

High-level quantum algorithms are technology-independent, that is, allow arbitrary quantum gates,
and do not take architectural constraints into account. Quite often, these algorithms involve quantum
gates acting on *n* qubits. In order to execute such an algorithm in a quantum computer it is
necessary to decompose these gates in an series of simpler gates. 

The *tweedledum* library implements several decomposition algorithms. The following table lists all
decomposition algorithms that are currently provided in *tweedledum*

.. toctree::
   :maxdepth: 2
   :hidden:

   decomposition/barenco
   decomposition/dt
