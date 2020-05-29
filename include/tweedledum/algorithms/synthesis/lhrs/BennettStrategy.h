/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "BaseStrategy.h"

#include <algorithm>
#include <vector>

namespace tweedledum {

class BennettStrategy : public BaseStrategy {
public:
	bool compute_steps(mockturtle::klut_network const& network) override;
};

inline bool BennettStrategy::compute_steps(
    mockturtle::klut_network const& network)
{
	std::vector<Node> outputs;
	network.foreach_po([&](auto const& signal) {
		auto node = network.get_node(signal);
		outputs.push_back(node);
		network.set_visited(node, 1u);
	});

	this->steps_.reserve(network.size() * 2);
	auto it = this->steps_.begin();
	network.foreach_node([&](auto node) {
		if (network.is_constant(node) || network.is_pi(node)) {
			return true;
		}

		it = this->steps_.emplace(it, Action::compute, node);
		++it;

		if (!network.visited(node)) {
			it = this->steps_.emplace(it, Action::cleanup, node);
		}
		return true;
	});
	return true;
}

} // namespace tweedledum
