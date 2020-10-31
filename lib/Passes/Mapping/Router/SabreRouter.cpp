/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Mapping/Router/SabreRouter.h"
#include "tweedledum/Operators/Reversible.h"

namespace tweedledum {

void SabreRouter::run()
{
    state_.original.foreach_output([&](InstRef const ref, Instruction const& inst) {
        visited_.at(ref) += 1;
        if (visited_.at(ref) == inst.num_wires()) {
            front_layer_.push_back(ref);
        }
    });

    uint32_t num_swap_searches = 0u;
    while (!front_layer_.empty()) {
        if (add_front_layer()) {
            continue;
        }
        num_swap_searches += 1;
        auto const [phy0, phy1] = find_swap();
        if ((num_swap_searches % num_rounds_decay_reset) == 0) {
            std::fill(phy_decay_.begin(), phy_decay_.end(), 1.0);
        } else {
            phy_decay_.at(phy0) += decay_delta;
            phy_decay_.at(phy1) += decay_delta;
        }
        add_swap(phy0, phy1);
        std::fill(involved_phy_.begin(), involved_phy_.end(), 0);
    }
}

bool SabreRouter::add_front_layer()
{
    bool added_at_least_one = false;
    std::vector<InstRef> new_front_layer;
    for (InstRef ref : front_layer_) {
        Instruction const& inst = state_.original.instruction(ref);
        if (add_instruction(inst) == false) {
                new_front_layer.push_back(ref);
                auto const qubits = inst.qubits();
                involved_phy_.at(state_.wire_to_wire(qubits.at(0))) = 1u;
                involved_phy_.at(state_.wire_to_wire(qubits.at(1))) = 1u;
                continue;
        }
        added_at_least_one = true;
        state_.original.foreach_child(ref, [&](InstRef cref, Instruction const& child) {
            visited_.at(cref) += 1;
            if (visited_.at(cref) == child.num_wires()) {
                new_front_layer.push_back(cref);
            }
        });
    }
    front_layer_ = std::move(new_front_layer);
    return added_at_least_one;
}

void SabreRouter::select_extended_layer()
{
    extended_layer_.clear();
    std::vector<InstRef> incremented;
    std::vector<InstRef> tmp_layer = front_layer_;
    while (!tmp_layer.empty()) {
        std::vector<InstRef> new_tmp_layer;
        for (InstRef const ref : tmp_layer) {
            state_.original.foreach_child(ref, [&](InstRef cref, Instruction const& child) {
                visited_.at(cref) += 1;
                incremented.push_back(cref);
                if (visited_.at(cref) == child.num_wires()) {
                    new_tmp_layer.push_back(cref);
                    if (child.num_qubits() == 2u) {
                        extended_layer_.emplace_back(cref);
                    }
                }
            });
            if (extended_layer_.size() >= e_set_size_) {
                goto undo_increment;
            }
        }
        tmp_layer = std::move(new_tmp_layer);
    }
undo_increment:
    for (InstRef const ref : incremented) {
        visited_.at(ref) -= 1;
    }
}

bool SabreRouter::add_instruction(Instruction const& inst)
{
    assert(inst.num_qubits() && inst.num_qubits() <= 2u);
    // Transform the wires to a new
    std::vector<WireRef> new_wires;
    SmallVector<WireRef, 2> qubits;
    new_wires.reserve(inst.num_wires());
    inst.foreach_wire([&](WireRef ref) {
        WireRef const new_wire = state_.wire_to_wire(ref);
        new_wires.push_back(new_wire);
        if (ref.kind() == Wire::Kind::quantum) {
            qubits.push_back(new_wire);
        }
    });

    if (inst.num_qubits() == 1) {
        state_.mapped.apply_operator(inst, new_wires);
        return true;
    }
    // FIXME: implement .at in SmallVector!
    if (!state_.device.are_connected(qubits[0], qubits[1])) {
        return false;
    }
    state_.mapped.apply_operator(inst, new_wires);
    return true;
}

void SabreRouter::add_swap(WireRef const phy0, WireRef const phy1)
{
    state_.swap_qubits(phy0, phy1);
    state_.mapped.apply_operator(Op::Swap(), {phy0, phy1});
}

SabreRouter::Swap SabreRouter::find_swap()
{
    // Obtain SWAP candidates
    std::vector<Swap> swap_candidates;
    for (uint32_t i = 0u; i < state_.device.num_edges(); ++i) {
        auto const& [u, v] = state_.device.edge(i);
        if (involved_phy_.at(u) || involved_phy_.at(v)) {
            swap_candidates.emplace_back(state_.mapped.wire_ref(u), state_.mapped.wire_ref(v));
        }
    }

    if (use_look_ahead_) {
        select_extended_layer();
    }

    // Compute cost
    std::vector<double> cost;
    for (auto const& [phy0, phy1] : swap_candidates) {
        std::vector<WireRef> v_to_phy = state_.v_to_phy;
        std::swap(v_to_phy.at(state_.phy_to_v.at(phy0)), v_to_phy.at(state_.phy_to_v.at(phy1)));
        double swap_cost = compute_cost(v_to_phy, front_layer_);
        double const max_decay = std::max(phy_decay_.at(phy0), phy_decay_.at(phy1));

        if (!extended_layer_.empty()) {
            double const f_cost = swap_cost / front_layer_.size();
            double e_cost = compute_cost(v_to_phy, extended_layer_);
            e_cost = e_cost / extended_layer_.size();
            swap_cost = f_cost + (e_weight_ * e_cost);
        }
        cost.emplace_back(max_decay * swap_cost);
    }

    // Find and return the swap with minimal cost
    uint32_t min = 0u;
    for (uint32_t i = 1u; i < cost.size(); ++i) {
        if (cost.at(i) < cost.at(min)) {
            min = i;
        }
    }
    return swap_candidates.at(min);
}

double SabreRouter::compute_cost(std::vector<WireRef> const& v_to_phy, std::vector<InstRef> const& layer)
{
    double cost = 0.0;
    for (InstRef ref : layer) {
        Instruction const& inst = state_.original.instruction(ref);
        WireRef const v0 = state_.wire_to_v.at(inst.qubit(0));
        WireRef const v1 = state_.wire_to_v.at(inst.qubit(1));
        cost += (state_.device.distance(v_to_phy.at(v0), v_to_phy.at(v1)) - 1);
    }
    return cost;
}

} // namespace tweedledum
