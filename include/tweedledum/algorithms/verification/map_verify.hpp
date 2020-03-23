/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../networks/mapped_dag.hpp"

#include <algorithm>
#include <limits>
#include <vector>

namespace tweedledum {

#pragma region Implementation detail
namespace detail {
using sum_type = std::vector<uint32_t>;

// Fake pathsums can be employed to verify mappings.  Assuming that the mapping do not
// screw-up adding single qubit gates, we can safely ignore them and verify if the set of
// output path sums of the original circuit mataches the set of output path sums of the
// mapped circuit.
//
// The user need to pass the _initial_ virtual->physical mapping so that the path literals
// can be placed correctly.
template<typename Network>
std::vector<sum_type> fake_pathsums(Network const& network, std::vector<wire_id> const& init)
{
	assert(init.size() == network.num_qubits());
	using node_type = typename Network::node_type;
	constexpr uint32_t qid_max = std::numeric_limits<uint32_t>::max();

	std::vector<uint32_t> wire_to_qid(network.num_wires(), qid_max);
	std::vector<sum_type> fake_pathsum;

	network.foreach_input([&](node_type const& node) {
		wire_id w_id = node.operation.target();
		if (!w_id.is_qubit()) {
			return;
		};
		wire_to_qid.at(w_id) = fake_pathsum.size();
		fake_pathsum.emplace_back(1u, init.at(fake_pathsum.size()));
	});
	network.foreach_op([&](auto const& node) {
		auto const& op = node.operation;
		if (!op.gate.is_two_qubit()) {
			return;
		}
		uint32_t t_qid = wire_to_qid.at(op.target());
		if (op.gate.is(gate_ids::swap)) {
			uint32_t t1_qid = wire_to_qid.at(op.target(1u));
			std::swap(fake_pathsum.at(t_qid), fake_pathsum.at(t1_qid));
			return;
		}
		uint32_t c_qid = wire_to_qid.at(op.control());
		std::vector<uint32_t> result;
		std::set_symmetric_difference(fake_pathsum.at(c_qid).begin(),
		                              fake_pathsum.at(c_qid).end(),
		                              fake_pathsum.at(t_qid).begin(),
		                              fake_pathsum.at(t_qid).end(),
		                              std::back_inserter(result));
		fake_pathsum.at(t_qid) = result;
	});
	return fake_pathsum;
}
} // namespace detail
#pragma endregion

template<typename Network>
bool map_verify(Network const& original, mapped_dag const& mapped)
{
	std::vector<wire_id> init_original(original.num_qubits(), wire::invalid);
	for (uint32_t i = 0u; i < init_original.size(); ++i) {
		init_original.at(i) = wire_id(i, true);
	}
	auto original_pathsums = detail::fake_pathsums(original, init_original);
	auto mapped_pathsums = detail::fake_pathsums(mapped, mapped.init_phy_to_v());
	return std::is_permutation(original_pathsums.begin(), original_pathsums.end(),
	                           mapped_pathsums.begin());
}

} // namespace tweedledum
