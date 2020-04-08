/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate.hpp"
#include "../networks/node.hpp"
#include "../networks/wire.hpp"

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

	wire::id create_qubit(std::string_view name, wire::modes const mode = wire::modes::inout) = delete;
	wire::id create_qubit(wire::modes const mode = wire::modes::inout) = delete;
	wire::id create_cbit(std::string_view name, wire::modes const mode = wire::modes::inout) = delete;
	wire::id create_cbit(wire::modes const mode = wire::modes::inout) = delete;
	void wire_name(wire::id const w_id, std::string_view new_name, bool const rename = true) = delete;
	void wire_mode(wire::id const w_id, wire::modes const new_mode) = delete;

	template<typename Op>
	node::id emplace_op(Op&& op) = delete;
	node::id create_op(gate const& g, wire::id const t) = delete;
	node::id create_op(gate const& g, wire::id const w0, wire::id const w1) = delete;
	node::id create_op(gate const& g, wire::id const c0, wire::id const c1, wire::id const t) = delete;
	node::id create_op(gate const& g, std::vector<wire::id> const& controls,
	                   std::vector<wire::id> const& targets)
	    = delete;
};

} // namespace tweedledum
