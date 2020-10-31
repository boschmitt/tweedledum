/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <algorithm>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <utility>

namespace tweedledum {

class Device {
public:
    using edge_type = std::pair<uint32_t, uint32_t>;

#pragma region Generic topologies
    // Create a device for a path topology.
    static Device path(uint32_t const num_qubits)
    {
        Device topology(num_qubits);
        for (uint32_t i = 1u; i < num_qubits; ++i) {
            topology.add_edge(i - 1, i);
        }
        return topology;
    }

    // Create a device for a ring topology.
    static Device ring(uint32_t const num_qubits)
    {
        Device topology(num_qubits);
        for (uint32_t i = 0u; i < num_qubits; ++i) {
            topology.add_edge(i, (i + 1) % num_qubits);
        }
        return topology;
    }

    // Create a device for a star topology.
    static Device star(uint32_t const num_qubits)
    {
        Device topology(num_qubits);
        for (uint32_t i = 1u; i < num_qubits; ++i) {
            topology.add_edge(0, i);
        }
        return topology;
    }

    // Create a device for a grid topology.
    static Device grid(uint32_t const width, uint32_t const height)
    {
        Device topology(width * height);
        for (uint32_t x = 0u; x < width; ++x) {
            for (uint32_t y = 0u; y < height; ++y) {
                uint32_t e = y * width + x;
                if (x < width - 1) {
                    topology.add_edge(e, e + 1);
                }
                if (y < height - 1) {
                    topology.add_edge(e, e + width);
                }
            }
        }
        return topology;
    }
#pragma endregion

    Device(uint32_t const num_qubits, std::string_view name = {})
        : name_(name), neighbors_(num_qubits), dist_matrix_()
    {}

#pragma region Qubits
    uint32_t num_qubits() const
    {
        return neighbors_.size();
    }

    uint32_t degree(uint32_t const qubit) const
    {
        return neighbors_.at(qubit).size();
    }

    template<typename Fn>
    void foreach_neighbor(uint32_t const qubit, Fn&& fn) const
    {
        static_assert(std::is_invocable_r_v<void, Fn, uint32_t const>);
        std::vector<uint32_t> const& neighbors = neighbors_.at(qubit);
        for (uint32_t i = 0u, i_limit = neighbors.size(); i < i_limit; ++i) {
            fn(neighbors.at(i));
        }
    }
#pragma endregion

#pragma region Edges
    uint32_t num_edges() const
    {
        return edges_.size();
    }

    edge_type const& edge(uint32_t const i) const
    {
        return edges_.at(i);
    }

    bool are_connected(uint32_t const v, uint32_t const u) const
    {
        assert(v <= num_qubits() && u <= num_qubits());
        if (!dist_matrix_.empty()) {
            return distance(v, u) == 1u;
        }
        edge_type const edge = {std::min(v, u), std::max(u, v)};
        auto const search = std::find(edges_.begin(), edges_.end(), edge);
        if (search == edges_.end()) {
            return false;
        }
        return true;
    }

    uint32_t distance(uint32_t const v, uint32_t const u) const
    {
        assert(v < num_qubits() && u < num_qubits());
        if (dist_matrix_.empty()) {
            compute_distance_matrix();
        }
        return dist_matrix_[v][u];
    }

    /*! \brief Add an _undirected_ edge between two qubits */
    void add_edge(uint32_t const v, uint32_t const u)
    {
        assert(v <= num_qubits() && u <= num_qubits());
        if (!are_connected(v, u)) {
            edges_.emplace_back(std::min(v, u), std::max(u, v));
            neighbors_.at(v).emplace_back(u);
            neighbors_.at(u).emplace_back(v);
        }
    }
#pragma endregion

private:
    void compute_distance_matrix() const;

private:
    std::string name_;
    std::vector<std::vector<uint32_t>> neighbors_;
    std::vector<edge_type> edges_;
    mutable std::vector<std::vector<uint32_t>> dist_matrix_;
};

inline void Device::compute_distance_matrix() const
{
    dist_matrix_.resize(num_qubits(), std::vector<uint32_t>(num_qubits(), num_qubits() + 1));
    for (uint32_t v = 0u; v < num_qubits(); ++v) {
        dist_matrix_[v][v] = 0;
    }
    for (auto const& [v, w] : edges_) {
        dist_matrix_[v][w] = 1;
        dist_matrix_[w][v] = 1;
    }
    for (uint32_t k = 0u; k < num_qubits(); ++k) {
        for (uint32_t i = 0u; i < num_qubits(); ++i) {
            for (uint32_t j = 0u; j < num_qubits(); ++j) {
                if (dist_matrix_[i][j] > dist_matrix_[i][k] + dist_matrix_[k][j]) {
                    dist_matrix_[i][j] = dist_matrix_[i][k] + dist_matrix_[k][j];
                }
            }
        }
    }
}

Device read_device_from_json(std::string const& filename);

} // namespace tweedledum
