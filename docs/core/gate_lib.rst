.. _gate_lib:

*****************
The Gate Library
*****************

An ``operation`` is ``gate`` that is applied to a collection of I/O's, i.e., objects with a
``io_id`` given by a ``network``. A ``gate`` can be applied to qubit(s) by direcetly constructing a 
``operation`` object, or by calling one of the ``add_gate`` methods from a ``network``---this will 
also create an ``operation`` object. For example:

.. code-block:: c++

  #include <tweedledum/tweedledum.hpp>

  int main(int argc, char** argv)
  {
      using namespace tweedledum;
      netlist<io3_op> network;

      io_id q0 = network.add_qubit();
      io_id q1 = network.add_qubit();
      operation op(gate::cx, q0, q1);
      auto node = network.add_operation(gate::cx, q0, q1);
  }

.. note::
   When using a network ``add_operation`` methdos, the returned value is a ``network::node``
   reference, which basically encapsulate the operation.

Gates are classified into three categories: **Meta**, **non-parameterisable**, and 
**parameterisable**.  Meta gates are internal helpers.  Non-parameterisable gates are uniquely 
indentified by a symbol, e.g., a |T| gate is identifed by ``gate_lib::t``, and from that we know its
parameters.  Parameterisable gates, on the other hand, require the user to define a symbol and the
parameters, e.g., we can define a |R1| gate by ``gate_lib:r1`` and a angle parameter 
:math:`\frac\pi4` (in fact, this gate is the |T| gate).

.. warning::
   It is possible to mix parameterisable and non-parameterisable gates in a quantum circuit
   representation, but some methods will work sub-optimally. For example:
   
   Suppose you want to optimize your circuit using ``gate_cancellation()``.  Somewhere 
   in your circuit, two conditional phase shifts that cancel each other out, say |T| and
   its adjoint |Td|, appear.  One is defined symbolically, and the other parametrically (using 
   ``gate_lib::r1`` and pi/4).  They won't be identified as adjoint by the ``is_adjoint()`` method
   defined in the ``gate`` class!  Hence, this optimization oportunity will be missed.*

Meta gates
===========

+------------------------------------------------------------------+
| .. centered:: Meta gates                                         |
+-----------------------------------------+------------------------+
| Name(s)                                 | tweedledum symbol      |
+=========================================+========================+
| Undefined                               | ``gate_lib::undefined``|
+-----------------------------------------+------------------------+
| Unknown                                 | ``gate_lib::unknown``  |
+-----------------------------------------+------------------------+
| Input                                   | ``gate_lib::input``    |
+-----------------------------------------+------------------------+
| Output                                  | ``gate_lib::output``   |
+-----------------------------------------+------------------------+

Non-parameterisable gates
=========================

.. |H| replace:: :math:`\mathrm{H}`
.. |I| replace:: :math:`\mathrm{I}`
.. |S| replace:: :math:`\mathrm{S}`
.. |T| replace:: :math:`\mathrm{T}`
.. |X| replace:: :math:`\mathrm{X}`
.. |Y| replace:: :math:`\mathrm{Y}`
.. |Z| replace:: :math:`\mathrm{Z}`
.. |Sd| replace:: :math:`\mathrm{S}^{\dagger}`
.. |Td| replace:: :math:`\mathrm{T}^{\dagger}`

.. |CX| replace:: :math:`\mathrm{CX}`
.. |CY| replace:: :math:`\mathrm{CY}`
.. |CZ| replace:: :math:`\mathrm{CZ}`
.. |SWAP| replace:: :math:`\mathrm{SWAP}`

One-qubit gates
----------------

+--------------------------------+--------+----------------------------+
| Name(s)                        | Symbol | tweedledum symbol          |
+================================+========+============================+
| Identity                       | |I|    | ``gate_lib::identity``     |
+--------------------------------+--------+----------------------------+
| Hadamard                       | |H|    | ``gate_lib::hadamard``     |
+--------------------------------+--------+----------------------------+
| Pauli X, NOT                   | |X|    | ``gate_lib::x``            |
+--------------------------------+--------+----------------------------+
| Pauli Y                        | |Y|    | ``gate_lib::y``            |
+--------------------------------+--------+----------------------------+
| Pauli Z, Phase flip            | |Z|    | ``gate_lib::z``            |
+--------------------------------+--------+----------------------------+
| Phase                          | |S|    | ``gate_lib::phase``        |
+--------------------------------+--------+----------------------------+
| Adjoint Phase                  | |Sd|   | ``gate_lib::phase_dagger`` |
+--------------------------------+--------+----------------------------+
| T                              | |T|    | ``gate_lib::t``            |
+--------------------------------+--------+----------------------------+
| Adjoint T                      | |Td|   | ``gate_lib::t_dagger``     |
+--------------------------------+--------+----------------------------+

Hadamard
^^^^^^^^^

The Hadamard is a half rotation of the Bloch sphere. It rotates around an axis located halfway
between the x and z axis. This gives it the effect of rotating states that point along the z axis
to those pointing along x, and vice versa.

.. math::

   \mathrm{H} = \frac{1}{\sqrt{2}}\pmatrix{1&1 \\ 1&-1}

.. note::

   The Hadamard gates is central in quantum computing. I can be used to create states in
   superposition from classical base states:

   .. math::
      \mathrm{H}|0\rangle = |+\rangle \\
      \mathrm{H}|1\rangle = |-\rangle \\
   
   Since |H| is self-adjoint, i.e., :math:`\mathrm{H}^\dagger = \mathrm{H}`, the inverse direction
   also holds:  

   .. math::
      \mathrm{H}|+\rangle = |0\rangle \\
      \mathrm{H}|-\rangle = |1\rangle \\

   Remember that both :math:`|+\rangle` and :math:`|-\rangle`, when measured in the computational
   basis, have 0.5 probability of beign :math:`|0\rangle` and 0.5 probability of beign 
   :math:`|1\rangle`. In other words, one can perceive the behaviour of the hadamard gate as 
   deterministically turning a "random state" into a classical one.


Identity
^^^^^^^^^

The identity element of the unitary group :math:`U(2)`.  This does not change the quantum state, so
it can be perceived as the absence of a gate.

.. math::

   \mathrm{I} = \pmatrix{1&0 \\ 0&1}

Pauli-X
^^^^^^^^^

The Pauli X gate swaps the amplitudes of the quantum base states. As :math:`X|0\rangle = |1\rangle`
and :math:`X|1\rangle = |0\rangle`, this gate is also known as :math:`\mathrm{NOT}`.

.. math::

   \sigma_x = \mathrm{X} = \pmatrix{0&1 \\ 1&0}

Pauli-Y
^^^^^^^^^
.. math::

   \sigma_y = \mathrm{Y} = \pmatrix{0&-i \\ i&0}

Pauli-Z
^^^^^^^^^

The Pauli Z gate inverts the sign of the second amplitudes of a quantum state.

.. math::

   \sigma_z = \mathrm{Z} = \pmatrix{1&0 \\ 0&-1}

Phase
^^^^^^^^^

.. math::

   \mathrm{S} = \pmatrix{1&0 \\ 0&i}

T
^^^^^^^^^

.. math::

   \mathrm{T} = \pmatrix{1&0 \\ 0&e^{i\frac{\pi}{4}}}


Two-qubit gates
----------------

+--------------------------------+--------+------------------------+
| Name(s)                        | Symbol | tweedledum symbol      |
+================================+========+========================+
| Control X, Control NOT, CNOT   | |CX|   | ``gate_lib::cx``       |
+--------------------------------+--------+------------------------+
| Control Y                      | |CY|   | ``gate_lib::cy``       |
+--------------------------------+--------+------------------------+
| Control Z                      | |CZ|   | ``gate_lib::cz``       |
+--------------------------------+--------+------------------------+
| Swap                           | |SWAP| | ``gate_lib::swap``     |
+--------------------------------+--------+------------------------+

CX
^^^^^^^^^

.. math::

   \mathrm{CX} = \pmatrix{1&0&0&0 \\ 0&1&0&0 \\ 0&0&0&1 \\ 0&0&1&0}

CY
^^^^^^^^^

.. math::

   \mathrm{CX} = \pmatrix{1&0&0&0 \\ 0&1&0&0 \\ 0&0&0&-i \\ 0&0&i&0}

CZ
^^^^^^^^^

.. math::

   \mathrm{CZ} = \pmatrix{1&0&0&0 \\ 0&1&0&0 \\ 0&0&1&0 \\ 0&0&0&-1}

Swap
^^^^^^^^^

.. math::

   \mathrm{SWAP} = \pmatrix{1&0&0&0 \\ 0&0&1&0 \\ 0&1&0&0 \\ 0&0&0&1}

N-qubit gates
----------------

+--------------------------------+--------+------------------------+
| Name(s)                        | Symbol | tweedledum symbol      |
+================================+========+========================+
| Multiple Control NOT, Toffoli  |        | ``gate_lib::mcx``      |
+--------------------------------+--------+------------------------+
| Multiple Control Y             |        | ``gate_lib::mcy``      |
+--------------------------------+--------+------------------------+
| Multiple Control Z             |        | ``gate_lib::mcz``      |
+--------------------------------+--------+------------------------+

MCX
^^^^^^^^^

.. math::

   \mathrm{MCR}_x = \pmatrix{1&&&0&0 \\ &\ddots&&\vdots&\vdots \\ &&1&0&0 \\ 0&\cdots&0&0&1\\ 0&\cdots&0&1&0 }

MCY
^^^^^^^^^

.. math::

   \mathrm{MCR}_y = \pmatrix{1&&&0&0 \\ &\ddots&&\vdots&\vdots \\ &&1&0&0 \\ 0&\cdots&0&0&-i \\ 0&\cdots&0&i&0 }

MCZ
^^^^^^^^^

.. math::

   \mathrm{MCR}_z = \pmatrix{1&&&0&0 \\ &\ddots&&\vdots&\vdots \\ &&1&0&0 \\ 0&\cdots&0&1&0 \\ 0&\cdots&0&0&-1 }


Parameterisable gates
=========================

.. |R1| replace:: :math:`\mathrm{R}_1`
.. |Rx| replace:: :math:`\mathrm{R}_x`
.. |Ry| replace:: :math:`\mathrm{R}_y`
.. |Rz| replace:: :math:`\mathrm{R}_z`
.. |U| replace:: :math:`\mathrm{U}`

.. |CRx| replace:: :math:`\mathrm{CR}_x`
.. |CRy| replace:: :math:`\mathrm{CR}_y`
.. |CRz| replace:: :math:`\mathrm{CR}_z`
.. |MCRx| replace:: :math:`\mathrm{MCR}_x`
.. |MCRy| replace:: :math:`\mathrm{MCR}_y`
.. |MCRz| replace:: :math:`\mathrm{MCR}_z`

One-qubit gates
----------------

+--------------------------------+--------+------------------------+
| Name(s)                        | Symbol | tweedledum symbol      |
+================================+========+========================+
| Rotation 1, Phase shift        | |R1|   | ``gate_lib::r1``       |
+--------------------------------+--------+------------------------+
| Rotation X                     | |Rx|   | ``gate_lib::rx``       |
+--------------------------------+--------+------------------------+
| Rotation Y                     | |Ry|   | ``gate_lib::ry``       |
+--------------------------------+--------+------------------------+
| Rotation Z                     | |Rz|   | ``gate_lib::rz``       |
+--------------------------------+--------+------------------------+
| U                              | |U|    | ``gate_lib::u3``       |
+--------------------------------+--------+------------------------+

R1
^^^^^^^^^

This is a parameterisable conditional phase shift gate.  This gate leaves the basis state 
:math:`|0\rangle` unchanged and map :math:`|1\rangle` to :math:`e^{{i\theta }}|1\rangle`.  It
**does not** affect probability of measuring a :math:`|0\rangle` or :math:`|1\rangle`, however it
modifies the phase of the quantum state. The angle of rotation must be specified in radians and can
be positive or negative.  It's matrix form is:

.. math::

   \mathrm{R}_1(\theta) = \pmatrix{1&0 \\ 0&e^{i\theta}}

The gates |T|, |S|, |Z|, |Sd|, and |Td| can be implemented using this gate:

.. math::

   \mathrm{T} &= \mathrm{R}_1(\pi \mathbin{/} 4) \\
   \mathrm{S} &= \mathrm{R}_1(\pi \mathbin{/} 2) = \mathrm{T}^2 \\
   \mathrm{Z} &= \mathrm{R}_1(\pi) = \mathrm{T}^4 \\
   \mathrm{S}^{\dagger} &= \mathrm{R}_1(3\pi \mathbin{/} 2) = \mathrm{T}^6 \\
   \mathrm{T}^{\dagger} &= \mathrm{R}_1(7\pi \mathbin{/} 4) = \mathrm{T}^7

Note that one can obtain it's adjoint by changing the sign of :math:`\theta`, i.e.:

.. math::

   \mathrm{R}^{\dagger}_1(\theta) = \mathrm{R}_1(-\theta).

.. note::

   One might be asking: "Why :math:`\theta` is not devided by two?".  As you can see, on all other 
   parameterisable gates this is the case.  Well, the answer lies on the following equation:

   .. math::
      \mathrm{R}_1(\theta) = e^{i\frac{\theta}{2}}\mathrm{R}_z(\theta).

   This means that :math:`\mathrm{R}_1(\theta)` is up to global phase equal to 
   :math:`\mathrm{R}_z(\theta)`. As long as we don't do anything that could make the global phases
   relevant, e.g. adding a control to |Rz|, those gates can have the same implementation.


Rx
^^^^^^^^^

On the Bloch sphere, this gate corresponds to rotating the qubit state around the x axis by the
given angle :math:`\theta`. The angle of rotation must be specified in radians and can be positive
or negative. It's matrix form is:

.. math::

   \mathrm{R}_x(\theta) = \pmatrix{\cos\frac\theta2 & -i\sin\frac\theta2 \\ -i\sin\frac\theta2 & \cos\frac\theta2}

Ry
^^^^^^^^^

On the Bloch sphere, this gate corresponds to rotating the qubit state around the y axis by the
given angle :math:`\theta`. The angle of rotation must be specified in radians and can be positive
or negative. It's matrix form is:

.. math::

   \mathrm{R}_y(\theta) = \pmatrix{\cos\frac\theta2 & -\sin\frac\theta2 \\ \sin\frac\theta2 & \cos\frac\theta2}

Rz
^^^^^^^^^

On the Bloch sphere, this gate corresponds to rotating the qubit state around the z axis by the
given angle :math:`\theta`. The angle of rotation must be specified in radians and can be positive
or negative. It's matrix form is

.. math::

   \mathrm{R}_z(\theta) = \pmatrix{e^{-i\frac\theta2}&0 \\ 0&e^{i\frac\theta2}}

U
^^^^^^^^^

.. math::

   \mathrm{U}(\theta, \phi, \lambda) = \pmatrix{\cos\frac\theta2 & -e^{i\lambda}\sin\frac\theta2 \\ e^{i\phi}\sin\frac\theta2 & e^{i(\lambda + \phi)}\cos\frac\theta2}

Most single-qubti gates can be implemented using this gates:

.. math::

   \mathrm{H} &= \mathrm{U}(\pi \mathbin{/} 2, 0, \pi) \\
   \mathrm{I} &= \mathrm{U}(0, 0, 0) \\
   \mathrm{X} &= \mathrm{U}(\pi, 0, \pi) \\
   \mathrm{Y} &= \mathrm{U}(\pi, \pi \mathbin{/} 2, \pi \mathbin{/} 2) \\
   \mathrm{Z} &= \mathrm{U}(0, 0, \pi) \\
   \mathrm{S} &= \mathrm{U}(0, 0, \pi \mathbin{/} 2) \\
   \mathrm{T} &= \mathrm{U}(0, 0, \pi \mathbin{/} 4) \\
   \mathrm{S}^\dagger &= \mathrm{U}(0, 0, -\pi \mathbin{/} 2) = \mathrm{U}(0, 0, 3\pi \mathbin{/} 2)\\
   \mathrm{T}^\dagger &= \mathrm{U}(0, 0, -\pi \mathbin{/} 4) = \mathrm{U}(0, 0, 7\pi \mathbin{/} 4) \\
   \mathrm{R}_1(\theta) &= \mathrm{U}(0, 0, \theta) \\
   \mathrm{R}_x(\theta) &= \mathrm{U}(\theta, -\pi \mathbin{/} 2, \pi \mathbin{/} 2) \\
   \mathrm{R}_y(\theta) &= \mathrm{U}(\theta, 0, 0) \\

Two-qubit gates
----------------

+--------------------------------+--------+------------------------+
| Name(s)                        | Symbol | tweedledum symbol      |
+================================+========+========================+
| Controlled rotation X          | |CRx|  | ``gate_lib::crx``      |
+--------------------------------+--------+------------------------+
| Controlled rotation Y          | |CRy|  | ``gate_lib::cry``      |
+--------------------------------+--------+------------------------+
| Controlled rotation Z          | |CRz|  | ``gate_lib::crz``      |
+--------------------------------+--------+------------------------+

CRx
^^^^^^^^^

.. math::

   \mathrm{CR}_x = \pmatrix{1&0&0&0 \\ 0&1&0&0 \\ 0&0&\cos\frac\theta2&-i\sin\frac\theta2 \\ 0&0&-i\sin\frac\theta2&\cos\frac\theta2}

CRy
^^^^^^^^^

.. math::

   \mathrm{CR}_y = \pmatrix{1&0&0&0 \\ 0&1&0&0 \\ 0&0&\cos\frac\theta2&-\sin\frac\theta2 \\ 0&0&\sin\frac\theta2&\cos\frac\theta2}

CRz
^^^^^^^^^

.. math::

   \mathrm{CR}_z = \pmatrix{1&0&0&0 \\ 0&1&0&0 \\ 0&0&e^{-i\frac\theta2}&0 \\ 0&0&0&e^{i\frac\theta2}}


N-qubit gates
----------------

+--------------------------------+--------+------------------------+
| Name(s)                        | Symbol | tweedledum symbol      |
+================================+========+========================+
| Controlled rotation X          | |MCRx| | ``gate_lib::mcrx``     |
+--------------------------------+--------+------------------------+
| Controlled rotation Y          | |MCRy| | ``gate_lib::mcry``     |
+--------------------------------+--------+------------------------+
| Controlled rotation Z          | |MCRz| | ``gate_lib::mcrz``     |
+--------------------------------+--------+------------------------+

MCRx
^^^^^^^^^

.. math::

   \mathrm{MCR}_x = \pmatrix{1&&&0&0 \\ &\ddots&&\vdots&\vdots \\ &&1&0&0 \\ 0&\cdots&0&\cos\frac\theta2&-i\sin\frac\theta2 \\ 0&\cdots&0&-i\sin\frac\theta2&\cos\frac\theta2 }

MCRy
^^^^^^^^^

.. math::

   \mathrm{MCR}_y = \pmatrix{1&&&0&0 \\ &\ddots&&\vdots&\vdots \\ &&1&0&0 \\ 0&\cdots&0&\cos\frac\theta2&-\sin\frac\theta2 \\ 0&\cdots&0&\sin\frac\theta2&\cos\frac\theta2 }

MCRz
^^^^^^^^^

.. math::

   \mathrm{MCR}_z = \pmatrix{1&&&0&0 \\ &\ddots&&\vdots&\vdots \\ &&1&0&0 \\ 0&\cdots&0&e^{-i\frac\theta2}&0 \\ 0&\cdots&0&0&e^{i\frac\theta2} }