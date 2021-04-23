/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Mapping/Router/JitRouter.h"
#include "tweedledum/Operators/Reversible.h"

namespace tweedledum {

std::pair<Circuit, Mapping> JitRouter::run()
{
    Circuit mapped;
    original_.foreach_cbit([&](std::string_view name) {
        mapped.create_cbit(name);
    });
    for (uint32_t i = 0u; i < device_.num_qubits(); ++i) {
        mapped.create_qubit();
    }
    mapped_ = &mapped;

    original_.foreach_output([&](InstRef const ref, Instruction const& inst) {
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

    std::vector<Qubit> const& v_to_phy = placement_.v_to_phy();
    std::vector<Qubit> phys = find_unmapped(placement_.phy_to_v());
    for (uint32_t i = 0; i < v_to_phy.size(); ++i) {
        if (v_to_phy.at(i) == Qubit::invalid()) {
            assert(!phys.empty());
            Qubit const v = Qubit(i);
            Qubit const phy = phys.back();
            phys.pop_back();
            placement_.map_v_phy(v, phy);
            add_delayed(v);
        }
    }
    Circuit const result = reverse(mapped);
    Mapping mapping(placement_);
    result.foreach_instruction([&](Instruction const& inst) { 
        if (inst.is_one<Op::Swap>()) {
            Qubit const t0 = inst.target(0u);
            Qubit const t1 = inst.target(1u);
            mapping.placement.swap_qubits(t0, t1);
        }
    });
    return {result, mapping};
}

bool JitRouter::add_front_layer()
{
    bool added_at_least_one = false;
    std::vector<InstRef> new_front_layer;
    for (InstRef ref : front_layer_) {
        Instruction const& inst = original_.instruction(ref);
        if (try_add_instruction(ref, inst) == false) {
                new_front_layer.push_back(ref);
                auto const qubits = inst.qubits();
                involved_phy_.at(placement_.v_to_phy(qubits.at(0))) = 1u;
                involved_phy_.at(placement_.v_to_phy(qubits.at(1))) = 1u;
                continue;
        }
        added_at_least_one = true;
        original_.foreach_child(ref, [&](InstRef cref, Instruction const& child) {
            visited_.at(cref) += 1;
            if (visited_.at(cref) == child.num_wires()) {
                new_front_layer.push_back(cref);
            }
        });
    }
    front_layer_ = std::move(new_front_layer);
    return added_at_least_one;
}

void JitRouter::select_extended_layer()
{
    extended_layer_.clear();
    std::vector<InstRef> incremented;
    std::vector<InstRef> tmp_layer = front_layer_;
    while (!tmp_layer.empty()) {
        std::vector<InstRef> new_tmp_layer;
        for (InstRef const ref : tmp_layer) {
            original_.foreach_child(ref, [&](InstRef cref, Instruction const& child) {
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

std::vector<Qubit> JitRouter::find_unmapped(std::vector<Qubit> const& map) const
{
    std::vector<Qubit> unmapped;
    for (uint32_t i = 0; i < map.size(); ++i) {
        if (map.at(i) == Qubit::invalid()) {
            unmapped.emplace_back(i);
        }
    }
    return unmapped;
}

void JitRouter::place_two_v(Qubit const v0, Qubit const v1)
{
    Qubit phy0 = placement_.v_to_phy(v0);
    Qubit phy1 = placement_.v_to_phy(v1);
    std::vector<Qubit> const free_phy = find_unmapped(placement_.phy_to_v());
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
                if (min_dist < device_.distance(i_phy, j_phy)) {
                    continue;
                }
                min_dist = device_.distance(i_phy, j_phy);
                phy0 = i_phy;
                phy1 = j_phy;
            }
        }
    }
    placement_.map_v_phy(v0, phy0);
    placement_.map_v_phy(v1, phy1);
    add_delayed(v0);
    add_delayed(v1);
}

void JitRouter::place_one_v(Qubit v0, Qubit v1)
{
    Qubit phy0 = placement_.v_to_phy(v0);
    Qubit phy1 = placement_.v_to_phy(v1);
    std::vector<Qubit> const free_phy = find_unmapped(placement_.phy_to_v());
    assert(free_phy.size() >= 1u);
    if (phy1 == Qubit::invalid()) {
        std::swap(v0, v1);
        std::swap(phy0, phy1);
    }
    phy0 = free_phy.at(0);
    uint32_t min_dist = device_.distance(phy1, phy0);
    for (uint32_t i = 1u; i < free_phy.size(); ++i) {
        if (min_dist > device_.distance(phy1, free_phy.at(i))) {
            min_dist = device_.distance(phy1, free_phy.at(i));
            phy0 = free_phy.at(i);
        }
    }
    placement_.map_v_phy(v0, phy0);
    add_delayed(v0);
}

//FIXME: need take care of wires
void JitRouter::add_delayed(Qubit const v)
{
    assert(v < delayed_.size());
    for (InstRef ref : delayed_.at(v)) {
        Instruction const& inst = original_.instruction(ref);
        add_instruction(inst);
    }
    delayed_.at(v).clear();
}

void JitRouter::add_instruction(Instruction const& inst)
{
    std::vector<Qubit> phys;
    inst.foreach_qubit([&](Qubit v) {
        phys.push_back(placement_.v_to_phy(v));
    });
    mapped_->apply_operator(inst, phys, inst.cbits());
}

bool JitRouter::try_add_instruction(InstRef ref, Instruction const& inst)
{
    assert(inst.num_qubits() && inst.num_qubits() <= 2u);
    // Transform the wires to a new
    SmallVector<Qubit, 2> qubits;
    inst.foreach_qubit([&](Qubit ref) {
        qubits.push_back(ref);
    });

    Qubit phy0 = placement_.v_to_phy(qubits[0]);
    if (inst.num_qubits() == 1) {
        if (phy0 == Qubit::invalid()) {
            delayed_.at(qubits[0]).push_back(ref);
        } else {
            add_instruction(inst);
        }
        return true;
    }
    // FIXME: implement .at in SmallVector!
    Qubit phy1 = placement_.v_to_phy(qubits[1]);
    if (phy0 == Qubit::invalid() && phy1 == Qubit::invalid()) {
        place_two_v(qubits[0], qubits[1]);
    } else if (phy0 == Qubit::invalid() || phy1 == Qubit::invalid()) {
        place_one_v(qubits[0], qubits[1]);
    }
    phy0 = placement_.v_to_phy(qubits[0]);
    phy1 = placement_.v_to_phy(qubits[1]);
    if (!device_.are_connected(phy0, phy1)) {
        return false;
    }
    add_instruction(inst);
    return true;
}

void JitRouter::add_swap(Qubit const phy0, Qubit const phy1)
{
    placement_.swap_qubits(phy0, phy1);
    mapped_->apply_operator(Op::Swap(), {phy0, phy1});
}

JitRouter::Swap JitRouter::find_swap()
{
    // Obtain SWAP candidates
    std::vector<Swap> swap_candidates;
    for (uint32_t i = 0u; i < device_.num_edges(); ++i) {
        auto const& [u, v] = device_.edge(i);
        if (involved_phy_.at(u) || involved_phy_.at(v)) {
            swap_candidates.emplace_back(Qubit(u), Qubit(v));
        }
    }

    if (use_look_ahead_) {
        select_extended_layer();
    }

    // Compute cost
    std::vector<double> cost;
    for (auto const& [phy0, phy1] : swap_candidates) {
        std::vector<Qubit> v_to_phy = placement_.v_to_phy();
        Qubit const v0 = placement_.phy_to_v(phy0);
        Qubit const v1 = placement_.phy_to_v(phy1);
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

double JitRouter::compute_cost(std::vector<Qubit> const& v_to_phy, std::vector<InstRef> const& layer)
{
    double cost = 0.0;
    for (InstRef ref : layer) {
        Instruction const& inst = original_.instruction(ref);
        Qubit const v0 = inst.qubit(0);
        Qubit const v1 = inst.qubit(1);
        Qubit const phy0 = v_to_phy.at(v0);
        Qubit const phy1 = v_to_phy.at(v1);
        if (phy0 == Qubit::invalid() || phy1 == Qubit::invalid()) {
            continue;
        }
        cost += (device_.distance(phy0, phy1) - 1);
    }
    return cost;
}

} // namespace tweedledum
