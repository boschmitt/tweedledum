***************************
02 : Parameterisable gates
***************************

.. todo:: Writing

TLDR
-------------------

.. code-block:: c++

  #include <tweedledum/tweedledum.hpp>

  int main()
  {
      using namespace tweedledum;
      netlist<w3_op> circuit;
      wire::id q0 = circuit.create_qubit("q0");
      wire::id c0 = circuit.create_cbit("c0");

      angle theta = angles::pi_half;
      angle phi = angles::zero;
      angle lambda = angles::pi;
      circuit.create_op(gate_lib::u3(theta, phi, lambda), q0);

      circuit.create_op(gate_lib::measure_z, q0, c0);
      write_utf8(circuit);
  }

The output::

  c1 : ═══════════■═══
                  ║   
          ┌────┐┌─╨──┐
  q0 : ───┤ U3 ├┤ Mz ├
          └────┘└────┘