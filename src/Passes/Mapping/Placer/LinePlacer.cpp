/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Mapping/Placer/LinePlacer.h"

#include <algorithm>
#include <random>
#include <vector>

namespace tweedledum {

// Partition the quantum circuit into timesteps.  The circuit structure provides
// a natural partial ordering of the gates; thus a greedy algorithm starting
// from inputs can divide the input circuit into "vertical" partitions of gates
// which can be executed simultaneously.
void LinePlacer::partition_into_timeframes()
{
    std::vector<uint32_t> frame(original_.size(), 0u);
    original_.foreach_instruction([&](InstRef ref, Instruction const& inst) {
        uint32_t max_timeframe = 0u;
        original_.foreach_child(ref, [&](InstRef child) {
            max_timeframe = std::max(max_timeframe, frame.at(child));
        });
        if (inst.num_qubits() == 1) {
            frame.at(ref) = max_timeframe;
        } else {
            frame.at(ref) = ++max_timeframe;
            uint32_t const control = inst.qubit(0);
            uint32_t const target = inst.qubit(1);
            if (max_timeframe == timeframes_.size()) {
                timeframes_.emplace_back();
            }
            timeframes_.at(max_timeframe).emplace_back(control, target);
        }
    });
}

// Iterate over the timesteps to construct a graph whose vertices are qubits.
// At each timestep, I add the edge (q0, q1) to the graph if:
//     (1) this pair is present in the timestep (duh!)
//     (2) both qubits q0 and q1 have degree less than 2
// Each connected component of the resulting graph is necessarily either a line
// or a ring; the rings are broken by removing an arbitrarily chosen edge.
//
// NOTE: I have no idea if this is random or not :(  I could use, for example:
//     (3) if the addition of this edge does not introduce a cycle
//
void LinePlacer::build_connectivity_graph()
{
    std::vector<int> parent(num_v(), -1);
    auto find_root = [&parent](uint32_t i) -> uint32_t {
        while (parent.at(i) != -1) {
            i = parent.at(i);
        }
        return i;
    };
    for (std::vector<pair_type> const& timeframe : timeframes_) {
        for (auto const& [q0, q1] : timeframe) {
            if (v_degree_.at(q0) >= 2u || v_degree_.at(q1) >= 2u) {
                continue;
            }
            uint32_t q0_root = find_root(q0);
            uint32_t q1_root = find_root(q1);
            if (q0_root == q1_root) {
                continue;
            }
            parent.at(q0_root) = q1_root;
            connectivity_graph_.emplace_back(q0, q1);
            ++v_degree_.at(q0);
            ++v_degree_.at(q1);
        }
    }
}

int LinePlacer::find_next_line_node(uint32_t const root)
{
    int result = -1;
    for (uint32_t edge = 0; edge < connectivity_graph_.size(); ++edge) {
        auto& [u, v] = connectivity_graph_.at(edge);
        if (u == root) {
            result = v;
            v = -1;
            break;
        } else if (v == root) {
            result = u;
            u = -1;
            break;
        }
    }
    return result;
};

void LinePlacer::extract_lines()
{
    for (uint32_t i = 0u; i < v_degree_.size(); ++i) {
        if (v_degree_.at(i) != 1) {
            continue;
        }
        lines_.emplace_back(1, i);
        int next = find_next_line_node(i);
        while (next != -1) {
            lines_.back().emplace_back(next);
            next = find_next_line_node(next);
        }
        --v_degree_.at(lines_.back().back());
    }
    std::sort(lines_.begin(), lines_.end(),
      [](auto const& l0, auto const& l1) { return l0.size() > l1.size(); });
}

Qubit LinePlacer::pick_neighbor(Placement const& placement, uint32_t phy) const
{
    Qubit max_degree_neighbor = Qubit::invalid();
    device_.foreach_neighbor(phy, [&](uint32_t const neighbor) {
        if (placement.phy_to_v(neighbor) != Qubit::invalid()) {
            return;
        }
        if (max_degree_neighbor == Qubit::invalid()) {
            max_degree_neighbor = Qubit(neighbor);
            return;
        }
        if (phy_degree_.at(max_degree_neighbor) < phy_degree_.at(neighbor)) {
            max_degree_neighbor = neighbor;
        }
    });
    return max_degree_neighbor;
}

std::optional<Placement> LinePlacer::place_lines()
{
    Qubit max_degree_phy = Qubit(0u);
    for (uint32_t phy = 0u; phy < phy_degree_.size(); ++phy) {
        phy_degree_.at(phy) = device_.degree(phy);
        if (device_.degree(max_degree_phy) < device_.degree(phy)) {
            max_degree_phy = Qubit(phy);
        }
    }
    Placement placement(device_.num_qubits(), original_.num_qubits());
    for (std::vector<uint32_t> const& line : lines_) {
        placement.map_v_phy(Qubit(line.at(0)), max_degree_phy);
        // --phy_degree_.at(max_degree_phy);
        phy_degree_.at(max_degree_phy) = 0;
        for (uint32_t i = 1u; i < line.size(); ++i) {
            Qubit neighbor = pick_neighbor(placement, max_degree_phy);
            if (neighbor == Qubit::invalid()) {
                break;
            }
            placement.map_v_phy(Qubit(line.at(i)), neighbor);
            phy_degree_.at(neighbor) = 0u;
        }
        for (uint32_t i = 0u; i < phy_degree_.size(); ++i) {
            // if (placement.phy_to_v(i) != Qubit::invalid()) {
            //     continue;
            // }
            if (phy_degree_.at(max_degree_phy) < phy_degree_.at(i)) {
                max_degree_phy = i;
            }
        }
    }
    return placement;
}

/*! \brief Yet to be written.
 */
std::optional<Placement> line_place(
  Device const& device, Circuit const& original)
{
    LinePlacer placer(device, original);
    return placer.run();
}

} // namespace tweedledum
