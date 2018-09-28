/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../traits.hpp"
#include "../utils/node_map.hpp"
#include "immutable_view.hpp"

#include <vector>

namespace tweedledum {

/*! \brief Implements `depth` and `get_level` methods for networks.
 *
 * This view computes the level of each node and also the depth of the network. It implements the
 * network interface methods `get_level` and `depth`. The levels are computed at construction and
 * can be recomputed by calling the `update` method.
 *
 * **Required gate functions:**
 * - `is`
 * 
 * **Required network functions:**
 * - `get_node`
 * - `clear_marks`
 * - `mark`
 * - `foreach_child`
 * - `foreach_output`
 */
template<typename Network>
class depth_view : public immutable_view<Network> {
public:
	using gate_type = typename Network::gate_type;
	using node_type = typename Network::node_type;
	using node_ptr_type = typename Network::node_ptr_type;
	using storage_type = typename Network::storage_type;

	explicit depth_view(Network& ntk)
	    : immutable_view<Network>(ntk)
	    , levels_(ntk)
	{
		update();
	}

	/*! \brief Returns the length of the critical path. */
	uint32_t depth() const
	{
		return depth_;
	}

	/*! \brief Returns the level of a node. */
	uint32_t level(node_type const& node) const
	{
		return levels_[node];
	}

	void update()
	{
		levels_.reset(0);
		compute_levels();
		this->clear_marks();
	}

private:
	auto compute_levels(node_type const& node)
	{
		if (this->mark(node)) {
			return levels_[node];
		}

		if (node.gate.is(gate_kinds_t::input)) {
			return levels_[node] = 0;
		}

		uint32_t level = 0;
		this->foreach_child(node, [&](auto child_index) {
			level = std::max(level, compute_levels(this->get_node(child_index)));
		});

		this->mark(node, 1);
		return levels_[node] = level + 1;
	}

	void compute_levels()
	{
		depth_ = 0;
		this->foreach_output(
		    [&](auto const& node) { depth_ = std::max(depth_, compute_levels(node)); });
	}

private:
	node_map<uint32_t, Network> levels_;
	uint32_t depth_;
};

} // namespace tweedledum
