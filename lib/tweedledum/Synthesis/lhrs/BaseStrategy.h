/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <mockturtle/networks/klut.hpp>
#include <vector>

namespace tweedledum {

class BaseStrategy {
protected:
    using Node = typename mockturtle::klut_network::node;

public:
    enum class Action : uint8_t
    {
        compute,
        cleanup,
    };

    struct Step {
        Action action;
        Node node;
        Step(Action a, Node n)
            : action(a)
            , node(n)
        {}
    };

    virtual ~BaseStrategy() = default;
    virtual bool compute_steps(mockturtle::klut_network const& network) = 0;

    uint32_t num_steps() const
    {
        return steps_.size();
    }

    auto begin() const
    {
        return steps_.cbegin();
    }

    auto end() const
    {
        return steps_.end();
    }

protected:
    std::vector<Step> steps_;
};

} // namespace tweedledum
