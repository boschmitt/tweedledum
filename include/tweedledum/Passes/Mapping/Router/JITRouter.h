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
#include "../../../Target/Mapping.h"
#include "../../Utility/reverse.h"

namespace tweedledum {

class JITRouter {
public:
    JITRouter(Device const& device, Circuit const& original,
        Placement const& init_placement)
        : device_(device), original_(original), mapping_(init_placement)
        , visited_(original_.size(), 0u)
        , involved_phy_(device_.num_qubits(), 0u)
        , phy_decay_(device_.num_qubits(), 1.0)
        , delayed_(device_.num_qubits())
    {
        extended_layer_.reserve(e_set_size_);
    }

    std::pair<Circuit, Mapping> run()
    {
        Circuit mapped;
        original_.foreach_cbit([&](std::string_view name) {
            mapped.create_cbit(name);
        });
        for (uint32_t i = 0u; i < original_.num_qubits(); ++i) {
            Qubit const qubit = mapping_.placement.phy_to_v(i);
            mapped.create_qubit(original_.name(qubit));
        }
        for (uint32_t i = original_.num_qubits(); i < device_.num_qubits(); ++i) {
            mapped.create_qubit();
        }
        mapped_ = &mapped;
        do_run();
        std::swap(mapping_.init_placement, mapping_.placement);
        return {reverse(mapped), mapping_};
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
    std::vector<std::vector<InstRef>> delayed_;
    
    // Sabre configuration
    uint32_t e_set_size_ = 20;
    float e_weight_ = 0.5;
    float decay_delta = 0.001;
    uint32_t num_rounds_decay_reset = 5;
    bool use_look_ahead_ = true;
};

} // namespace tweedledum
