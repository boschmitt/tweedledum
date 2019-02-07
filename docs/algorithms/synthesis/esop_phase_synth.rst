ESOP-phase synthesis
--------------------

**Header:** :file:`tweedledum/algorithms/synthesis/esop_phase_synth.hpp`

Given an :math:`n`-variable Boolean function :math:`f`, this synthesis
algorithm generates a quantum circuit with :math:`n` qubits composed of
multiple-controlled Z gates that computes the unitary operation
:math:`U : |\varphi\rangle \mapsto (-1)^{f(\varphi)}|\varphi\rangle`.

Algorithm
~~~~~~~~~

.. doxygenfunction:: tweedledum::esop_phase_synth(kitty::dynamic_truth_table const&)

.. doxygenfunction:: tweedledum::esop_phase_synth(Network&, std::vector<qubit_id> const&, kitty::dynamic_truth_table const&)
