/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken, Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <kitty/constructors.hpp>
#include <kitty/cube.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <kitty/operators.hpp>
#include <tweedledum/algorithms/synthesis/esop_phase_synth.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/netlist.hpp>

using namespace tweedledum;
TEMPLATE_PRODUCT_TEST_CASE("Synthesize phase circuits", "[esop_phase_synth][template]",
                           (gg_network, netlist), (mcmt_gate))
{
	for (auto n = 2u; n <= 10u; ++n) {
		for (auto t = 0u; t <= 50u; ++t) {
			kitty::dynamic_truth_table func(5u);
			kitty::create_random(func);
			const auto network = esop_phase_synth<TestType>(func);
			std::vector<kitty::cube> cubes;
			network.foreach_cgate([&](auto& node) {
				CHECK(node.gate.is(gate_set::mcz));
				uint32_t bits = 0u;
				node.gate.foreach_control([&](auto qubit) { bits |= 1 << qubit; });
				node.gate.foreach_target([&](auto qubit) { bits |= 1 << qubit; });
				cubes.emplace_back(bits, bits);
			});

			auto func2 = func.construct();
			kitty::create_from_cubes(func2, cubes, true);
			CHECK(func == (kitty::get_bit(func, 0) ? ~func2 : func2));
		}
	}
}
