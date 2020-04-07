/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../../networks/storage.hpp"
#include "../../../networks/wire_id.hpp"
#include "../../../utils/device.hpp"

#include <vector>

namespace tweedledum::detail {

template<typename Network>
class line_placer {
	using op_type = typename Network::op_type;
	using node_type = typename Network::node_type;
	using pair_type = std::pair<uint32_t, uint32_t>;
public:
	line_placer(Network const& network, device const& device)
	    : network_(network)
	    , device_(device)
	    , v_degree_(num_v(), 0u)
	    , phy_degree_(num_phy(), 0u)
	    , wire_to_v_(network_.num_wires(), wire::invalid)
	    , phy_to_v_(num_phy(), wire::invalid)
	    , timeframes_(1u)
	{
		uint32_t v = 0u;
		network.foreach_wire([&](wire_id id) {
			if (id.is_qubit()) {
				wire_to_v_.at(id) = wire_id(v, true);
				++v;
			}
		});
	}

	std::vector<wire_id> place()
	{
		partition_into_timeframes();
		build_connectivity_graph();
		extract_lines();
		place_lines();

		std::vector<wire_id> v_to_phy(num_phy(), wire::invalid);
		for (uint32_t i = 0u; i < phy_to_v_.size(); ++i) {
			if (phy_to_v_.at(i) == wire::invalid) {
				continue;
			}
			v_to_phy.at(phy_to_v_.at(i)) = wire_id(i, true);
		}
		return v_to_phy;
	}

private:
	// Returns tht number of *virtual* qubits.
	uint32_t num_v() const
	{
		return network_.num_qubits();
	}

	// Returns tht number of *physical* qubits.
	uint32_t num_phy() const
	{
		return device_.num_qubits();
	}

	// Partition the quantum circuit into timesteps.  The circuit structure provides a natural
	// partial ordering of the gates; thus a greedy algorithm starting from inputs can divide
	// the input circuit into "vertical" partitions of gates which can be executed
	// simultaneously.
	void partition_into_timeframes()
	{
		network_.clear_values();
		network_.foreach_op([&](op_type const& op, node_type const& node) {
			uint32_t max_timeframe = 0u;
			network_.foreach_child(node, [&](node_type const& child) {
				max_timeframe = std::max(max_timeframe, network_.value(child));
			});
			if (op.is_one_qubit()) {
				network_.value(node, max_timeframe);
			} else {
				network_.value(node, ++max_timeframe);
				uint32_t const control = wire_to_v_.at(op.control());
				uint32_t const target = wire_to_v_.at(op.target());
				if (max_timeframe == timeframes_.size()) {
					timeframes_.emplace_back();
				}
				timeframes_.at(max_timeframe).emplace_back(control, target);
			}
		});
		network_.clear_values();
	}

	// Iterate over the timesteps to construct a graph whose vertices are qubits.  At each
	// timestep, I add the edge (q0, q1) to the graph if:
	//     (1) this pair is present in the timestep (duh!)
	//     (2) both qubits q0 and q1 have degree less than 2
	// Each connected component of the resulting graph is necessarily either a line or a ring;
	// the rings are broken by removing an arbitrarily chosen edge.
	//
	// NOTE: I hanve no idea if this is random or not :(  I could use, for example:
	//     (3) if the addition of this edge does not introduce a cycle
	//
	void build_connectivity_graph()
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

	int find_next_line_node(uint32_t const root) {
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

	void extract_lines()
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

	int pick_neighbor(uint32_t const phy)
	{
		int max_degree_neighbor = -1;
		device_.foreach_neighbor(phy, [&](uint32_t const neighbor) {
			if (phy_to_v_.at(neighbor) != wire::invalid) {
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

	void place_lines()
	{
		uint32_t max_degree_phy = 0u;
		for (uint32_t phy = 0u; phy < phy_degree_.size(); ++phy) {
			phy_degree_.at(phy) = device_.degree(phy);
			if (device_.degree(max_degree_phy) < device_.degree(phy)) {
				max_degree_phy = phy;
			}
		}

		for (std::vector<uint32_t> const& line : lines_) {
			phy_to_v_.at(max_degree_phy) = wire_id(line.at(0), true);
			--phy_degree_.at(max_degree_phy);
			for (uint32_t i = 1u; i < line.size(); ++i) {
				int neighbor = pick_neighbor(max_degree_phy);
				if (neighbor == -1) {
					break;
				}
				phy_to_v_.at(neighbor) = wire_id(line.at(i), true);
				--phy_degree_.at(neighbor);
			}
			for (uint32_t i = 0u; i < phy_degree_.size(); ++i) {
				if (phy_degree_.at(max_degree_phy) < phy_degree_.at(i)) {
					max_degree_phy = i;
				}
			}
		}
	}

private:
	Network const& network_;
	device const& device_;

	std::vector<uint32_t> v_degree_;
	std::vector<uint32_t> phy_degree_;
	std::vector<wire_id> wire_to_v_;
	std::vector<wire_id> phy_to_v_;
	std::vector<std::vector<pair_type>> timeframes_;
	std::vector<pair_type> connectivity_graph_;
	std::vector<std::vector<uint32_t>> lines_;
};

// From https://drops.dagstuhl.de/opus/volltexte/2019/10397/pdf/LIPIcs-TQC-2019-5.pdf:
// Side effect: clear node values in the network
template<typename Network>
std::vector<wire_id> line_placement(Network const& network, device const& device)
{
	line_placer placer(network, device);
	return placer.place();
}

// template<typename Network>
// std::vector<wire_id> line_placement(Network const& network, device const& device)
// {
// 	using op_type = typename Network::op_type;
// 	using node_type = typename Network::node_type;
// 	using pair_type = std::pair<uint32_t, uint32_t>;

// 	// Partition the quantum circuit into timesteps.  The circuit structure provides a natural
// 	// partial ordering of the gates; thus a greedy algorithm starting from inputs can divide
// 	// the input circuit into "vertical" partitions of gates which can be executed
// 	// simultaneously.
// 	uint32_t const num_qubits = network.num_qubits();
// 	std::vector<std::vector<pair_type>> timeframes(1u);
// 	network.clear_values();
// 	network.foreach_op([&](op_type const& op, node_type const& node) {
// 		uint32_t max_timeframe = 0u;
// 		network.foreach_child(node, [&](node_type const& child) {
// 			max_timeframe = std::max(max_timeframe, network.value(child));
// 		});
// 		if (op.is_one_qubit()) {
// 			network.value(node, max_timeframe);
// 		} else {
// 			network.value(node, ++max_timeframe);
// 			uint32_t const control = op.control();
// 			uint32_t const target = op.target();
// 			if (max_timeframe == timeframes.size()) {
// 				timeframes.emplace_back();
// 			}
// 			timeframes.at(max_timeframe).emplace_back(control, target);
// 		}
// 	});
// 	network.clear_values();
// 	fmt::print("Number of timeframes: {}\n", timeframes.size());

// 	// Iterate over the timesteps to construct a graph whose vertices are qubits.  At each
// 	// timestep, I add the edge (q0, q1) to the graph if:
// 	//     (1) this pair is present in the timestep (duh!)
// 	//     (2) both qubits q0 and q1 have degree less than 2
// 	// Each connected component of the resulting graph is necessarily either a line or a ring;
// 	// the rings are broken by removing an arbitrarily chosen edge.
// 	//
// 	// NOTE: I hanve no idea if this is random or not :(  I could use, for example:
// 	//     (3) if the addition of this edge does not introduce a cycle
// 	//
// 	std::vector<pair_type> edges;
// 	std::vector<uint32_t> degree(num_qubits, 0u);
// 	std::vector<int> parent(num_qubits, -1);
// 	auto find_root = [&parent](uint32_t i) -> uint32_t {
// 		while (parent.at(i) != -1) {
// 			i = parent.at(i);
// 		}
// 		return i;
// 	};
// 	for (std::vector<pair_type> const& timeframe : timeframes) {
// 		for (auto const& [q0, q1] : timeframe) {
// 			if (degree.at(q0) >= 2u || degree.at(q1) >= 2u) {
// 				continue;
// 			}
// 			uint32_t q0_root = find_root(q0);
// 			uint32_t q1_root = find_root(q1);
// 			if (q0_root == q1_root) {
// 				// This pair will make the graph a ring!
// 				continue;
// 			}
// 			parent.at(q0_root) = q1_root;
// 			edges.emplace_back(q0, q1);
// 			++degree.at(q0);
// 			++degree.at(q1);
// 			// fmt::print("{} : {}\n", q0, q1);
// 		}
// 		// fmt::print("{}\n", fmt::join(parent, ", "));
// 	}
// 	fmt::print("{}\n", fmt::join(parent, ", "));
// 	fmt::print("edges: {}\n", edges.size());
// 	for (uint32_t edge = 0; edge < edges.size(); ++edge) {
// 		auto& [u, v] = edges.at(edge);
// 		fmt::print("{} : {}\n", u, v);
// 	}
// 	fmt::print("\n");

// 	// Extract lines from the graph
// 	auto find_next = [&edges](uint32_t const root) -> int {
// 		int result = -1;
// 		for (uint32_t edge = 0; edge < edges.size(); ++edge) {
// 			auto& [u, v] = edges.at(edge);
// 			if (u == root) {
// 				result = v;
// 				v = -1;
// 				break;
// 			} else if (v == root) {
// 				result = u;
// 				u = -1;
// 				break;
// 			}
// 		}
// 		return result;
// 	};
// 	std::vector<std::vector<uint32_t>> lines;
// 	for (uint32_t i = 0u; i < degree.size(); ++i) {
// 		if (degree.at(i) != 1) {
// 			continue;
// 		}
// 		// fmt::print("{}\n", i);
// 		lines.emplace_back(1, i);
// 		int next = find_next(i);
// 		while (next != -1) {
// 			lines.back().emplace_back(next);
// 			next = find_next(next);
// 			// for (uint32_t edge = 0; edge < edges.size(); ++edge) {
// 			// 	auto& [u, v] = edges.at(edge);
// 			// 	fmt::print("{} : {}\n", u, v);
// 			// }
// 			// fmt::print("\n");
// 		}
// 		--degree.at(lines.back().back());
// 		// fmt::print("size: {}, {}\n", lines.back().size(), subset_size.at(i));
// 		// assert(lines.back().size() == subset_size.at(i));
// 	}
// 	std::sort(lines.begin(), lines.end(),
// 	          [](auto const& a, auto const& b) { return a.size() > b.size(); });
// 	fmt::print("Number of subsets: {}\n", lines.size());
// 	// fmt::print("Biggest parent: {} (size: {})\n", biggest_subset, max_size);

// 	// If the architecture is Hamiltonian connected then it is possible to map the qubit graph
// 	// as one long line starting from a high degree qubit, and greedily choosing the highest
// 	// degree available neighbour.
// 	uint32_t max_degree_qubit = 0u;
// 	std::vector<uint32_t> qubit_degree(device.num_qubits(), 0u);
// 	for (uint32_t qubit = 0u; qubit < device.num_qubits(); ++qubit) {
// 		qubit_degree.at(qubit) = device.degree(qubit);
// 		if (device.degree(max_degree_qubit) < device.degree(qubit)) {
// 			max_degree_qubit = qubit;
// 		}
// 	}

// 	std::vector<wire_id> phy_to_v(device.num_qubits(), wire::invalid);

// 	auto pick_neighbor = [&phy_to_v, &device, &qubit_degree](uint32_t const phy) {
// 		int max_degree_neighbor = -1;
// 		device.foreach_neighbor(phy, [&](uint32_t const neighbor) {
// 			if (phy_to_v.at(neighbor) != wire::invalid) {
// 				return;
// 			}
// 			if (max_degree_neighbor == -1) {
// 				max_degree_neighbor = neighbor;
// 				return;
// 			}
// 			if (qubit_degree.at(max_degree_neighbor) < qubit_degree.at(neighbor)) {
// 				max_degree_neighbor = neighbor;
// 			}
// 		});
// 		return max_degree_neighbor;
// 	};

// 	for (std::vector<uint32_t> const& line : lines) {
// 		fmt::print("{}\n", line.size());
// 		phy_to_v.at(max_degree_qubit) = wire_id(line.at(0), true);
// 		--qubit_degree.at(max_degree_qubit);
// 		for (uint32_t i = 1u; i < line.size(); ++i) {
// 			int neighbor = pick_neighbor(max_degree_qubit);
// 			if (neighbor == -1) {
// 				break;
// 			}
// 			phy_to_v.at(neighbor) = wire_id(line.at(i), true);
// 			--qubit_degree.at(neighbor);
// 		}
// 		for (uint32_t i = 0u; i < qubit_degree.size(); ++i) {
// 			if (qubit_degree.at(max_degree_qubit) < qubit_degree.at(i)) {
// 				max_degree_qubit = i;
// 			}
// 		}
// 	}

// 	std::vector<wire_id> v_to_phy(device.num_qubits(), wire::invalid);
// 	for (uint32_t i = 0u; i < phy_to_v.size(); ++i) {
// 		if (phy_to_v.at(i) == wire::invalid) {
// 			continue;
// 		}
// 		v_to_phy.at(phy_to_v.at(i)) = wire_id(i, true);
// 	}
// 	fmt::print("map:\n");
// 	for (uint32_t i = 0u; i < v_to_phy.size(); ++i) {
// 		fmt::print("{}\n", v_to_phy.at(i));
// 	}
// 	return v_to_phy;
// }

} // namespace tweedledum::detail
