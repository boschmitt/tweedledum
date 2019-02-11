Gray synthesis for {CNOT, Rz} circuits
--------------------------------------

**Header:** :file:`tweedledum/algorithms/synthesis/gray_synth.hpp`

Parameters
~~~~~~~~~~

.. doxygenstruct:: tweedledum::gray_synth_params
   :members:

Algorithm
~~~~~~~~~

.. doxygenfunction:: tweedledum::gray_synth(Network&, std::vector<uint32_t> const&, parity_terms const&, gray_synth_params)

.. doxygenfunction:: tweedledum::gray_synth(uint32_t, parity_terms const&, gray_synth_params)
