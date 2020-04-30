/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../../networks/storage.hpp"
#include "../../../networks/wire.hpp"
#include "../../../target/device.hpp"

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
	    , wire_to_v_(network_.num_wires(), wire::invalid_id)
	    , phy_to_v_(num_phy(), wire::invalid_id)
	    , timeframes_(1u)
	{
		uint32_t v = 0u;
		network.foreach_wire([&](wire::id id) {
			if (id.is_qubit()) {
				wire_to_v_.at(id) = wire::make_qubit(v);
				++v;
			}
		});
	}

	std::vector<wire::id> place()
	{
		partition_into_timeframes();
		build_connectivity_graph();
		extract_lines();
		place_lines();

		std::vector<wire::id> v_to_phy(num_phy(), wire::invalid_id);
		for (uint32_t i = 0u; i < phy_to_v_.size(); ++i) {
			if (phy_to_v_.at(i) == wire::invalid_id) {
				continue;
			}
			v_to_phy.at(phy_to_v_.at(i)) = wire::make_qubit(i);
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
			if (phy_to_v_.at(neighbor) != wire::invalid_id) {
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
			phy_to_v_.at(max_degree_phy) = wire::make_qubit(line.at(0));
			--phy_degree_.at(max_degree_phy);
			for (uint32_t i = 1u; i < line.size(); ++i) {
				int neighbor = pick_neighbor(max_degree_phy);
				if (neighbor == -1) {
					break;
				}
				phy_to_v_.at(neighbor) = wire::make_qubit(line.at(i));
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
	std::vector<wire::id> wire_to_v_;
	std::vector<wire::id> phy_to_v_;
	std::vector<std::vector<pair_type>> timeframes_;
	std::vector<pair_type> connectivity_graph_;
	std::vector<std::vector<uint32_t>> lines_;
};

// From https://drops.dagstuhl.de/opus/volltexte/2019/10397/pdf/LIPIcs-TQC-2019-5.pdf:
// Side effect: clear node values in the network

/*! \brief Yet to be written.
 */
template<typename Network>
std::vector<wire::id> line_placement(Network const& network, device const& device)
{
	line_placer placer(network, device);
	return placer.place();
}

} // namespace tweedledum::detail
