/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../utils/bit_matrix_rm.hpp"

#include <chrono>
#include <cstdint>
#include <random>
#include <utility>
#include <vector>

namespace tweedledum {
struct device_t {
	std::vector<std::pair<uint8_t, uint8_t>> edges;
	uint8_t num_vertices;

	static device_t ring(uint8_t m)
	{
		std::vector<std::pair<uint8_t, uint8_t>> edges;
		for (auto i = 0; i < m; ++i) {
			edges.emplace_back(i, (i + 1) % m);
		}
		return {edges, m};
	}

	static device_t star(uint8_t m)
	{
		std::vector<std::pair<uint8_t, uint8_t>> edges;
		for (auto i = 1; i < m; ++i) {
			edges.emplace_back(0, i);
		}
		return {edges, m};
	}

	static device_t grid(uint8_t w, uint8_t h)
	{
		std::vector<std::pair<uint8_t, uint8_t>> edges;
		for (auto x = 0; x < w; ++x) {
			for (auto y = 0; y < h; ++y) {
				auto e = y * w + x;
				if (x < w - 1) {
					edges.emplace_back(e, e + 1);
				}
				if (y < h - 1) {
					edges.emplace_back(e, e + w);
				}
			}
		}
		return {edges, static_cast<uint8_t>(w * h)};
	}

	static device_t random(uint8_t m, uint8_t num_edges)
	{
		std::default_random_engine gen(
		    std::chrono::system_clock::now().time_since_epoch().count());
		std::uniform_int_distribution<uint8_t> d1(0, m - 1);
		std::uniform_int_distribution<uint8_t> d2(1, m - 2);

		std::vector<std::pair<uint8_t, uint8_t>> edges;
		while (edges.size() != num_edges) {
			uint8_t p = d1(gen);
			uint8_t q = (p + d2(gen)) % m;
			std::pair<uint8_t, uint8_t> edge{std::min(p, q), std::max(p, q)};
			if (std::find(edges.begin(), edges.end(), edge) == edges.end()) {
				edges.push_back(edge);
			}
		}

		return {edges, m};
	}

	bit_matrix_rm<> get_coupling_matrix() const
	{
		bit_matrix_rm<> mat{num_vertices, num_vertices};

		for (auto const& [v, w] : edges) {
			mat.at(v, w) = true;
			mat.at(w, v) = true;
		}

		return mat;
	}
};

} // namespace tweedledum