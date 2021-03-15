/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../MapState.h"
#include "../../../Operators/Reversible.h"
#include "../../Utility/shallow_duplicate.h"

namespace tweedledum {

class JITRouter {
public:
    JITRouter(MapState& state)
        : state_(state), visited_(state_.original.size(), 0u)
        , involved_phy_(state_.device.num_qubits(), 0u)
        , phy_decay_(state_.device.num_qubits(), 1.0)
        , delayed_(state_.device.num_qubits())
    {
        extended_layer_.reserve(e_set_size_);
    }

    void run()
    {
        Circuit mapped = shallow_duplicate(state_.mapped);
        mapped_ = &mapped;
        do_run();
        mapped.foreach_r_instruction([&](Instruction const& inst) {
            state_.mapped.apply_operator(inst);
        });
    }

private:
    using Swap = std::pair<Qubit, Qubit>;

    void do_run();

    bool add_front_layer();

    void select_extended_layer();

    std::vector<Qubit> find_free_phy() const;

    void place_two_v(Qubit const v0, Qubit const v1);

    void place_one_v(Qubit const v0, Qubit const v1);

    void add_instruction(Instruction const& inst);

    void add_delayed(Qubit const v);

    void add_swap(Qubit const phy0, Qubit const phy1);

    bool try_add_instruction(InstRef ref, Instruction const& inst);

    Swap find_swap();

    double compute_cost(std::vector<Qubit> const&, std::vector<InstRef> const&);

    MapState& state_;
    Circuit* mapped_;
    std::vector<uint32_t> visited_;

    // Sabre internals
    std::vector<InstRef> front_layer_;
    std::vector<InstRef> extended_layer_;
    std::vector<uint32_t> involved_phy_;
    std::vector<float> phy_decay_;
    std::vector<std::vector<InstRef>> delayed_;
    
    // Sabre configuration
    uint32_t e_set_size_ = 20;
    float e_weight_ = 0.5;
    float decay_delta = 0.001;
    uint32_t num_rounds_decay_reset = 5;
    bool use_look_ahead_ = true;
};

} // namespace tweedledum
