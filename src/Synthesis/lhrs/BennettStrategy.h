/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "BaseStrategy.h"

#include <algorithm>
#include <mockturtle/networks/klut.hpp>
#include <vector>

namespace tweedledum {

class BennettStrategy : public BaseStrategy {
public:
    bool compute_steps(mockturtle::klut_network const& klut) override;
};

inline bool BennettStrategy::compute_steps(mockturtle::klut_network const& klut)
{
    std::vector<Node> outputs;
    klut.clear_visited();
    klut.foreach_po([&](auto const& signal) {
        auto node = klut.get_node(signal);
        outputs.push_back(node);
        klut.set_visited(node, 1u);
    });

    this->steps_.reserve(klut.size() * 2);
    auto it = this->steps_.begin();
    klut.foreach_node([&](auto node) {
        if (klut.is_constant(node) || klut.is_pi(node)) {
            return true;
        }

        it = this->steps_.emplace(it, Action::compute, node);
        ++it;

        if (!klut.visited(node)) {
            it = this->steps_.emplace(it, Action::cleanup, node);
        }
        return true;
    });
    return true;
}

} // namespace tweedledum
