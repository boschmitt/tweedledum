/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../MapState.h"
#include "../../../Operators/Reversible.h"

namespace tweedledum {

class SabreRouter {
public:
    SabreRouter(MapState& state)
        : state_(state), visited_(state.original.size(), 0u)
        , involved_phy_(state.device.num_qubits(), 0u)
        , phy_decay_(state.device.num_qubits(), 1.0)
    {
        extended_layer_.reserve(e_set_size_);
    }

    void run();

private:
    using Swap = std::pair<Qubit, Qubit>;

    bool add_front_layer();

    void select_extended_layer();

    bool add_instruction(Instruction const& inst);

    void add_swap(Qubit const phy0, Qubit const phy1);

    Swap find_swap();

    double compute_cost(std::vector<Qubit> const&, std::vector<InstRef> const&);

    MapState& state_;
    std::vector<uint32_t> visited_;

    // Sabre internals
    std::vector<InstRef> front_layer_;
    std::vector<InstRef> extended_layer_;
    std::vector<uint32_t> involved_phy_;
    std::vector<float> phy_decay_;
    
    // Sabre configuration
    uint32_t e_set_size_ = 20;
    float e_weight_ = 0.5;
    float decay_delta = 0.001;
    uint32_t num_rounds_decay_reset = 5;
    bool use_look_ahead_ = true;
};

} // namespace tweedledum
