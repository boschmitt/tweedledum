/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../../IR/Circuit.h"
#include "../../../IR/Qubit.h"
#include "../../../Target/Device.h"
#include "../../../Target/Placement.h"

#include <vector>

namespace tweedledum {

class LinePlacer {
public:
    LinePlacer(Device const& device, Circuit const& original)
        : device_(device)
        , original_(original)
        , v_degree_(num_v(), 0u)
        , phy_degree_(num_phy(), 0u)
        , timeframes_(1u)
    {}

    std::optional<Placement> run()
    {
        partition_into_timeframes();
        build_connectivity_graph();
        extract_lines();
        return place_lines();
    }

private:
    using pair_type = std::pair<uint32_t, uint32_t>;

    // Returns the number of *virtual* qubits.
    uint32_t num_v() const
    {
        return original_.num_qubits();
    }

    // Returns the number of *physical* qubits.
    uint32_t num_phy() const
    {
        return device_.num_qubits();
    }

    void partition_into_timeframes();

    void build_connectivity_graph();

    int find_next_line_node(uint32_t const root);

    void extract_lines();

    Qubit pick_neighbor(Placement const& placement, uint32_t phy) const;

    std::optional<Placement> place_lines();

    Device const& device_;
    Circuit const& original_;

    std::vector<uint32_t> v_degree_;
    std::vector<uint32_t> phy_degree_;
    std::vector<std::vector<pair_type>> timeframes_;
    std::vector<pair_type> connectivity_graph_;
    std::vector<std::vector<uint32_t>> lines_;
};

/*! \brief Yet to be written.
 */
std::optional<Placement> line_place(
  Device const& device, Circuit const& original);

} // namespace tweedledum
