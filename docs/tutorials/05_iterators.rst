***************************
05 : Iterators
***************************

.. todo:: Writing

TLDR
-------------------

.. code-block:: c++

  #include <CLI11/CLI11.hpp>
  #include <fmt/core.h>
  #include <string>
  #include <tweedledum/tweedledum.hpp>

  int main()
  {
      using namespace tweedledum;
      CLI::App app("Analyse T gates");

      std::string filepath;
      app.add_option("-i", filepath, "Require an input file (*.qasm)")
          ->required()
          ->check(CLI::ExistingFile);

      CLI11_PARSE(app, argc, argv);

      auto circuit = read_qasm_from_file<netlist<w3_op>>(filepath);

      uint32_t num_t = 0u;
      circuit.foreach_op([&num_t](w3_op const& op) {
          if (op.is(gate_ids::t) || op.is(gate_ids::tdg)) {
              num_t = num_t + 1;
          }
      });
      fmt::print("Number of T gates: {}\n", num_t);

      
      uint32_t t_depth = 0u;
      using node_type = op_dag<w3_op>::node_type;
      circuit.foreach_op([&](w3_op const& op, node_type const& node) {
          uint32_t node_t_depth = 0u;
          circuit.foreach_child(node, [&](node_type const& child) {
              node_t_depth = std::max(node_t_depth, circuit.value(child));
          });
          circuit.value(node, node_t_depth);
          if (op.is(gate_ids::t) || op.is(gate_ids::tdg)) {
              circuit.incr_value(node);
          }
          t_depth = std::max(t_depth, circuit.value(node));
      });
      fmt::print("Circuit T depth: {}\n", t_depth);

      return EXIT_SUCCESS
  }

The circuit::

          ┌───┐                                                                   ┌───┐     ┌───┐                ┌────┐          ┌───┐     
  q4 : ───┤ X ├───────────────────────────────────────────────────────────────────┤ T ├─────┤ X ├───────●─────●──┤ T† ├───────●──┤ X ├─────
          └─┬─┘                                                                   └───┘     └─┬─┘       │     │  └────┘       │  └─┬─┘     
            │  ┌───┐     ┌───┐                ┌────┐          ┌───┐                           │         │     │               │    │       
  q3 : ─────●──┤ T ├─────┤ X ├───────●─────●──┤ T† ├───────●──┤ X ├───────────────────────────┼─────────┼─────┼───────────────┼────┼───────
               └───┘     └─┬─┘       │     │  └────┘       │  └─┬─┘                           │         │     │               │    │       
               ┌───┐┌───┐  │       ┌─┴─┐   │  ┌───┐      ┌─┴─┐  │  ┌───┐     ┌───┐┌───┐┌───┐  │       ┌─┴─┐   │  ┌───┐      ┌─┴─┐  │  ┌───┐
  q2 : ─────●──┤ H ├┤ T ├──┼────●──┤ X ├───┼──┤ T ├───●──┤ X ├──┼──┤ H ├──●──┤ X ├┤ H ├┤ T ├──┼────●──┤ X ├───┼──┤ T ├───●──┤ X ├──┼──┤ H ├
            │  └───┘└───┘  │    │  └───┘   │  └───┘   │  └───┘  │  └───┘  │  └───┘└───┘└───┘  │    │  └───┘   │  └───┘   │  └───┘  │  └───┘
          ┌─┴─┐┌───┐       │  ┌─┴─┐┌────┐┌─┴─┐┌────┐┌─┴─┐       │         │                   │    │          │          │         │       
  q1 : ───┤ X ├┤ T ├───────●──┤ X ├┤ T† ├┤ X ├┤ T† ├┤ X ├───────●─────────┼───────────────────┼────┼──────────┼──────────┼─────────┼───────
          └───┘└───┘          └───┘└────┘└───┘└────┘└───┘                 │                   │    │          │          │         │       
                                                                        ┌─┴─┐     ┌───┐       │  ┌─┴─┐┌────┐┌─┴─┐┌────┐┌─┴─┐       │       
  q0 : ─────────────────────────────────────────────────────────────────┤ X ├─────┤ T ├───────●──┤ X ├┤ T† ├┤ X ├┤ T† ├┤ X ├───────●───────
                                                                        └───┘     └───┘          └───┘└────┘└───┘└────┘└───┘               

The input OpenQASM::

  OPENQASM 2.0;
  include "qelib1.inc";
  qreg q[5];
  cx q[3], q[4];
  cx q[2], q[1];
  h q[2];
  t q[3];
  t q[1];
  t q[2];
  cx q[1], q[3];
  cx q[2], q[1];
  cx q[3], q[2];
  tdg q[1];
  cx q[3], q[1];
  tdg q[3];
  tdg q[1];
  t q[2];
  cx q[2], q[1];
  cx q[3], q[2];
  cx q[1], q[3];
  h q[2];
  cx q[2], q[0];
  x q[2];
  h q[2];
  t q[4];
  t q[0];
  t q[2];
  cx q[0], q[4];
  cx q[2], q[0];
  cx q[4], q[2];
  tdg q[0];
  cx q[4], q[0];
  tdg q[4];
  tdg q[0];
  t q[2];
  cx q[2], q[0];
  cx q[4], q[2];
  cx q[0], q[4];
  h q[2];

The output::

  Number of T gates: 14
  Circuit T depth: 6
