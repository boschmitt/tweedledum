/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt, Mathias Soeken, Fereshte Mozafari
*-------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/algorithms/synthesis/cnot_patel.hpp>
#include <tweedledum/gates/gate_set.hpp>
#include <tweedledum/gates/mcst_gate.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/utils/bit_matrix_rm.hpp>

TEST_CASE("Check example from paper", "[cnot_patel]")
{
	using namespace tweedledum;
	std::vector<uint32_t> rows = {0b000011, 0b011001, 0b010010, 0b111111, 0b111011, 0b011100};
	bit_matrix_rm matrix(6, rows);

	cnot_patel_params parameters;
	parameters.allow_rewiring = false;
	parameters.best_partition_size = false;
	parameters.partition_size = 2u;
	auto network = cnot_patel<netlist<mcst_gate>>(matrix, parameters);

	// Simulate CNOTs in network
	bit_matrix_rm id_matrix(6, 6);
	id_matrix.foreach_row([](auto& row, const auto row_index){
		row[row_index] = 1;
	});

	network.foreach_cnode([&](auto const& node) {
		if (node.gate.op().is(gate_set::cx)) {
			uint32_t c;
			uint32_t t;
			node.gate.foreach_control([&](auto _c) { c = _c; });
			node.gate.foreach_target([&](auto _t) { t = _t; });
			id_matrix.row(t) ^= id_matrix.row(c);
		}
	});

	// Check if network realizes original matrix
	for (auto row_index = 0u; row_index < matrix.num_rows(); ++row_index) {
		CHECK(matrix.row(row_index) == id_matrix.row(row_index));
	}
}

TEST_CASE("Find best permutation for the example from paper", "[cnot_patel]")
{
	using namespace tweedledum;
	std::vector<uint32_t> rows = {0b000011, 0b011001, 0b010010, 0b111111, 0b111011, 0b011100};
	bit_matrix_rm matrix(6, rows);

	cnot_patel_params parameters;
	parameters.allow_rewiring = true;
	parameters.best_partition_size = true;
	auto network = cnot_patel<netlist<mcst_gate>>(matrix, parameters);

	// Simulate CNOTs in network
	bit_matrix_rm id_matrix(6, 6);
	id_matrix.foreach_row([](auto& row, const auto row_index){
		row[row_index] = 1;
	});

	auto rewire_map = network.rewire_map();
	network.foreach_cnode([&](auto const& node) {
		if (node.gate.op().is(gate_set::cx)) {
			uint32_t c;
			uint32_t t;
			node.gate.foreach_control([&](auto _c) { c = _c; });
			node.gate.foreach_target([&](auto _t) { t = _t; });
			id_matrix.row(t) ^= id_matrix.row(c);
		}
	});
	matrix.permute_rows(rewire_map);

	// Check if network realizes original matrix
	for (auto row_index = 0u; row_index < matrix.num_rows(); ++row_index) {
		CHECK(matrix.row(row_index) == id_matrix.row(row_index));
	}
}
