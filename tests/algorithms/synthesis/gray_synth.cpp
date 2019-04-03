/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/algorithms/synthesis/gray_synth.hpp>
#include <tweedledum/gates/gate_set.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/gates/mcst_gate.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/utils/angle.hpp>
#include <tweedledum/utils/parity_terms.hpp>

using namespace tweedledum;
TEMPLATE_PRODUCT_TEST_CASE("Gray synthesis", "[gray_synth][template]",
                           (gg_network, netlist), (mcmt_gate, mcst_gate))
{
	SECTION("Check simple example from Amy paper")
	{
		parity_terms parities;
		parities.add_term(0b0110, angles::one_eighth);
		parities.add_term(0b0001, angles::one_eighth);
		parities.add_term(0b1001, angles::one_eighth);
		parities.add_term(0b0111, angles::one_eighth);
		parities.add_term(0b1011, angles::one_eighth);
		parities.add_term(0b0011, angles::one_eighth);

		auto network = gray_synth<TestType>(4, parities);
		bit_matrix_rm id_matrix(4, 4);
		id_matrix.foreach_row([](auto& row, const auto row_index) { row[row_index] = 1; });

		auto rewire_map = network.rewire_map();
		network.foreach_node([&](auto const& node) {
			if (node.gate.is(gate_set::cx)) {
				id_matrix.row(node.gate.target()) ^= id_matrix.row(
				    node.gate.control());
			}
		});
	}

	SECTION("Check with empty parities")
	{
		auto network = gray_synth<TestType>(4, {});
		CHECK(network.num_gates() == 0u);
		CHECK(network.num_qubits() == 4u);
	}
}
