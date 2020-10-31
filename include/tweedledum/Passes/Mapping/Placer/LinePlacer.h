/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../../IR/Circuit.h"
#include "../../../IR/Wire.h"
#include "../../../Target/Device.h"
#include "../MapState.h"

#include <vector>

namespace tweedledum {

class LinePlacer {
public:
    LinePlacer(MapState& state)
        : state_(state), v_degree_(num_v(), 0u), phy_degree_(num_phy(), 0u)
        , timeframes_(1u)
    {}

    void run()
    {
        std::fill(state_.v_to_phy.begin(), state_.v_to_phy.end(), WireRef::invalid());
        std::fill(state_.phy_to_v.begin(), state_.phy_to_v.end(), WireRef::invalid());
        partition_into_timeframes();
        build_connectivity_graph();
        extract_lines();
        place_lines();
    }

private:
    using pair_type = std::pair<uint32_t, uint32_t>;

    // Returns the number of *virtual* qubits.
    uint32_t num_v() const
    {
        return state_.original.num_qubits();
    }

    // Returns the number of *physical* qubits.
    uint32_t num_phy() const
    {
        return state_.device.num_qubits();
    }

    void partition_into_timeframes();

    void build_connectivity_graph();

    int find_next_line_node(uint32_t const root);

    void extract_lines();

    int pick_neighbor(uint32_t const phy) const;

    void place_lines();

    MapState& state_;

    std::vector<uint32_t> v_degree_;
	std::vector<uint32_t> phy_degree_;
	std::vector<std::vector<pair_type>> timeframes_;
	std::vector<pair_type> connectivity_graph_;
	std::vector<std::vector<uint32_t>> lines_;
};

} // namespace tweedledum
