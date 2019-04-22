Transformation-based synthesis (TBS)
------------------------------------

**Header:** :file:`tweedledum/algorithms/synthesis/tbs.hpp`

Transformation-based synthesis algorithms modify the input permutation by adding
gates to the input or output side of a circuit in a way that the permutation
becomes the identity.  The gates collected in this process construct a
reversible circuit realizing the permutation.

Parameters
~~~~~~~~~~

.. doxygenstruct:: tweedledum::tbs_params
   :members:

Algorithm
~~~~~~~~~

.. doxygenfunction:: tweedledum::tbs(std::vector<uint32_t>, tbs_params)

.. doxygenfunction:: tweedledum::tbs(Network&, std::vector<io_id> const&, std::vector<uint32_t>, tbs_params)
