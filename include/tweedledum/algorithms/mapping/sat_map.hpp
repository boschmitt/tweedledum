/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../utils/device.hpp"
#include "../../views/mapping_view.hpp"

#include <bill/sat/cardinality.hpp>
#include <bill/sat/solver.hpp>

namespace tweedledum {

#pragma region Implementation details
namespace detail {

template<typename Network, typename Solver>
class map_cnf_encoder {
	using lbool_type = bill::lbool_type;
	using lit_type = bill::lit_type;
	using var_type = bill::var_type;

public:
	map_cnf_encoder(Network const& network, device const& device, Solver& solver)
	    : network_(network)
	    , device_(device)
	    , pairs_(network_.num_qubits() * (network_.num_qubits() + 1) / 2, 0u)
	    , solver_(solver)
	    , io_qid_map_(network.num_io(), io_invalid)
	{
		network.foreach_io([&](io_id io) {
			if (io.is_qubit()) {
				io_qid_map_.at(io.index()) = qid_io_map_.size();
				qid_io_map_.push_back(io);
			}
		});
	}

	std::vector<uint32_t> run_incrementally()
	{
		solver_.add_variables(num_virtual_qubits() * num_physical_qubits());
		qubits_constraints();
		network_.foreach_gate([&](auto const& node) {
			if (!node.gate.is(gate_lib::cx)) {
				return true;
			}
			uint32_t control = io_qid_map_.at(node.gate.control());
			uint32_t target = io_qid_map_.at(node.gate.target());
			if (pairs_[triangle_to_vector_idx(control, target)] == 0) {
				gate_constraints(control, target);
			}
			++pairs_[triangle_to_vector_idx(control, target)];
			solver_.solve();
			bill::result result = solver_.get_result();
			if (result) {
				model_= result.model();
			} else {
				return false;
			}
			return true;
		});
		return decode(model_);
	}

	std::vector<uint32_t> run_greedly()
	{
		solver_.add_variables(num_virtual_qubits() * num_physical_qubits());
		qubits_constraints();
		network_.foreach_gate([&](auto const& node) {
			if (!node.gate.is(gate_lib::cx)) {
				return;
			}
			uint32_t control = io_qid_map_.at(node.gate.control());
			uint32_t target = io_qid_map_.at(node.gate.target());
			++pairs_[triangle_to_vector_idx(control, target)];
		});

		std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> aa;
		for (uint32_t i = 0; i < num_virtual_qubits(); ++i) {
			for (uint32_t j = 0; j < num_virtual_qubits(); ++j) {
				if (pairs_[triangle_to_vector_idx(i, j)] != 0) {
					aa.emplace_back(i, j, pairs_[triangle_to_vector_idx(i, j)]);
					pairs_[triangle_to_vector_idx(i, j)] = 0;
				}
			}
		}
		std::sort(aa.begin(), aa.end(), [&](auto const& c0, auto const& c1) {
			return std::get<2>(c0) > std::get<2>(c1); 
		});
		std::vector<lit_type> assumptions;
		for (auto &&[control, target, score] : aa) {
			var_type act = gate_act_constraints(control, target);
			assumptions.emplace_back(act, bill::positive_polarity);
			solver_.solve(assumptions);
			bill::result result = solver_.get_result();
			if (result) {
				model_= result.model();
			} else {
				assumptions.back().complement();
			}
		}
		return decode(model_);
	}

	std::vector<uint32_t> run()
	{
		solver_.add_variables(num_virtual_qubits() * num_physical_qubits());
		qubits_constraints();
		network_.foreach_gate([&](auto const& node) {
			if (!node.gate.is(gate_lib::cx)) {
				return true;
			}
			uint32_t control = io_qid_map_.at(node.gate.control());
			uint32_t target = io_qid_map_.at(node.gate.target());
			if (pairs_[triangle_to_vector_idx(control, target)] == 0) {
				gate_constraints(control, target);
			}
			++pairs_[triangle_to_vector_idx(control, target)];
			solver_.solve();
		});
		solver_.solve();
		bill::result result = solver_.get_result();
		if (result.is_satisfiable()) {
			return decode(result.model());
		}
		return {};
	}

	void encode()
	{
		solver_.add_variables(num_virtual_qubits() * num_physical_qubits());
		qubits_constraints();
		network_.foreach_gate([&](auto const& node) {
			if (!node.gate.is(gate_lib::cx)) {
				return;
			}
			uint32_t control = io_qid_map_.at(node.gate.control());
			uint32_t target = io_qid_map_.at(node.gate.target());
			if (pairs_[triangle_to_vector_idx(control, target)] == 0) {
				gate_constraints(control, target);
			}
			++pairs_[triangle_to_vector_idx(control, target)];
		});
	}

	std::vector<uint32_t> decode(std::vector<lbool_type> const& model)
	{
		std::vector<uint32_t> mapping(network_.num_qubits(), 0);
		for (uint32_t v_qid = 0; v_qid < num_virtual_qubits(); ++v_qid) {
			for (uint32_t phy_qid = 0; phy_qid < num_physical_qubits(); ++phy_qid) {
				var_type var = virtual_physical_var(v_qid, phy_qid);
				if (model.at(var) == lbool_type::true_) {
					mapping.at(v_qid) = phy_qid;
					break;
				}
			}
		}
		for (uint32_t phy_qid = 0; phy_qid < num_physical_qubits(); ++phy_qid) {
			bool used = false;
			for (uint32_t v_qid = 0; v_qid < num_virtual_qubits(); ++v_qid) {
				var_type var = virtual_physical_var(v_qid, phy_qid);
				if (model.at(var) == lbool_type::true_) {
					used = true;
					break;
				}
			}
			if (!used) {
				mapping.push_back(phy_qid);
			}
		}
		return mapping;
	}

private:
	uint32_t num_physical_qubits() const
	{
		return device_.num_vertices();
	}

	uint32_t num_virtual_qubits() const
	{
		return network_.num_qubits();
	}

	void qubits_constraints()
	{
		// Condition 2:
		std::vector<bill::var_type> variables;
		for (auto v = 0u; v < num_virtual_qubits(); ++v) {
			for (auto p = 0u; p < num_physical_qubits(); ++p) {
				variables.emplace_back(virtual_physical_var(v, p));
			}
			at_least_one(variables, solver_);
			at_most_one_pairwise(variables, solver_);
			variables.clear();
		}

		// Condition 3:
		for (auto p = 0u; p < num_physical_qubits(); ++p) {
			for (auto v = 0u; v < num_virtual_qubits(); ++v) {
				variables.emplace_back(virtual_physical_var(v, p));
			}
			at_most_one_pairwise(variables, solver_);
			variables.clear();
		}
	}

	// Abbreviations:
	// - c_v_qid (control, virtual qubit identifier)
	// - t_v_qid (target, virtual qubit identifier)
	// - c_phy_qid (control, physical qubit identifier)
	// - t_phy_qid (target, physical qubit identifier)
	void gate_constraints(uint32_t c_v_qid, uint32_t t_v_qid)
	{
		bit_matrix_rm<uint32_t> const& coupling_matrix = device_.get_coupling_matrix();
		std::vector<bill::lit_type> clause;
		for (uint32_t t_phy_qid = 0; t_phy_qid < num_physical_qubits(); ++t_phy_qid) {
			uint32_t t_v_phy_var = virtual_physical_var(t_v_qid, t_phy_qid);
			clause.emplace_back(t_v_phy_var, bill::negative_polarity);
			for (uint32_t c_phy_qid = 0; c_phy_qid < num_physical_qubits(); ++c_phy_qid) {
				if (c_phy_qid == t_phy_qid || !coupling_matrix.at(c_phy_qid, t_phy_qid)) {
					continue;
				}
				uint32_t c_v_phy_var = virtual_physical_var(c_v_qid, c_phy_qid);
				clause.emplace_back(c_v_phy_var, bill::positive_polarity);
			}
			solver_.add_clause(clause);
			clause.clear();
		}
	}

	var_type gate_act_constraints(uint32_t c_v_qid, uint32_t t_v_qid)
	{
		bit_matrix_rm<uint32_t> const& coupling_matrix = device_.get_coupling_matrix();
		std::vector<bill::lit_type> clause;
		var_type act_var = solver_.add_variable();
		for (uint32_t t_phy_qid = 0; t_phy_qid < num_physical_qubits(); ++t_phy_qid) {
			uint32_t t_v_phy_var = virtual_physical_var(t_v_qid, t_phy_qid);
			clause.emplace_back(act_var, bill::negative_polarity);
			clause.emplace_back(t_v_phy_var, bill::negative_polarity);
			for (uint32_t c_phy_qid = 0; c_phy_qid < num_physical_qubits(); ++c_phy_qid) {
				if (c_phy_qid == t_phy_qid || !coupling_matrix.at(c_phy_qid, t_phy_qid)) {
					continue;
				}
				uint32_t c_v_phy_var = virtual_physical_var(c_v_qid, c_phy_qid);
				clause.emplace_back(c_v_phy_var, bill::positive_polarity);
			}
			solver_.add_clause(clause);
			clause.clear();
		}
		return act_var;
	}

private:
	bill::var_type virtual_physical_var(uint32_t virtual_id, uint32_t physical_id)
	{
		return virtual_id * num_physical_qubits() + physical_id;
	}

	uint32_t triangle_to_vector_idx(uint32_t i, uint32_t j)
	{
		if (i > j) {
			std::swap(i, j);
		}
		return i * num_virtual_qubits() - (i - 1) * i / 2 + j - i;
	}

private:
	// Problem data
	Network const& network_;
	device const& device_;
	std::vector<uint32_t> pairs_;

	//
	Solver& solver_;
	std::vector<lbool_type> model_;

	// Qubit normalization
	std::vector<uint32_t> io_qid_map_;
	std::vector<io_id> qid_io_map_;
};

template<typename Network>
std::vector<uint32_t> map_without_swaps(Network const& network, device const& device)
{
	bill::solver solver;
	map_cnf_encoder encoder(network, device, solver);
	return encoder.run();
}

} // namespace detail
#pragma endregion

/*! \brief
 */
template<typename Network>
mapping_view<Network> sat_map(Network const& network, device const& device)
{
	mapping_view<Network> mapped_network(network, device);

	std::vector<uint32_t> mapping = detail::map_without_swaps(network, device);
	if (mapping.empty()) {
		return mapped_network;
	}
	mapped_network.set_virtual_phy_map(mapping);
	network.foreach_gate([&](auto const& node) {
		if (node.gate.is_single_qubit()) {
			mapped_network.add_gate(node.gate, node.gate.target());
			return;
		}
		mapped_network.add_gate(node.gate, node.gate.control(), node.gate.target());
	});

	return mapped_network;
}

/*! \brief
 */
template<typename Network>
std::vector<uint32_t> sat_initial_map(Network const& network, device const& device)
{
	bill::solver solver;
	detail::map_cnf_encoder encoder(network, device, solver);
	// return encoder.run_incrementally();
	return encoder.run_greedly();
}

} // namespace tweedledum
