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
    std::vector<uint32_t> frame(state_.original.size(), 0u);
    state_.original.foreach_instruction([&](InstRef ref, Instruction const& inst) {
        uint32_t max_timeframe = 0u;
        state_.original.foreach_child(ref, [&](InstRef child) {
            max_timeframe = std::max(max_timeframe, frame.at(child));
        });
        if (inst.num_qubits() == 1) {
            frame.at(ref) = max_timeframe;
        } else {
            frame.at(ref) = ++max_timeframe;
            uint32_t const control = state_.wire_to_v.at(inst.qubit(0));
            uint32_t const target = state_.wire_to_v.at(inst.qubit(1));
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

int LinePlacer::pick_neighbor(uint32_t const phy) const
{
    int max_degree_neighbor = -1;
    state_.device.foreach_neighbor(phy, [&](uint32_t const neighbor) {
        if (state_.phy_to_v.at(neighbor) != WireRef::invalid()) {
            return;
        }
        if (max_degree_neighbor == -1) {
            max_degree_neighbor = neighbor;
            return;
        }
        if (phy_degree_.at(max_degree_neighbor) < phy_degree_.at(neighbor)) {
            max_degree_neighbor = neighbor;
        }
    });
    return max_degree_neighbor;
}

void LinePlacer::place_lines()
{
    uint32_t max_degree_phy = 0u;
    for (uint32_t phy = 0u; phy < phy_degree_.size(); ++phy) {
        phy_degree_.at(phy) = state_.device.degree(phy);
        if (state_.device.degree(max_degree_phy) < state_.device.degree(phy)) {
            max_degree_phy = phy;
        }
    }
    for (std::vector<uint32_t> const& line : lines_) {
        state_.phy_to_v.at(max_degree_phy) = state_.mapped.wire_ref(line.at(0));
        state_.v_to_phy.at(line.at(0)) = state_.mapped.wire_ref(max_degree_phy);
        // --phy_degree_.at(max_degree_phy);
        phy_degree_.at(max_degree_phy) = 0;
        for (uint32_t i = 1u; i < line.size(); ++i) {
            int neighbor = pick_neighbor(max_degree_phy);
            if (neighbor == -1) {
                break;
            }
            state_.phy_to_v.at(neighbor) = state_.mapped.wire_ref(line.at(i));
            state_.v_to_phy.at(line.at(i)) = state_.mapped.wire_ref(neighbor);
            // --phy_degree_.at(neighbor);
            phy_degree_.at(neighbor) = 0;
        }
        for (uint32_t i = 0u; i < phy_degree_.size(); ++i) {
            // if (state_.phy_to_v.at(i) != WireRef::invalid()) {
            //     continue;
            // }
            if (phy_degree_.at(max_degree_phy) < phy_degree_.at(i)) {
                max_degree_phy = i;
            }
        }
    }
}

} // namespace tweedledum
