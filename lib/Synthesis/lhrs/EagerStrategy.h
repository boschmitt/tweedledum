/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "BaseStrategy.h"

#include <algorithm>
#include <mockturtle/networks/klut.hpp>
#include <mockturtle/utils/node_map.hpp>
#include <vector>

namespace tweedledum {

class EagerStrategy : public BaseStrategy {
    using LogicNetwork = mockturtle::klut_network;
    using Node = LogicNetwork::node;

public:
    bool compute_steps(LogicNetwork const& klut) override;

private:
    void cleanup(LogicNetwork const& klut,
        mockturtle::node_map<uint32_t, LogicNetwork>& ref_counts, Node node);
};

inline bool EagerStrategy::compute_steps(LogicNetwork const& klut)
{
    using namespace mockturtle;
    // Initialize reference counters
    node_map<uint32_t, LogicNetwork> ref_counts(klut, 0);
    klut.foreach_node([&](auto node) {
        ref_counts[node] = klut.fanout_size(node);
    });

    klut.clear_visited();
    std::vector<Node> outputs;
    klut.foreach_po([&](auto const& signal) {
        auto node = klut.get_node(signal);
        outputs.push_back(node);
        klut.set_visited(node, 1u);
    });

    this->steps_.reserve(klut.size() * 2);
    klut.foreach_node([&](auto node) {
        if (klut.is_constant(node) || klut.is_pi(node)) {
            return true;
        }

        steps_.emplace_back(Action::compute, node);
        if (!klut.visited(node)) {
            cleanup(klut, ref_counts, node);
        }
        return true;
    });
    return true;
}

inline void EagerStrategy::cleanup(LogicNetwork const& klut,
    mockturtle::node_map<uint32_t, LogicNetwork>& ref_counts, Node node)
{
    klut.foreach_fanin(node, [&](auto const& input) {
        const auto child = klut.get_node(input);
        if (klut.is_constant(child) || klut.is_pi(child)) {
            return;
        }

        if (--ref_counts[child] == 0u) {
            steps_.emplace_back(Action::cleanup, node);
            cleanup(klut, ref_counts, child);
        }
    });
}

} // namespace tweedledum
