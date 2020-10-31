/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Mapping/Router/JITRouter.h"
#include "tweedledum/Operators/Reversible.h"

namespace tweedledum {

void JITRouter::do_run()
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

bool JITRouter::add_front_layer()
{
    bool added_at_least_one = false;
    std::vector<InstRef> new_front_layer;
    for (InstRef ref : front_layer_) {
        Instruction const& inst = state_.original.instruction(ref);
        if (try_add_instruction(ref, inst) == false) {
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

void JITRouter::select_extended_layer()
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

std::vector<WireRef> JITRouter::find_free_phy() const
{
    std::vector<WireRef> free_phy;
    for (uint32_t i = 0; i < state_.phy_to_v.size(); ++i) {
        if (state_.phy_to_v.at(i) == WireRef::invalid()) {
            free_phy.push_back(state_.mapped.wire_ref(i));
        }
    }
    return free_phy;
}
    
void JITRouter::place_two_v(WireRef const v0, WireRef const v1)
{
    WireRef phy0 = state_.v_to_phy.at(v0);
    WireRef phy1 = state_.v_to_phy.at(v1);
    std::vector<WireRef> const free_phy = find_free_phy();
    assert(free_phy.size() >= 2u);
    if (free_phy.size() == 2u) {
        phy0 = free_phy.at(0);
        phy1 = free_phy.at(1);
    } else {
        uint32_t min_dist = std::numeric_limits<uint32_t>::max();
        for (uint32_t i = 0u; i < free_phy.size(); ++i) {
            for (uint32_t j = i + 1u; j < free_phy.size(); ++j) {
                WireRef const i_phy = free_phy.at(i);
                WireRef const j_phy = free_phy.at(j);
                if (min_dist < state_.device.distance(i_phy, j_phy)) {
                    continue;
                }
                min_dist = state_.device.distance(i_phy, j_phy);
                phy0 = i_phy;
                phy1 = j_phy;
            }
        }
    }
    state_.v_to_phy.at(v0) = phy0;
    state_.v_to_phy.at(v1) = phy1;
    state_.phy_to_v.at(phy0) = v0;
    state_.phy_to_v.at(phy1) = v1;
    add_delayed(v0);
    add_delayed(v1);
}

void JITRouter::place_one_v(WireRef v0, WireRef v1)
{
    WireRef phy0 = state_.v_to_phy.at(v0);
    WireRef phy1 = state_.v_to_phy.at(v1);
    std::vector<WireRef> const free_phy = find_free_phy();
    assert(free_phy.size() >= 1u);
    if (phy1 == WireRef::invalid()) {
        std::swap(v0, v1);
        std::swap(phy0, phy1);
    }
    phy0 = free_phy.at(0);
    uint32_t min_dist = state_.device.distance(phy1, phy0);
    for (uint32_t i = 1u; i < free_phy.size(); ++i) {
        if (min_dist > state_.device.distance(phy1, free_phy.at(i))) {
            min_dist = state_.device.distance(phy1, free_phy.at(i));
            phy0 = free_phy.at(i);
        }
    }
    state_.v_to_phy.at(v0) = phy0;
    state_.phy_to_v.at(phy0) = v0;
    add_delayed(v0);
}

//FIXME: need take care of wires
void JITRouter::add_delayed(WireRef const v)
{
    assert(v < delayed_.size());
    for (InstRef ref : delayed_.at(v)) {
        Instruction const& inst = state_.original.instruction(ref);
        add_instruction(inst);
    }
    delayed_.at(v).clear();
}

void JITRouter::add_instruction(Instruction const& inst)
{
    std::vector<WireRef> new_wires;
    new_wires.reserve(inst.num_wires());
    inst.foreach_wire([&](WireRef ref) {
        new_wires.push_back(state_.wire_to_wire(ref));
    });
    state_.mapped.apply_operator(inst, new_wires);
}

bool JITRouter::try_add_instruction(InstRef ref, Instruction const& inst)
{
    assert(inst.num_qubits() && inst.num_qubits() <= 2u);
    // Transform the wires to a new
    SmallVector<WireRef, 2> qubits;
    inst.foreach_wire([&](WireRef ref) {
        if (ref.kind() == Wire::Kind::quantum) {
            qubits.push_back(ref);
        }
    });

    WireRef phy0 = state_.wire_to_wire(qubits[0]);
    if (inst.num_qubits() == 1) {
        if (phy0 == WireRef::invalid()) {
            delayed_.at(state_.wire_to_v.at(qubits[0])).push_back(ref);
        } else {
            add_instruction(inst);
        }
        return true;
    }
    // FIXME: implement .at in SmallVector!
    WireRef phy1 = state_.wire_to_wire(qubits[1]);
    if (phy0 == WireRef::invalid() && phy1 == WireRef::invalid()) {
        place_two_v(state_.wire_to_v.at(qubits[0]), state_.wire_to_v.at(qubits[1]));
    } else if (phy0 == WireRef::invalid() || phy1 == WireRef::invalid()) {
        place_one_v(state_.wire_to_v.at(qubits[0]), state_.wire_to_v.at(qubits[1]));
    }
    phy0 = state_.wire_to_wire(qubits[0]);
    phy1 = state_.wire_to_wire(qubits[1]);
    if (!state_.device.are_connected(phy0, phy1)) {
        return false;
    }
    add_instruction(inst);
    return true;
}

void JITRouter::add_swap(WireRef const phy0, WireRef const phy1)
{
    state_.swap_qubits(phy0, phy1);
    mapped_->apply_operator(Op::Swap(), {phy0, phy1});
}

JITRouter::Swap JITRouter::find_swap()
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
        WireRef const v0 = state_.phy_to_v.at(phy0);
        WireRef const v1 = state_.phy_to_v.at(phy1);
        if (v0 != WireRef::invalid()) {
            v_to_phy.at(v0) = phy1;
        }
        if (v1 != WireRef::invalid()) {
            v_to_phy.at(v1) = phy0;
        }
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

double JITRouter::compute_cost(std::vector<WireRef> const& v_to_phy, std::vector<InstRef> const& layer)
{
    double cost = 0.0;
    for (InstRef ref : layer) {
        Instruction const& inst = state_.original.instruction(ref);
        WireRef const v0 = state_.wire_to_v.at(inst.qubit(0));
        WireRef const v1 = state_.wire_to_v.at(inst.qubit(1));
        WireRef const phy0 = v_to_phy.at(v0);
        WireRef const phy1 = v_to_phy.at(v1);
        if (phy0 == WireRef::invalid() || phy1 == WireRef::invalid()) {
            continue;
        }
        cost += (state_.device.distance(phy0, phy1) - 1);
    }
    return cost;
}

} // namespace tweedledum
