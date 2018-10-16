/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken, Fereshte Mozafari
*-----------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/algorithms/synthesis/cnot_patel.hpp>
#include <tweedledum/gates/gate_kinds.hpp>
#include <tweedledum/gates/mcst_gate.hpp>
#include <tweedledum/networks/gg_network.hpp>

TEST_CASE("Check example from paper", "cnot_patel")
{
	using namespace tweedledum;
	gg_network<mcst_gate> network;
	std::vector<uint32_t> matrix{
	    {0b000011, 0b011001, 0b010010, 0b111111, 0b111011, 0b011100}};
	auto matrix_orig = matrix;
	cnot_patel(network, matrix, 2);

	/* after algorithm, matrix is identity matrix */
	for (auto j = 0u; j < matrix.size(); ++j) {
		CHECK(matrix[j] == 1 << j);
	}

	/* simulate CNOTs in network */
	network.foreach_node([&](auto const& n) {
		if (n.gate.is(gate_kinds_t::cx)) {
			uint32_t c, t;
			n.gate.foreach_control([&](auto _c) { c = _c; });
			n.gate.foreach_target([&](auto _t) { t = _t; });
			matrix[t] ^= matrix[c];
		}
	});

	/* network realizes original matrix? */
	for (auto j = 0u; j < matrix.size(); ++j) {
		CHECK(matrix_orig[j] == matrix[j]);
	}
}
