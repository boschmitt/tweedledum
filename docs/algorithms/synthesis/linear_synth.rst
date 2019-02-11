Linear synthesis for {CNOT, Rz} circuits
----------------------------------------

**Header:** :file:`tweedledum/algorithms/synthesis/linear_synth.hpp`

Parameters
~~~~~~~~~~

.. doxygenstruct:: tweedledum::linear_synth_params
   :members:

Algorithm
~~~~~~~~~

.. doxygenfunction:: tweedledum::linear_synth(Network&, std::vector<uint32_t> const&, parity_terms const&, linear_synth_params)

.. doxygenfunction:: tweedledum::linear_synth(uint32_t, parity_terms const&, linear_synth_params)
