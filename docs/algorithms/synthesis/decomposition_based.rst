Decomposition-based synthesis
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This synthesis algorithm is based on the following property of reversible
functions:  Any reversible function :math:`f : \mathbb{B}^n \to \mathbb{B}^n`
can be decomposed into three reversible functions :math:`f_r \circ f' \circ
f_l`, where :math:`f_l` and :math:`f_r` are single-target gates acting on target
line :math:`x_i` and :math:`f'` is a reversible function that does not change in
:math:`x_i`.

.. doxygenfunction:: tweedledum::decomposition_based_synthesis

.. doxygenstruct:: tweedledum::decomposition_based_synthesis_params
