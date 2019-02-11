Mapping
-------

We call **mapping** the task of assigning the pseudo qubits of a quantum
circuits to physical qubits of a NISQ computer.  This task does not include
any error correction, i.e., a pseudo qubit is assigned to a single physical
qubit.  The task includes to find an initial mapping of the pseudo qubits to
the physical ones and a rearrangement of gates corresponding to this initial
mapping.  The mapper may include additional SWAP gates in order to satisfy the
coupling constraints of the device.

.. toctree::
   :maxdepth: 2
   :hidden:

   mapping/types
   mapping/greedy_map
