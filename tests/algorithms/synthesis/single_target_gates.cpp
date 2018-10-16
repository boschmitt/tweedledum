/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <sstream>
#include <tweedledum/algorithms/synthesis/single_target_gates.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/gates/mcst_gate.hpp>
#include <tweedledum/networks/gdg_network.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/netlist.hpp>

using namespace tweedledum;

namespace tweedledum::detail {

template<class Network>
inline std::pair<Network, std::vector<uint32_t>> circuit_and_map(uint32_t qubits)
{
	Network circ;
	for (auto i = 0u; i < qubits; ++i) {
		circ.add_qubit();
	}
	std::vector<uint32_t> map(qubits);
	std::iota(map.begin(), map.end(), 0u);
	return {circ, map};
}

} // namespace tweedledum::detail

TEST_CASE("Synthesize a simple function into a quantum network using stg_from_pprm",
          "[single_target_gates]")
{
	kitty::dynamic_truth_table tt(5);
	kitty::create_from_hex_string(tt, "DA657041");
	auto [circ, map] = detail::circuit_and_map<gg_network<mcmt_gate>>(6u);
	stg_from_pprm()(circ, tt, map);

	CHECK(circ.num_gates() == 21u);
	CHECK(circ.num_qubits() == 6u);
}

TEST_CASE("Synthesize a simple function into a quantum network using stg_from_pkrm",
          "[single_target_gates]")
{
	kitty::dynamic_truth_table tt(5);
	kitty::create_from_hex_string(tt, "DA657041");
	auto [circ, map] = detail::circuit_and_map<gg_network<mcmt_gate>>(6u);
	stg_from_pkrm()(circ, tt, map);

	CHECK(circ.num_gates() == 38u);
	CHECK(circ.num_qubits() == 6u);
}

TEST_CASE("Synthesize a simple function into a quantum network using stg_from_spectrum",
          "[single_target_gates]")
{
	kitty::dynamic_truth_table tt(5);
	kitty::create_from_hex_string(tt, "DA657041");
	auto [circ, map] = detail::circuit_and_map<gg_network<mcmt_gate>>(6u);
	stg_from_spectrum()(circ, tt, map);

	CHECK(circ.num_gates() == 185u);
	CHECK(circ.num_qubits() == 6u);
}
