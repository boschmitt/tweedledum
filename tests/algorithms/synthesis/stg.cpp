/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <sstream>
#include <tweedledum/algorithms/simulation/simulate_classically.hpp>
#include <tweedledum/algorithms/synthesis/stg.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/io_id.hpp>
#include <tweedledum/networks/netlist.hpp>

namespace tweedledum::detail {

template<class Network>
inline auto circuit_and_map(uint32_t qubits)
{
	Network network;
	for (auto i = 0u; i < qubits; ++i) {
		network.add_qubit();
	}
	return std::make_pair(network, network.rewire_map());
}

} // namespace tweedledum::detail

using namespace tweedledum;
TEMPLATE_PRODUCT_TEST_CASE("Single-target gate synthesis", "[stg][template]", (gg_network, netlist),
                           (mcmt_gate))
{
	SECTION("Synthesize using stg_from_pkrm")
	{
		kitty::dynamic_truth_table tt(5);
		kitty::create_from_hex_string(tt, "DA657041");
		auto [network, map] = detail::circuit_and_map<TestType>(6u);
		stg_from_pkrm()(network, map, tt);
		for (auto i = 0ull; i < tt.num_bits(); ++i) {
			auto expected_output = i;
			if (kitty::get_bit(tt, i)) {
				expected_output |= (1ull << tt.num_vars());
			}
			CHECK(simulate_classically(network, i) == expected_output);
		}
	}
	SECTION("Synthesize using stg_from_pprm")
	{
		kitty::dynamic_truth_table tt(5);
		kitty::create_from_hex_string(tt, "DA657041");
		auto [network, map] = detail::circuit_and_map<TestType>(6u);
		stg_from_pprm()(network, map, tt);
		for (auto i = 0ull; i < tt.num_bits(); ++i) {
			auto expected_output = i;
			if (kitty::get_bit(tt, i)) {
				expected_output |= (1ull << tt.num_vars());
			}
			CHECK(simulate_classically(network, i) == expected_output);
		}
	}
	SECTION("Synthesize stg_from_spectrum")
	{
		kitty::dynamic_truth_table tt(5);
		kitty::create_from_hex_string(tt, "DA657041");
		auto [network, map] = detail::circuit_and_map<TestType>(6u);
		stg_from_spectrum()(network, map, tt);
		CHECK(network.num_gates() == 108u);
		CHECK(network.num_qubits() == 6u);
	}
}