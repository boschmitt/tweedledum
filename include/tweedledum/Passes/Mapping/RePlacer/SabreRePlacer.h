/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../../IR/Circuit.h"
#include "../../../IR/Qubit.h"
#include "../../../Operators/Reversible.h"
#include "../../../Target/Device.h"
#include "../../../Target/Placement.h"
#include "../../Utility/reverse.h"

namespace tweedledum {

class SabreRePlacer {
public:
    SabreRePlacer(Device const& device, Circuit const& original,
        Placement& placement)
        : device_(device), original_(original), placement_(placement)
        , visited_(original.size(), 0u)
        , involved_phy_(device_.num_qubits(), 0u)
        , phy_decay_(device_.num_qubits(), 1.0)
    {
        extended_layer_.reserve(e_set_size_);
    }

    void run()
    {
        current_ = &original_;
        do_run();

        Circuit reversed = reverse(original_);
        current_ = &reversed;
        reset();
        do_run();
    }

private:
    using Swap = std::pair<Qubit, Qubit>;

    void do_run();

    void reset()
    {
        std::fill(visited_.begin(), visited_.end(), 0u);
        std::fill(phy_decay_.begin(), phy_decay_.end(), 1.0);
    }

    bool add_front_layer();

    void select_extended_layer();

    bool add_instruction(Instruction const& inst);

    void add_swap(Qubit const phy0, Qubit const phy1);

    Swap find_swap();

    double compute_cost(std::vector<Qubit> const&, std::vector<InstRef> const&);

    Device const& device_;
    Circuit const& original_;
    Circuit const* current_;
    Placement& placement_;

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

/*! \brief Yet to be written.
 */
void sabre_re_place(Device const& device, Circuit const& original,
    Placement& placement);

} // namespace tweedledum
