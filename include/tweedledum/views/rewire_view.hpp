/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../networks/wire_id.hpp"

#include <vector>

namespace tweedledum {

/*! \brief 
 */
template<typename Network>
class rewire_view : public Network {
public:
	using op_type = typename Network::op_type;
	using node_type = typename Network::node_type;
	using storage_type = typename Network::storage_type;

	explicit rewire_view(Network& network)
	    : Network(network)
	{
	}

#pragma region Creating operations (using wire ids)
	node_id create_op(gate const& g, wire_id t)
	{
		return emplace_op(op_type(g, wire_to_wire_.at(t)));
	}

	node_id create_op(gate const& g, wire_id w0, wire_id w1)
	{
		w0 = w0.is_complemented() ? !wire_to_wire_.at(w0) : wire_to_wire_.at(w0);
		return emplace_op(op_type(g, w0, wire_to_wire_.at(w1)));
	}

	node_id create_op(gate const& g, wire_id c0, wire_id c1, wire_id t)
	{
		c0 = c0.is_complemented() ? !wire_to_wire_.at(c0) : wire_to_wire_.at(c0);
		c1 = c1.is_complemented() ? !wire_to_wire_.at(c1) : wire_to_wire_.at(c1);
		return emplace_op(op_type(g, c0, c1, wire_to_wire_.at(t)));
	}

	node_id create_op(gate const& g, std::vector<wire_id> controls, std::vector<wire_id> targets)
	{
		std::transform(controls.begin(), controls.end(), controls.begin(),
		               [&](wire_id id) -> wire_id {
			               const wire_id real_id = wire_to_wire_.at(id);
			               return id.is_complemented() ? !real_id : real_id;
		               });
		std::transform(targets.begin(), targets.end(), targets.begin(),
		               [&](wire_id id) -> wire_id { return wire_to_wire_.at(id); });
		return emplace_op(op_type(g, controls, targets));
	}
#pragma endregion

#pragma region Rewiring
	std::vector<wire_id> wire_to_wire_() const
	{
		return wire_to_wire_;
	}

	void rewire(std::vector<wire_id> const& new_wiring)
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
	std::vector<wire_id> init_wire_to_wire_;
	std::vector<wire_id> wire_to_wire_;
};

} // namespace tweedledum
