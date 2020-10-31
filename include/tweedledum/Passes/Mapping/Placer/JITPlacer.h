/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../MapState.h"
#include "../../../Operators/Reversible.h"
#include "../../Utility/reverse.h"

namespace tweedledum {

class JITPlacer {
public:
    JITPlacer(MapState& state)
        : state_(state), visited_(state_.original.size(), 0u)
        , involved_phy_(state_.device.num_qubits(), 0u)
        , phy_decay_(state_.device.num_qubits(), 1.0), num_swaps_(0u)
    {
        extended_layer_.reserve(e_set_size_);
    }

    void run()
    {
        Circuit reversed = reverse(state_.original);

        for (uint32_t i = 0; i < 1; ++i) {
            current_ = &state_.original;
            do_run();

            current_ = &reversed;
            reset();
            do_run();
        }
    }

private:
    using Swap = std::pair<WireRef, WireRef>;

    void do_run();

    void reset()
    {
        std::fill(visited_.begin(), visited_.end(), 0u);
        std::fill(phy_decay_.begin(), phy_decay_.end(), 1.0);
        num_swaps_ = 0u;
    }

    bool add_front_layer();

    void select_extended_layer();

    std::vector<WireRef> find_free_phy() const;

    void place_two_v(WireRef const v0, WireRef const v1);

    void place_one_v(WireRef const v0, WireRef const v1);

    bool add_instruction(Instruction const& inst);

    void add_swap(WireRef const phy0, WireRef const phy1);

    Swap find_swap();

    double compute_cost(std::vector<WireRef> const&, std::vector<InstRef> const&);

    MapState& state_;
    Circuit const* current_;
    std::vector<uint32_t> visited_;

    // Sabre internals
    std::vector<InstRef> front_layer_;
    std::vector<InstRef> extended_layer_;
    std::vector<uint32_t> involved_phy_;
    std::vector<float> phy_decay_;
    uint32_t num_swaps_;
    
    // Sabre configuration
    uint32_t e_set_size_ = 20;
    float e_weight_ = 0.5;
    float decay_delta = 0.001;
    uint32_t num_rounds_decay_reset = 5;
    bool use_look_ahead_ = true;
};

} // namespace tweedledum
