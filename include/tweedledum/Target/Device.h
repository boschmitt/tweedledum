/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <Eigen/Dense>
#include <algorithm>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>
#include <vector>

namespace tweedledum {

class Device {
public:
    using Edge = std::pair<uint32_t, uint32_t>;

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

    static Device from_edge_list(std::vector<Edge> const& edges);

    static Device from_json(nlohmann::json const& device_info);

    static Device from_file(std::string const& filename);

    Device(uint32_t const num_qubits, std::string_view name = {})
        : name_(name)
        , neighbors_(num_qubits)
        , dist_matrix_()
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

    Edge const& edge(uint32_t const i) const
    {
        return edges_.at(i);
    }

    bool are_connected(uint32_t const v, uint32_t const u) const
    {
        assert(v <= num_qubits() && u <= num_qubits());
        if (!shortest_path_.empty()) {
            return distance(v, u) == 1u;
        }
        Edge const edge = {std::min(v, u), std::max(u, v)};
        auto const search = std::find(edges_.begin(), edges_.end(), edge);
        if (search == edges_.end()) {
            return false;
        }
        return true;
    }

    /*! \brief Get a shortest path between two qubits
     *
     * Paths are computed once and cached.  Since I assume a undirected graph,
     * I can save some space by only storing the path between `begin` to `end`,
     * where `begin` < `end`.  If we are interested in the other direction, i.e,
     * `begin` > `end` we just need to reverse the stored path.
     *
     * TODO: When considering the fidelity of qubits the cost of (u, v) might be
     * different than the cost of (v, u). (distance changes too!!)
     *
     * \param[in] begin The starting qubit
     * \param[in] end The ending qubit
     * \return A shortest path between represented as a vector of qubits
     */
    std::vector<uint32_t> shortest_path(uint32_t begin, uint32_t end) const
    {
        assert(begin < num_qubits() && end < num_qubits());
        if (begin == end) {
            return {};
        }
        if (shortest_path_.empty()) {
            compute_shortest_paths();
        }
        uint32_t const idx = triangle_idx(begin, end);
        std::vector<uint32_t> result = shortest_path_.at(idx);
        if (begin > end) {
            std::reverse(result.begin(), result.end());
        }
        return result;
    }

    /*! \brief Get the distance of a shortest path between two qubits
     *
     * \param[in] begin The starting qubit
     * \param[in] end The ending qubit
     * \return The length of a shortest path between the qubits
     */
    uint32_t distance(uint32_t begin, uint32_t end) const
    {
        assert(begin < num_qubits() && end < num_qubits());
        if (begin == end) {
            return 0;
        }
        if (shortest_path_.empty()) {
            compute_shortest_paths();
        }
        uint32_t const idx = triangle_idx(begin, end);
        return shortest_path_.at(idx).size() - 1;
    }

    std::vector<Device::Edge> steiner_tree(
      std::vector<uint32_t> terminals, uint32_t root) const;

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
    void compute_shortest_paths() const;

    uint32_t triangle_idx(uint32_t i, uint32_t j) const
    {
        if (i > j) {
            std::swap(i, j);
        }
        return i * num_qubits() - (i - 1) * i / 2 + j - i;
    }

private:
    std::string name_;
    std::vector<std::vector<uint32_t>> neighbors_;
    std::vector<Edge> edges_;
    mutable std::vector<std::vector<uint32_t>> dist_matrix_;
    mutable std::vector<std::vector<uint32_t>> shortest_path_;
};

inline void Device::compute_shortest_paths() const
{
    Eigen::MatrixXi shortest_paths(num_qubits(), num_qubits());
    Eigen::MatrixXi dist(num_qubits(), num_qubits());

    // All-pairs shortest paths
    for (uint32_t i = 0; i < num_qubits(); i++) {
        for (uint32_t j = 0; j < num_qubits(); j++) {
            if (i == j) {
                dist(i, j) = 0;
                shortest_paths(i, j) = j;
            } else if (are_connected(i, j)) {
                dist(i, j) = 1;
                shortest_paths(i, j) = j;
            } else {
                dist(i, j) = num_qubits();
                shortest_paths(i, j) = num_qubits();
            }
        }
    }

    for (uint32_t k = 0u; k < num_qubits(); ++k) {
        for (uint32_t i = 0u; i < num_qubits(); ++i) {
            for (uint32_t j = 0u; j < num_qubits(); ++j) {
                if (dist(i, j) > dist(i, k) + dist(k, j)) {
                    dist(i, j) = dist(i, k) + dist(k, j);
                    shortest_paths(i, j) = shortest_paths(i, k);
                }
            }
        }
    }

    shortest_path_.resize(num_qubits() * (num_qubits() + 1) / 2);
    for (uint32_t i = 0u; i < num_qubits(); ++i) {
        for (uint32_t j = i + 1; j < num_qubits(); ++j) {
            uint32_t const idx = triangle_idx(j, i);
            uint32_t current = i;
            shortest_path_.at(idx).push_back(i);
            while (current != j) {
                current = shortest_paths(current, j);
                shortest_path_.at(idx).push_back(current);
            }
        }
    }
}

/*! \brief Get an approximation to a minimal Steiner tree
 *
 * Given a set of terminal nodes and a root node in the coupling graph,
 * attempts to find a minimal weight set of edges connecting the root to
 * each terminal.
 *
 * Steiner Tree problem is NP-Hard.  Bellow is one simple approximate algorithm
 * based on Shortest Path. (Usually computing all shortest paths is considered
 * a disadvantage, but in this class I do compute them for other things.)
 *
 * Outline:
 *   1. Start with a subtree T consisting of one given terminal vertex
 *   2. While T does not span all terminals
 *        a) Select a terminal x not in T that is closest to a vertex in T.
 *        b) Add to T the shortest path that connects x with T.
 *
 * The algorithm is (2-2/n) approximate
 *
 * \param[in] terminals A vector of terminal qubits to be connected
 * \param[in] root A root for the Steiner tree
 * \return A spanning tree represented as a vector of edges
 */
inline std::vector<Device::Edge> Device::steiner_tree(
  std::vector<uint32_t> terminals, uint32_t root) const
{
    if (terminals.empty()) {
        return {};
    }
    // The steiner tree
    std::vector<Device::Edge> tree;

    // Internal data structures
    std::vector<uint32_t> vertex_cost(num_qubits());
    std::vector<uint32_t> edge_in(num_qubits(), root);
    std::vector<uint8_t> in_tree(num_qubits(), 0u);
    in_tree.at(root) = 1u;

    auto add_path = [&](std::vector<uint32_t> const& path) {
        std::vector<uint32_t> vertices;
        if (path.empty()) {
            return vertices;
        }
        // Deal with the first element:
        if (in_tree.at(path.back())) {
            return std::vector<uint32_t>(1, path.back());
        }
        in_tree.at(path.back()) = 1;
        vertices.push_back(path.back());
        uint32_t begin = tree.size();
        for (auto it = path.rbegin() + 1; it != path.rend(); ++it) {
            tree.emplace_back(*it, *(it - 1));
            if (in_tree.at(*it)) {
                break;
            }
            vertices.push_back(*it);
            in_tree.at(*it) = 1;
        }
        std::reverse(tree.begin() + begin, tree.end());
        return vertices;
    };

    // Choose minimal vertex, i.e., vertex closest to the root
    auto min_vertex = terminals.begin();
    vertex_cost.at(*min_vertex) = distance(root, *min_vertex);
    for (auto it = terminals.begin() + 1; it != terminals.end(); ++it) {
        uint32_t const vertex = *it;
        vertex_cost.at(vertex) = distance(root, vertex);
        if (vertex_cost.at(vertex) < vertex_cost.at(*min_vertex)) {
            min_vertex = it;
        }
    }

    // While `tree` does not span all terminals
    while (!terminals.empty()) {
        uint32_t curr_vertex = *min_vertex;
        terminals.erase(min_vertex);
        auto const st = shortest_path(edge_in.at(curr_vertex), curr_vertex);
        auto const new_vertices = add_path(st);
        // Update costs and select a new minimal vertex
        min_vertex = terminals.begin();
        for (auto it = terminals.begin(); it != terminals.end(); ++it) {
            uint32_t const vertex = *it;
            for (uint32_t new_vertex : new_vertices) {
                if (distance(new_vertex, vertex) < vertex_cost.at(vertex)) {
                    vertex_cost.at(vertex) = distance(new_vertex, vertex);
                    edge_in.at(vertex) = new_vertex;
                }
            }
            vertex_cost.at(vertex) = distance(root, vertex);
            if (vertex_cost.at(vertex) < vertex_cost.at(*min_vertex)) {
                min_vertex = it;
            }
        }
    }
    return tree;
}

} // namespace tweedledum
