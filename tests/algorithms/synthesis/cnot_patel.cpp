/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt, Mathias Soeken, Fereshte Mozafari
*-------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/algorithms/synthesis/cnot_patel.hpp>
#include <tweedledum/gates/gate_set.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/gates/mcst_gate.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/utils/bit_matrix_rm.hpp>

using namespace tweedledum;
TEMPLATE_PRODUCT_TEST_CASE("CNOT patel synthesis", "[cnot_patel][template]",
                           (gg_network, netlist), (mcmt_gate, mcst_gate))
{
	std::vector<uint32_t> rows = {0b000011, 0b011001, 0b010010, 0b111111, 0b111011, 0b011100};
	bit_matrix_rm matrix(6, rows);
	SECTION("Check example from paper")
	{
		cnot_patel_params parameters;
		parameters.allow_rewiring = false;
		parameters.best_partition_size = false;
		parameters.partition_size = 2u;
		auto network = cnot_patel<TestType>(matrix, parameters);

		// Simulate CNOTs in network
		bit_matrix_rm id_matrix(6, 6);
		id_matrix.foreach_row([](auto& row, const auto row_index) { row[row_index] = 1; });

		network.foreach_node([&](auto const& node) {
			if (!node.gate.is(gate_set::cx)) {
				return;
			}
			id_matrix.row(node.gate.target()) ^= id_matrix.row(node.gate.control());
		});
		// Check if network realizes original matrix
		for (auto row_index = 0u; row_index < matrix.num_rows(); ++row_index) {
			CHECK(matrix.row(row_index) == id_matrix.row(row_index));
		}
	}
	SECTION("Find best permutation for the example from paper")
	{
		cnot_patel_params parameters;
		parameters.allow_rewiring = true;
		parameters.best_partition_size = true;
		auto network = cnot_patel<TestType>(matrix, parameters);

		// Simulate CNOTs in network
		bit_matrix_rm id_matrix(6, 6);
		id_matrix.foreach_row([](auto& row, const auto row_index) { row[row_index] = 1; });

		network.foreach_node([&](auto const& node) {
			if (!node.gate.is(gate_set::cx)) {
				return;
			}
			id_matrix.row(node.gate.target()) ^= id_matrix.row(node.gate.control());
		});
		matrix.permute_rows(network.rewire_map());

		// Check if network realizes original matrix
		for (auto row_index = 0u; row_index < matrix.num_rows(); ++row_index) {
			CHECK(matrix.row(row_index) == id_matrix.row(row_index));
		}
	}
}
