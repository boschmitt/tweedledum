***************************
03 : N-qubit gates
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
      wire::id q1 = circuit.create_qubit("q1");
      wire::id q2 = circuit.create_qubit("q2");
      wire::id c0 = circuit.create_cbit("c0");
      circuit.create_op(gate_lib::ncx, q1, q2, q0);
      circuit.create_op(gate_lib::measure_z, q0, c0);
      write_utf8(circuit);
  }

The output::

  c0 : ══════════■═══
                 ║   
                 ║   
  q2 : ─────●────╫───
            │    ║   
            │    ║   
  q1 : ─────●────╫───
            │    ║   
          ┌─┴─┐┌─╨──┐
  q0 : ───┤ X ├┤ Mz ├
          └───┘└────┘