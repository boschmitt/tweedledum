/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate_kinds.hpp"

#include <string> 
#include <vector>

namespace tweedledum {

/*! \brief Deletes all methods that can change the network.
 *
 * This view deletes all methods that can change the network structure such as
 * This view is convenient to use as a base class for other views that make some
 * computations based on the structure when being constructed. 
 */
template<typename Network>
class immutable_view : public Network {
public:
	using gate_type = typename Network::gate_type;
	using node_type = typename Network::node_type;
	using node_ptr_type = typename Network::node_ptr_type;
	using storage_type = typename Network::storage_type;

	/*! \brief Default constructor.
	 *
	 * Constructs immutable view on a network.
	 */
	immutable_view(Network const& ntk)
	    : Network(ntk)
	{}

	auto add_qubit() = delete;
	auto add_qubit(std::string const&) = delete;
	auto& add_gate(gate_type) = delete;
	auto& add_gate(gate_kinds_t, std::string const&) = delete;
	auto& add_gate(gate_kinds_t, uint32_t) = delete;
	auto& add_x_rotation(std::string const&, float) = delete;
	auto& add_z_rotation(std::string const&, float) = delete;
	auto& add_x_rotation(uint32_t, float) = delete;
	auto& add_z_rotation(uint32_t, float) = delete;
	auto& add_controlled_gate(gate_kinds_t, std::string const&, std::string const&) = delete;
	auto& add_controlled_gate(gate_kinds_t, uint32_t, uint32_t) = delete;
	auto& add_multiple_controlled_gate(gate_kinds_t, std::vector<std::string> const&) = delete;
	auto& add_multiple_controlled_gate(gate_kinds_t, std::vector<uint32_t> const&) = delete;
};

} // namespace tweedledum
