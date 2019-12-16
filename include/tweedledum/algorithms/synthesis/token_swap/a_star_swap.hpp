/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../../utils/device.hpp"
#include "../../../utils/hash.hpp"
#include "parameters.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace tweedledum {
namespace detail {

class a_star_swapper {
private:
	using map_type = std::vector<uint32_t>;
	using swap_type = std::pair<uint32_t, uint32_t>;

	struct node_type {
		map_type mapping;
		uint32_t swap;
		uint32_t previous;
		uint32_t g;
		uint32_t h;
		bool closed;

		node_type(map_type const& mapping_, uint32_t edge, uint32_t prev, uint32_t g_, uint32_t h_)
		    : mapping(mapping_)
		    , swap(edge)
		    , previous(prev)
		    , g(g_)
		    , h(h_)
		    , closed(false)
		{}
	};

public:
	explicit a_star_swapper(device const& topology)
	: topology_(topology)
	{}

	std::vector<swap_type> run(map_type const& init_mapping,
	                           map_type const& final_mapping,
				   bool admissable = true)
	{
		std::vector<node_type> nodes;
		std::vector<uint32_t> open_nodes;
		std::vector<uint32_t> closed_nodes;
		std::unordered_map<map_type, uint32_t, hash<map_type>> mappings;

		mappings.emplace(init_mapping, 0);
		nodes.emplace_back(init_mapping, 0, 0, 0, 0);
		open_nodes.emplace_back(0);
		while (!open_nodes.empty()) {
			node_type node = nodes.at(open_nodes.back());
			closed_nodes.push_back(open_nodes.back());
			nodes.at(open_nodes.back()).closed = true;
			if (node.mapping == final_mapping) {
				break;
			}
			open_nodes.pop_back();
			assert(node.mapping.size() == init_mapping.size());
			for (uint32_t i = 0; i < topology_.num_edges(); ++i) {
				map_type new_mapping = node.mapping;
				std::swap(new_mapping.at(topology_.edges[i].first),
					  new_mapping.at(topology_.edges[i].second));

				// Try to add new mapping to the mappings database
				auto [it, was_added] = mappings.emplace(new_mapping, nodes.size());
				node_type& new_node = was_added ?
				                      nodes.emplace_back(new_mapping, i, closed_nodes.back(), node.g + 1, 0) :
						      nodes.at(it->second);

				if (was_added) {
					// If a new node was created, need to add to the list of nodes
					open_nodes.push_back(nodes.size() - 1);
				} else if (new_node.g <= node.g + 1) {
					// Do not update a node if its new cost exeeds the previous one
					continue;
				} else if (new_node.closed) {
					// If new cost is smaller and the node was already closed, re-open it!
					new_node = nodes.emplace_back(new_mapping, i, closed_nodes.back(), node.g + 1, 0);
					open_nodes.push_back(nodes.size() - 1);
					mappings.at(new_mapping) = nodes.size() - 1;
				}
				for (auto k = 0ull; k < final_mapping.size(); ++k) {
					if (new_node.mapping[k] != final_mapping[k]) {
						auto it = std::find(final_mapping.begin(),
								    final_mapping.end(),
								    new_node.mapping[k]);
						auto idx = std::distance(final_mapping.begin(), it);
						new_node.h += topology_.distance(k, idx);
					}
				}
				if (admissable) {
					new_node.h = std::ceil(new_node.h / 2.0);
				}
			}
			auto min_it = std::min_element(open_nodes.begin(), open_nodes.end(),
						       [&](auto a_idx, auto b_idx) {
							       auto& a = nodes[a_idx];
							       auto& b = nodes[b_idx];
							       return (a.g + a.h) < (b.g + b.h);
						       });
			std::swap(*min_it, open_nodes.back());
		}

		// Reconstruc sequence of swaps
		std::vector<swap_type> swaps;
		auto& node = nodes.at(closed_nodes.back());
		while (node.previous) {
			swaps.emplace_back(topology_.edges[node.swap]);
			node = nodes.at(node.previous);
		}
		if (closed_nodes.size() > 1) {
			swaps.push_back(topology_.edges[node.swap]);
		}
		std::reverse(swaps.begin(), swaps.end());
		return swaps;
	}

private:
	device const& topology_;
};

auto a_star_swap(device const& topology, std::vector<uint32_t> const& init_mapping, std::vector<uint32_t> const& final_mapping, swap_network_params params)
{
	a_star_swapper swapper(topology);
	if (params.method == swap_network_params::methods::non_admissible) {
		return swapper.run(init_mapping, final_mapping, false);
	}
	return swapper.run(init_mapping, final_mapping);
}

} // namespace detail
} // namespace tweedledum
