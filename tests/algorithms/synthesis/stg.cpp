/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <sstream>
#include <tweedledum/algorithms/synthesis/stg.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/networks/io_id.hpp>

namespace tweedledum::detail {

template<class Network>
inline auto circuit_and_map(uint32_t qubits)
{
	Network network;
	for (auto i = 0u; i < qubits; ++i) {
		network.add_qubit();
	}
	std::vector<io_id> map(qubits);
	std::iota(map.begin(), map.end(), 0u);
	return std::make_pair(network, map);
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
		CHECK(network.num_gates() == 10u);
		CHECK(network.num_qubits() == 6u);
	}
	SECTION("Synthesize using stg_from_pprm")
	{
		kitty::dynamic_truth_table tt(5);
		kitty::create_from_hex_string(tt, "DA657041");
		auto [network, map] = detail::circuit_and_map<TestType>(6u);
		stg_from_pprm()(network, map, tt);
		CHECK(network.num_gates() == 21u);
		CHECK(network.num_qubits() == 6u);
	}
	SECTION("Synthesize stg_from_spectrum")
	{
		kitty::dynamic_truth_table tt(5);
		kitty::create_from_hex_string(tt, "DA657041");
		auto [network, map] = detail::circuit_and_map<TestType>(6u);
		stg_from_spectrum()(network, map, tt);
		CHECK(network.num_gates() == 185u);
		CHECK(network.num_qubits() == 6u);
	}
}

// // TODO: improve test case
// SECTION("Synthesize a simple function into a quantum network using stg_from_exact_esop",
//           "[stg]")
// {
// 	kitty::dynamic_truth_table tt(3);
// 	kitty::create_from_binary_string(tt, "10000001");
// 	auto [network, map] = detail::circuit_and_map<netlist<mcmt_gate>>(4u);
// 	stg_from_exact_esop()(network, map, tt);

// 	CHECK(network.num_gates() == 8u);
// 	CHECK(network.num_qubits() == 4u);

// 	// for (auto i = 0ull; i < tt.num_bits(); ++i) {
// 	// 	auto expected_output = i;
// 	// 	if (kitty::get_bit(tt, i)) {
// 	// 		expected_output |= 1 << tt.num_vars();
// 	// 	}
// 	// 	CHECK(simulate_pattern_classical(network, i) == expected_output);
// 	// }
// }

// SECTION("Synthesize a simple function into a quantum network using stg_from_pkrm",
//           "[stg]")
// {
// 	kitty::dynamic_truth_table tt(5);
// 	kitty::create_from_hex_string(tt, "DA657041");
// 	auto [network, map] = detail::circuit_and_map<netlist<mcmt_gate>>(6u);
// 	stg_from_pkrm()(network, map, tt);

// 	CHECK(network.num_gates() == 10u);
// 	CHECK(network.num_qubits() == 6u);
// }

// TEST_CASE("Synthesize a simple function into a quantum network using stg_from_pprm",
//           "[stg]")
// {
// 	kitty::dynamic_truth_table tt(5);
// 	kitty::create_from_hex_string(tt, "DA657041");
// 	auto [network, map] = detail::circuit_and_map<netlist<mcmt_gate>>(6u);
// 	stg_from_pprm()(network, map, tt);

// 	CHECK(network.num_gates() == 21u);
// 	CHECK(network.num_qubits() == 6u);
// }

// TEST_CASE("Synthesize a simple function into a quantum network using stg_from_spectrum",
//           "[stg]")
// {
// 	kitty::dynamic_truth_table tt(5);
// 	kitty::create_from_hex_string(tt, "DA657041");
// 	auto [network, map] = detail::circuit_and_map<netlist<mcmt_gate>>(6u);
// 	stg_from_spectrum()(network, map, tt);

// 	CHECK(network.num_gates() == 185u);
// 	CHECK(network.num_qubits() == 6u);
// }
