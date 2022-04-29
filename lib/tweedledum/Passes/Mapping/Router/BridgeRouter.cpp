/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Mapping/Router/BridgeRouter.h"
#include "tweedledum/Operators/Extension/Bridge.h"
#include "tweedledum/Operators/Standard/X.h"

namespace tweedledum {

std::pair<Circuit, Mapping> BridgeRouter::run()
{
    Circuit mapped;
    original_.foreach_cbit(
      [&](std::string_view name) { mapped.create_cbit(name); });
    for (uint32_t i = 0u; i < device_.num_qubits(); ++i) {
        mapped.create_qubit();
    }
    mapped_ = &mapped;

    original_.foreach_instruction([&](InstRef ref, Instruction const& inst) {
        if (try_add_instruction(ref, inst)) {
            return;
        }
        assert(inst.is_a<Op::X>() && inst.num_qubits() == 2);
        Qubit const control = placement_.v_to_phy(inst.control());
        Qubit const target = placement_.v_to_phy(inst.target());
        mapped_->apply_operator(Op::Bridge(), {control, target});
    });

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
    Mapping mapping(placement_);
    return {mapped, mapping};
}

std::vector<Qubit> BridgeRouter::find_unmapped(
  std::vector<Qubit> const& map) const
{
    std::vector<Qubit> unmapped;
    for (uint32_t i = 0; i < map.size(); ++i) {
        if (map.at(i) == Qubit::invalid()) {
            unmapped.emplace_back(i);
        }
    }
    return unmapped;
}

void BridgeRouter::place_two_v(Qubit const v0, Qubit const v1)
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

void BridgeRouter::place_one_v(Qubit v0, Qubit v1)
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

void BridgeRouter::add_delayed(Qubit const v)
{
    assert(v < delayed_.size());
    for (InstRef ref : delayed_.at(v)) {
        Instruction const& inst = original_.instruction(ref);
        add_instruction(inst);
    }
    delayed_.at(v).clear();
}

void BridgeRouter::add_instruction(Instruction const& inst)
{
    std::vector<Qubit> phys;
    inst.foreach_qubit(
      [&](Qubit v) { phys.push_back(placement_.v_to_phy(v)); });
    mapped_->apply_operator(inst, phys, inst.cbits());
}

bool BridgeRouter::try_add_instruction(InstRef ref, Instruction const& inst)
{
    assert(inst.num_qubits() && inst.num_qubits() <= 2u);
    // Transform the wires to a new
    SmallVector<Qubit, 2> qubits;
    inst.foreach_qubit([&](Qubit qubit) { qubits.push_back(qubit); });

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

} // namespace tweedledum
