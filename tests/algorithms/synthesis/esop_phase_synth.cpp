/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#include "tweedledum/algorithms/synthesis/esop_phase_synth.hpp"

#include "tweedledum/gates/wn32_op.hpp"
#include "tweedledum/networks/netlist.hpp"
#include "tweedledum/networks/op_dag.hpp"

#include <catch.hpp>
#include <kitty/constructors.hpp>
#include <kitty/cube.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <kitty/operators.hpp>

using namespace tweedledum;

TEMPLATE_PRODUCT_TEST_CASE("Synthesize phase circuits", "[esop_phase_synth][template]",
                           (op_dag, netlist), (wn32_op))
{
	for (uint32_t n = 2u; n <= 10u; ++n) {
		for (uint32_t t = 0u; t <= 50u; ++t) {
			kitty::dynamic_truth_table func(5u);
			kitty::create_random(func);
			const auto network = esop_phase_synth<TestType>(func);
			std::vector<kitty::cube> cubes;
			network.foreach_op([&](auto const& node) {
				CHECK(node.operation.gate.is(gate_ids::ncz));
				uint32_t bits = 0u;
				node.operation.foreach_control([&](auto qubit) { bits |= 1 << qubit; });
				node.operation.foreach_target([&](auto qubit) { bits |= 1 << qubit; });
				cubes.emplace_back(bits, bits);
			});

			auto func2 = func.construct();
			kitty::create_from_cubes(func2, cubes, true);
			CHECK(func == (kitty::get_bit(func, 0) ? ~func2 : func2));
		}
	}
}
