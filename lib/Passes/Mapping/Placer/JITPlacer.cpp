/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Mapping/Placer/JITPlacer.h"

namespace tweedledum {

void JITPlacer::do_run()
{
    current_->foreach_output([&](InstRef const ref, Instruction const& inst) {
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

bool JITPlacer::add_front_layer()
{
    bool added_at_least_one = false;
    std::vector<InstRef> new_front_layer;
    for (InstRef ref : front_layer_) {
        Instruction const& inst = current_->instruction(ref);
        if (add_instruction(inst) == false) {
                new_front_layer.push_back(ref);
                auto const qubits = inst.qubits();
                involved_phy_.at(state_.v_to_phy.at(qubits.at(0))) = 1u;
                involved_phy_.at(state_.v_to_phy.at(qubits.at(1))) = 1u;
                continue;
        }
        added_at_least_one = true;
        current_->foreach_child(ref, [&](InstRef cref, Instruction const& child) {
            visited_.at(cref) += 1;
            if (visited_.at(cref) == child.num_wires()) {
                new_front_layer.push_back(cref);
            }
        });
    }
    front_layer_ = std::move(new_front_layer);
    return added_at_least_one;
}

void JITPlacer::select_extended_layer()
{
    extended_layer_.clear();
    std::vector<InstRef> incremented;
    std::vector<InstRef> tmp_layer = front_layer_;
    while (!tmp_layer.empty()) {
        std::vector<InstRef> new_tmp_layer;
        for (InstRef const ref : tmp_layer) {
            current_->foreach_child(ref, [&](InstRef cref, Instruction const& child) {
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

std::vector<Qubit> JITPlacer::find_free_phy() const
{
    std::vector<Qubit> free_phy;
    for (uint32_t i = 0; i < state_.phy_to_v.size(); ++i) {
        if (state_.phy_to_v.at(i) == Qubit::invalid()) {
            free_phy.push_back(state_.mapped.qubit(i));
        }
    }
    return free_phy;
}

void JITPlacer::place_two_v(Qubit const v0, Qubit const v1)
{
    Qubit phy0 = state_.v_to_phy.at(v0);
    Qubit phy1 = state_.v_to_phy.at(v1);
    std::vector<Qubit> const free_phy = find_free_phy();
    assert(free_phy.size() >= 2u);
    if (free_phy.size() == 2u) {
        phy0 = free_phy.at(0);
        phy1 = free_phy.at(1);
    } else {
        uint32_t min_dist = std::numeric_limits<uint32_t>::max();
        for (uint32_t i = 0u; i < free_phy.size(); ++i) {
            for (uint32_t j = i + 1u; j < free_phy.size(); ++j) {
                Qubit const i_phy = free_phy.at(i);
                Qubit const j_phy = free_phy.at(j);
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
}

void JITPlacer::place_one_v(Qubit v0, Qubit v1)
{
    Qubit phy0 = state_.v_to_phy.at(v0);
    Qubit phy1 = state_.v_to_phy.at(v1);
    std::vector<Qubit> const free_phy = find_free_phy();
    assert(free_phy.size() >= 1u);
    if (phy1 == Qubit::invalid()) {
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
}

bool JITPlacer::add_instruction(Instruction const& inst)
{
    assert(inst.num_qubits() && inst.num_qubits() <= 2u);
    if (inst.num_qubits() == 1) {
        return true;
    }
    // Transform the wires to a new
    SmallVector<Qubit, 2> qubits;
    inst.foreach_qubit([&](Qubit ref) {
        qubits.push_back(ref);
    });
    // FIXME: implement .at in SmallVector!
    Qubit phy0 = state_.v_to_phy.at(qubits[0]);
    Qubit phy1 = state_.v_to_phy.at(qubits[1]);
    if (phy0 == Qubit::invalid() && phy1 == Qubit::invalid()) {
        place_two_v(qubits[0], qubits[1]);
    } else if (phy0 == Qubit::invalid() || phy1 == Qubit::invalid()) {
        place_one_v(qubits[0], qubits[1]);
    }
    phy0 = state_.v_to_phy.at(qubits[0]);
    phy1 = state_.v_to_phy.at(qubits[1]);
    if (!state_.device.are_connected(phy0, phy1)) {
        return false;
    }
    return true;
}

void JITPlacer::add_swap(Qubit const phy0, Qubit const phy1)
{
    num_swaps_ += 1;
    state_.swap_qubits(phy0, phy1);
}

JITPlacer::Swap JITPlacer::find_swap()
{
    // Obtain SWAP candidates
    std::vector<Swap> swap_candidates;
    for (uint32_t i = 0u; i < state_.device.num_edges(); ++i) {
        auto const& [u, v] = state_.device.edge(i);
        if (involved_phy_.at(u) || involved_phy_.at(v)) {
            swap_candidates.emplace_back(state_.mapped.qubit(u), state_.mapped.qubit(v));
        }
    }

    if (use_look_ahead_) {
        select_extended_layer();
    }

    // Compute cost
    std::vector<double> cost;
    for (auto const& [phy0, phy1] : swap_candidates) {
        std::vector<Qubit> v_to_phy = state_.v_to_phy;
        Qubit const v0 = state_.phy_to_v.at(phy0);
        Qubit const v1 = state_.phy_to_v.at(phy1);
        if (v0 != Qubit::invalid()) {
            v_to_phy.at(v0) = phy1;
        }
        if (v1 != Qubit::invalid()) {
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

double JITPlacer::compute_cost(std::vector<Qubit> const& v_to_phy, std::vector<InstRef> const& layer)
{
    double cost = 0.0;
    for (InstRef ref : layer) {
        Instruction const& inst = current_->instruction(ref);
        Qubit const v0 = inst.qubit(0);
        Qubit const v1 = inst.qubit(1);
        Qubit const phy0 = v_to_phy.at(v0);
        Qubit const phy1 = v_to_phy.at(v1);
        if (phy0 == Qubit::invalid() || phy1 == Qubit::invalid()) {
            continue;
        }
        cost += (state_.device.distance(phy0, phy1) - 1);
    }
    return cost;
}

} // namespace tweedledum
