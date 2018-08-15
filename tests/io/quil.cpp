/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*-----------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/io/quil.hpp>
#include <tweedledum/networks/dag_path.hpp>
#include <tweedledum/networks/gates/qc_gate.hpp>
#include <tweedledum/networks/gates/gate_kinds.hpp>

#include <sstream>

TEST_CASE("Write MCT into QUIL", "quil")
{
	using namespace tweedledum;
	dag_path<qc_gate> network;
	network.allocate_qubit();
	network.allocate_qubit();
	network.allocate_qubit();
  network.add_multiple_controlled_gate(gate_kinds_t::cx, {2, 0, 1});

  std::ostringstream os;
  write_quil(network, os);
  CHECK(os.str() == "CCNOT 0 1 2\n");
}
