/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../networks/wire.hpp"
#include "../networks/node.hpp"

#include <vector>

namespace tweedledum {

/*! \brief 
 */
template<typename Network>
class rewire_view : public Network {
public:
	using op_type = typename Network::op_type;
	using node_type = typename Network::node_type;
	using dstrg_type = typename Network::dstrg_type;

	explicit rewire_view(Network& network)
	    : Network(network)
	{
	}

#pragma region Creating operations (using wire ids)
	node::id create_op(gate const& g, wire::id t)
	{
		return emplace_op(op_type(g, wire_to_wire_.at(t)));
	}

	node::id create_op(gate const& g, wire::id w0, wire::id w1)
	{
		w0 = w0.is_complemented() ? !wire_to_wire_.at(w0) : wire_to_wire_.at(w0);
		return emplace_op(op_type(g, w0, wire_to_wire_.at(w1)));
	}

	node::id create_op(gate const& g, wire::id c0, wire::id c1, wire::id t)
	{
		c0 = c0.is_complemented() ? !wire_to_wire_.at(c0) : wire_to_wire_.at(c0);
		c1 = c1.is_complemented() ? !wire_to_wire_.at(c1) : wire_to_wire_.at(c1);
		return emplace_op(op_type(g, c0, c1, wire_to_wire_.at(t)));
	}

	node::id create_op(gate const& g, std::vector<wire::id> controls, std::vector<wire::id> targets)
	{
		std::transform(controls.begin(), controls.end(), controls.begin(),
		               [&](wire::id id) -> wire::id {
			               const wire::id real_id = wire_to_wire_.at(id);
			               return id.is_complemented() ? !real_id : real_id;
		               });
		std::transform(targets.begin(), targets.end(), targets.begin(),
		               [&](wire::id id) -> wire::id { return wire_to_wire_.at(id); });
		return emplace_op(op_type(g, controls, targets));
	}
#pragma endregion

#pragma region Rewiring
	std::vector<wire::id> wire_to_wire_() const
	{
		return wire_to_wire_;
	}

	void rewire(std::vector<wire::id> const& new_wiring)
	{
		wire_to_wire_ = new_wiring;
	}

	void rewire(std::vector<std::pair<uint32_t, uint32_t>> const& transpositions)
	{
		for (auto&& [i, j] : transpositions) {
			std::swap(wire_to_wire_[i], wire_to_wire_[j]);
		}
	}
#pragma endregion

private:
	std::vector<wire::id> init_wire_to_wire_;
	std::vector<wire::id> wire_to_wire_;
};

} // namespace tweedledum
