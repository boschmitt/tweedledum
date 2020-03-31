/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "bit_matrix_rm.hpp"

#include <chrono>
#include <cstdint>
#include <random>
#include <string>
#include <utility>
#include <vector>

namespace tweedledum {
/*! \brief Data-structure for the architecture of a quantum device.
 *
 * This data structure encapsulates the most essential properties of a physical quantum device used
 * by our mapping algorithms. These are the number of qubits and an undirected coupling graph
 * describing which pairs of qubits can interact with each other.
 */
struct device {
	using edge_type = std::pair<uint32_t, uint32_t>;

#pragma region Generic topologies
	/*! \brief Create a device for a path topology. */
	static device path(uint32_t num_qubits)
	{
		device topology(num_qubits);
		for (uint32_t i = 1; i < num_qubits; ++i) {
			topology.add_edge(i - 1, i);
		}
		return topology;
	}

	/*! \brief Create a device for a ring topology. */
	static device ring(uint32_t num_qubits)
	{
		device topology(num_qubits);
		for (uint32_t i = 0; i < num_qubits; ++i) {
			topology.add_edge(i, (i + 1) % num_qubits);
		}
		return topology;
	}

	/*! \brief Create a device for a star topology. */
	static device star(uint32_t num_qubits)
	{
		device topology(num_qubits);
		for (uint32_t i = 1; i < num_qubits; ++i) {
			topology.add_edge(0, i);
		}
		return topology;
	}

	/*! \brief Create a device for a grid topology.
	 *
	 * The device has `width * height` number of qubits.
	 *
	 * \param width Width of the grid
	 * \param height Height of the grid
	 */
	static device grid(uint32_t width, uint32_t height)
	{
		device topology(width * height);
		for (uint32_t x = 0; x < width; ++x) {
			for (uint32_t y = 0; y < height; ++y) {
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

	/*! \brief Creates a device with a random topology.
	 *
	 * \param num_edges Number of coupling_map_ in coupling graph
	 */
	static device random(uint32_t num_qubits, uint32_t num_edges)
	{
		std::default_random_engine gen(
		    std::chrono::system_clock::now().time_since_epoch().count());
		std::uniform_int_distribution<uint32_t> d1(0, num_qubits - 1);
		std::uniform_int_distribution<uint32_t> d2(1, num_qubits - 2);

		device topology(num_qubits);
		while (topology.coupling_map_.size() != num_edges) {
			uint32_t p = d1(gen);
			uint32_t q = (p + d2(gen)) % num_qubits;
			topology.add_edge(p, q);
		}
		return topology;
	}
#pragma endregion

	device(uint32_t const num_qubits = 0, std::string_view name = {})
	    : num_qubits_(num_qubits)
	    , name_(name)
	{}

	uint32_t num_qubits() const
	{
		return num_qubits_;
	}

	uint32_t num_edges() const
	{
		return coupling_map_.size();
	}

	/*! \brief Add an _undirected_ edge between two qubits */
	void add_edge(uint32_t const v, uint32_t const w)
	{
		assert(v <= num_qubits() && w <= num_qubits());
		edge_type const edge = {std::min(v, w), std::max(w, v)};
		auto const search = std::find(coupling_map_.begin(), coupling_map_.end(), edge);
		if (search == coupling_map_.end()) {
			coupling_map_.emplace_back(edge);
		}
	}

	edge_type const& edge(uint32_t const i) const
	{
		return coupling_map_.at(i);
	}

	/*! \brief Returns adjacency matrix of coupling graph. */
	auto coupling_map() const
	{
		return coupling_map_;
	}

	/*! \brief Returns adjacency matrix of coupling graph. */
	bit_matrix_rm<uint32_t> coupling_matrix() const
	{
		bit_matrix_rm<uint32_t> matrix(num_qubits(), num_qubits());

		for (auto const& [v, w] : coupling_map_) {
			matrix.at(v, w) = true;
			matrix.at(w, v) = true;
		}
		return matrix;
	}

	/*! \brief Returns the distance between nodes `v` and `u`. */
	uint32_t distance(uint32_t const v, uint32_t const u) const
	{
		assert(v < num_qubits() && u < num_qubits());
		if (dist_matrix_.empty()) {
			compute_distance_matrix();
		}
		return dist_matrix_[v][u];
	}

	/*! \brief Returns distance matrix of coupling graph. */
	std::vector<std::vector<uint32_t>> distance_matrix() const
	{
		if (dist_matrix_.empty()) {
			compute_distance_matrix();
		}
		return dist_matrix_;
	}

private:
	/*! \brief Returns distance matrix of coupling graph. */
	void compute_distance_matrix() const
	{
		dist_matrix_.resize(num_qubits(),
		                    std::vector<uint32_t>(num_qubits(), num_qubits() + 1));
		for (auto const& [v, w] : coupling_map_) {
			dist_matrix_[v][w] = 1;
			dist_matrix_[w][v] = 1;
		}
		for (uint32_t v = 0u; v < num_qubits(); ++v) {
			dist_matrix_[v][v] = 0;
		}
		for (uint32_t k = 0u; k < num_qubits(); ++k) {
			for (uint32_t i = 0u; i < num_qubits(); ++i) {
				for (uint32_t j = 0u; j < num_qubits(); ++j) {
					if (dist_matrix_[i][j] > dist_matrix_[i][k] + dist_matrix_[k][j]) {
						dist_matrix_[i][j] = dist_matrix_[i][k]
						                     + dist_matrix_[k][j];
					}
				}
			}
		}
	}

private:
	uint32_t num_qubits_;
	std::string name_;
	std::vector<edge_type> coupling_map_;
	mutable std::vector<std::vector<uint32_t>> dist_matrix_;
};

} // namespace tweedledum
