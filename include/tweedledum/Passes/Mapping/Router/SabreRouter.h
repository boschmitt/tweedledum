/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../../IR/Circuit.h"
#include "../../../IR/Qubit.h"
#include "../../../Operators/Reversible.h"
#include "../../../Target/Device.h"
#include "../../../Target/Mapping.h"
#include "../../../Target/Placement.h"
#include "../../Utility/reverse.h"

namespace tweedledum {

class SabreRouter {
public:
    SabreRouter(Device const& device, Circuit const& original,
      Placement const& init_placement)
        : device_(device)
        , original_(original)
        , mapping_(init_placement)
        , visited_(original_.size(), 0u)
        , involved_phy_(device_.num_qubits(), 0u)
        , phy_decay_(device_.num_qubits(), 1.0)
    {
        extended_layer_.reserve(e_set_size_);
    }

    std::pair<Circuit, Mapping> run();

private:
    using Swap = std::pair<Qubit, Qubit>;

    bool add_front_layer();

    void select_extended_layer();

    bool add_instruction(Instruction const& inst);

    void add_swap(Qubit const phy0, Qubit const phy1);

    Swap find_swap();

    double compute_cost(std::vector<Qubit> const&, std::vector<InstRef> const&);

    Device const& device_;
    Circuit const& original_;
    Circuit* mapped_;
    Mapping mapping_;
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
