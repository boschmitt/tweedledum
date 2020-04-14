/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../../networks/wire.hpp"
#include "../../../utils/device.hpp"

#include <bill/sat/cardinality.hpp>
#include <bill/sat/solver.hpp>
#include <vector>

namespace tweedledum::detail {

template<typename Network, typename Solver>
class hplace_cnf_encoder {
	using lbool_type = bill::lbool_type;
	using lit_type = bill::lit_type;
	using var_type = bill::var_type;
	using op_type = typename Network::op_type;

public:
	hplace_cnf_encoder(Network const& network, device const& device, Solver& solver)
	    : network_(network)
	    , device_(device)
	    , pairs_(network_.num_qubits() * (network_.num_qubits() + 1) / 2, 0u)
	    , solver_(solver)
	    , wire_to_v_(network.num_wires(), wire::invalid_id)
	{
		network.foreach_wire([&](wire::id id) {
			if (id.is_qubit()) {
				wire_to_v_.at(id) = wire::make_qubit(v_to_wire_.size());
				v_to_wire_.push_back(id);
			}
		});
	}

	std::vector<wire::id> run()
	{
		solver_.add_variables(num_v() * num_phy());
		qubits_constraints();
		network_.foreach_op([&](op_type const& op) {
			if (!op.is_two_qubit()) {
				return;
			}
			wire::id control = wire_to_v_.at(op.control());
			wire::id target = wire_to_v_.at(op.target());
			++pairs_[triangle_to_vector_idx(control, target)];
		});

		std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> aa;
		for (uint32_t i = 0; i < num_v(); ++i) {
			for (uint32_t j = 0; j < num_v(); ++j) {
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
		for (auto&& [control, target, score] : aa) {
			var_type act = gate_constraints(control, target);
			assumptions.emplace_back(act, bill::positive_polarity);
			solver_.solve(assumptions);
			bill::result result = solver_.get_result();
			if (result) {
				model_ = result.model();
			} else {
				assumptions.back().complement();
			}
		}
		return decode(model_);
	}

	std::vector<wire::id> decode(std::vector<lbool_type> const& model)
	{
		std::vector<wire::id> mapping(network_.num_qubits(), wire::invalid_id);
		for (uint32_t v_qid = 0; v_qid < num_v(); ++v_qid) {
			for (uint32_t phy_qid = 0; phy_qid < num_phy(); ++phy_qid) {
				var_type var = v_to_phy_var(v_qid, phy_qid);
				if (model.at(var) == lbool_type::true_) {
					mapping.at(v_qid) = wire::make_qubit(phy_qid);
					break;
				}
			}
		}
		for (uint32_t phy_qid = 0; phy_qid < num_phy(); ++phy_qid) {
			bool used = false;
			for (uint32_t v_qid = 0; v_qid < num_v(); ++v_qid) {
				var_type var = v_to_phy_var(v_qid, phy_qid);
				if (model.at(var) == lbool_type::true_) {
					used = true;
					break;
				}
			}
			if (!used) {
				mapping.emplace_back(phy_qid, true);
			}
		}
		return mapping;
	}

private:
	uint32_t num_phy() const
	{
		return device_.num_qubits();
	}

	uint32_t num_v() const
	{
		return network_.num_qubits();
	}

	void qubits_constraints()
	{
		// Condition 2:
		std::vector<bill::var_type> variables;
		for (auto v = 0u; v < num_v(); ++v) {
			for (auto p = 0u; p < num_phy(); ++p) {
				variables.emplace_back(v_to_phy_var(v, p));
			}
			at_least_one(variables, solver_);
			at_most_one_pairwise(variables, solver_);
			variables.clear();
		}

		// Condition 3:
		for (auto p = 0u; p < num_phy(); ++p) {
			for (auto v = 0u; v < num_v(); ++v) {
				variables.emplace_back(v_to_phy_var(v, p));
			}
			at_most_one_pairwise(variables, solver_);
			variables.clear();
		}
	}

	// Abbreviations:
	// - c_v (control, virtual qubit identifier)
	// - t_v (target, virtual qubit identifier)
	// - c_phy (control, physical qubit identifier)
	// - t_phy (target, physical qubit identifier)
	var_type gate_constraints(uint32_t c_v, uint32_t t_v)
	{
		std::vector<bill::lit_type> clause;
		var_type act_var = solver_.add_variable();
		for (uint32_t t_phy = 0; t_phy < num_phy(); ++t_phy) {
			uint32_t t_v_phy_var = v_to_phy_var(t_v, t_phy);
			clause.emplace_back(act_var, bill::negative_polarity);
			clause.emplace_back(t_v_phy_var, bill::negative_polarity);
			for (uint32_t c_phy = 0; c_phy < num_phy(); ++c_phy) {
				if (c_phy == t_phy || !device_.are_connected(c_phy, t_phy)) {
					continue;
				}
				uint32_t c_v_phy_var = v_to_phy_var(c_v, c_phy);
				clause.emplace_back(c_v_phy_var, bill::positive_polarity);
			}
			solver_.add_clause(clause);
			clause.clear();
		}
		return act_var;
	}

private:
	bill::var_type v_to_phy_var(uint32_t virtual_id, uint32_t physical_id)
	{
		return virtual_id * num_phy() + physical_id;
	}

	uint32_t triangle_to_vector_idx(uint32_t i, uint32_t j)
	{
		if (i > j) {
			std::swap(i, j);
		}
		return i * num_v() - (i - 1) * i / 2 + j - i;
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
	std::vector<wire::id> wire_to_v_;
	std::vector<wire::id> v_to_wire_;
};

/*! \brief Yet to be written.
 */
template<typename Network>
std::vector<wire::id> hsat_placement(Network const& network, device const& device)
{
	bill::solver solver;
	detail::hplace_cnf_encoder encoder(network, device, solver);
	return encoder.run();
}

} // namespace tweedledum::detail
