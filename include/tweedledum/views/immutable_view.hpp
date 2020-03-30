/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate.hpp"
#include "../networks/storage.hpp"
#include "../networks/wire_id.hpp"

namespace tweedledum {

/*! \brief Deletes all methods that can change the network.
 *
 * This view deletes all methods that can change the network structure
 * This view is convenient to use as a base class for other views that make some computations based
 * on the structure when being constructed.
 */
template<typename Network>
class immutable_view : public Network {
public:
	using op_type = typename Network::op_type;
	using node_type = typename Network::node_type;
	using dstrg_type = typename Network::dstrg_type;

	/*! \brief Default constructor.
	 *
	 * Constructs immutable view on a network.
	 */
	immutable_view(Network const& network)
	    : Network(network)
	{}

	wire_id create_qubit(std::string_view name, wire_modes const mode = wire_modes::inout) = delete;
	wire_id create_qubit(wire_modes const mode = wire_modes::inout) = delete;
	wire_id create_cbit(std::string_view name, wire_modes const mode = wire_modes::inout) = delete;
	wire_id create_cbit(wire_modes const mode = wire_modes::inout) = delete;
	void wire_name(wire_id const w_id, std::string_view new_name, bool const rename = true) = delete;
	void wire_mode(wire_id const w_id, wire_modes const new_mode) = delete;

	template<typename Op>
	node_id emplace_op(Op&& op) = delete;
	node_id create_op(gate const& g, wire_id const t) = delete;
	node_id create_op(gate const& g, wire_id const w0, wire_id const w1) = delete;
	node_id create_op(gate const& g, wire_id const c0, wire_id const c1, wire_id const t) = delete;
	node_id create_op(gate const& g, std::vector<wire_id> const& controls,
	                  std::vector<wire_id> const& targets)
	    = delete;
};

} // namespace tweedledum
