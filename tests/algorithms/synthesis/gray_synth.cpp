/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/algorithms/synthesis/gray_synth.hpp>
#include <tweedledum/gates/mcst_gate.hpp>
#include <tweedledum/io/write_unicode.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/utils/angle.hpp>
#include <tweedledum/utils/parity_terms.hpp>

TEST_CASE("Check simple example from Amy paper", "[gray_synth]")
{
	using namespace tweedledum;
	parity_terms parities;
	parities.add_term(0b0110, symbolic_angles::one_eighth);
	parities.add_term(0b0001, symbolic_angles::one_eighth);
	parities.add_term(0b1001, symbolic_angles::one_eighth);
	parities.add_term(0b0111, symbolic_angles::one_eighth);
	parities.add_term(0b1011, symbolic_angles::one_eighth);
	parities.add_term(0b0011, symbolic_angles::one_eighth);

	auto network = gray_synth<netlist<mcst_gate>>(4, parities);
	write_unicode(network);

	bit_matrix_rm id_matrix(4, 4);
	id_matrix.foreach_row([](auto& row, const auto row_index){
		row[row_index] = 1;
	});

	auto rewire_map = network.rewire_map();
	network.foreach_cnode([&](auto const& node) {
		if (node.gate.is(gate_set::cx)) {
			uint32_t c;
			uint32_t t;
			node.gate.foreach_control([&](auto _c) { c = _c; });
			node.gate.foreach_target([&](auto _t) { t = _t; });
			id_matrix.row(t) ^= id_matrix.row(c);
		}
	});
	id_matrix.print();
}

TEST_CASE("Check with empty parities", "[gray_synth]")
{
	using namespace tweedledum;
	auto network = gray_synth<netlist<mcst_gate>>(4, {});

	CHECK(network.num_gates() == 0u);
	CHECK(network.num_qubits() == 4u);
}
