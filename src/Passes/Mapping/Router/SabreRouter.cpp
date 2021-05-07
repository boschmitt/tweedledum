/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Mapping/Router/SabreRouter.h"
#include "tweedledum/Operators/Reversible.h"

namespace tweedledum {

std::pair<Circuit, Mapping> SabreRouter::run()
{
    Circuit mapped;
    original_.foreach_cbit(
      [&](std::string_view name) { mapped.create_cbit(name); });
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
    std::swap(mapping_.init_placement, mapping_.placement);
    return {reverse(mapped), mapping_};
}

bool SabreRouter::add_front_layer()
{
    bool added_at_least_one = false;
    std::vector<InstRef> new_front_layer;
    for (InstRef ref : front_layer_) {
        Instruction const& inst = original_.instruction(ref);
        if (add_instruction(inst) == false) {
            new_front_layer.push_back(ref);
            auto const qubits = inst.qubits();
            involved_phy_.at(mapping_.placement.v_to_phy(qubits.at(0))) = 1u;
            involved_phy_.at(mapping_.placement.v_to_phy(qubits.at(1))) = 1u;
            continue;
        }
        added_at_least_one = true;
        original_.foreach_child(
          ref, [&](InstRef cref, Instruction const& child) {
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
            original_.foreach_child(
              ref, [&](InstRef cref, Instruction const& child) {
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
    std::vector<Qubit> phys;
    inst.foreach_qubit([&](Qubit v) {
        Qubit const phy = mapping_.placement.v_to_phy(v);
        phys.push_back(phy);
    });

    if (inst.num_qubits() == 1) {
        mapped_->apply_operator(inst, phys, inst.cbits());
        return true;
    }
    if (!device_.are_connected(phys.at(0), phys.at(1))) {
        return false;
    }
    mapped_->apply_operator(inst, phys, inst.cbits());
    return true;
}

void SabreRouter::add_swap(Qubit const phy0, Qubit const phy1)
{
    mapping_.placement.swap_qubits(phy0, phy1);
    mapped_->apply_operator(Op::Swap(), {phy0, phy1});
}

SabreRouter::Swap SabreRouter::find_swap()
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
        std::vector<Qubit> v_to_phy = mapping_.placement.v_to_phy();
        Qubit const v0 = mapping_.placement.phy_to_v(phy0);
        Qubit const v1 = mapping_.placement.phy_to_v(phy1);
        if (v0 != Qubit::invalid()) {
            v_to_phy.at(v0) = phy1;
        }
        if (v1 != Qubit::invalid()) {
            v_to_phy.at(v1) = phy0;
        }
        double swap_cost = compute_cost(v_to_phy, front_layer_);
        double const max_decay =
          std::max(phy_decay_.at(phy0), phy_decay_.at(phy1));

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

double SabreRouter::compute_cost(
  std::vector<Qubit> const& v_to_phy, std::vector<InstRef> const& layer)
{
    double cost = 0.0;
    for (InstRef ref : layer) {
        Instruction const& inst = original_.instruction(ref);
        Qubit const v0 = inst.qubit(0);
        Qubit const v1 = inst.qubit(1);
        cost += (device_.distance(v_to_phy.at(v0), v_to_phy.at(v1)) - 1);
    }
    return cost;
}

} // namespace tweedledum
