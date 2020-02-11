/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../gates/gate_base.hpp"
#include "../../gates/gate_lib.hpp"
#include "../../networks/io_id.hpp"
#include "../../utils/parity_terms.hpp"

#include <bill/sat/cardinality.hpp>
#include <bill/sat/solver.hpp>
#include <cassert>
#include <vector>

namespace tweedledum {

/*! \brief Parameters for `cnot_rz`. */
struct cnot_rz_params {
	bool use_symmetry_break = true;
};

namespace detail {

// TODO:
//  - Investigate lower bound!, this woudl allow me to encode a bunch o moments in first go
//  -
template<typename Matrix, typename Cnf>
class cnot_rz_encoder {
	using lbool_type = bill::lbool_type;
	using lit_type = bill::lit_type;
	using var_type = bill::var_type;

public:
	cnot_rz_encoder(Matrix const& transform, parity_terms<uint32_t> const& parities, Cnf& cnf_builder, cnot_rz_params params)
	: transform_(transform)
	, parities_(parities)
	, use_symmetry_break_(params.use_symmetry_break)
	, num_terms_(parities.num_terms())
	, num_moments_(0)
	, offset_((num_qubits() * num_qubits()) + num_terms() + (num_terms() * num_qubits()) + (2 * num_qubits()))
	, cnf_builder_(cnf_builder)
	{ 
		assert(transform_.is_square());
	}

	void encode()
	{
		// Create matrix variables
		cnf_builder_.add_variables(transform_.num_rows() * transform_.num_columns());
		// Encode intial states
		for (uint32_t row = 0u; row < transform_.num_rows(); ++row) {
			for (uint32_t column = 0u; column < transform_.num_columns(); ++column) {
				const auto polarity = (row == column) ? bill::positive_polarity : bill::negative_polarity;
				lit_type lit(matrix_var(0, row, column), polarity);
				cnf_builder_.add_clause(lit);
			}
		}
		if (use_symmetry_break_) {
			symmetry_break_matrix(num_moments_);
		}
		encode_parity_terms(num_moments_);
		++num_moments_;
	}

	std::vector<lit_type> encode_assumptions() const
	{
		std::vector<lit_type> assumptions;
		for (uint32_t row = 0u; row < num_qubits(); ++row ) {
			for (uint32_t column = 0u; column < num_qubits(); ++column) {
				const auto polarity = transform_.at(row, column) ? bill::positive_polarity : bill::negative_polarity;
				assumptions.emplace_back(matrix_var(num_moments_ - 1, row, column), polarity);
			}
		}
		for (uint32_t term_id = 0u; term_id < num_terms(); ++term_id) {
			assumptions.emplace_back(parity_term_var(num_moments_ - 1, term_id), bill::positive_polarity);
		}
		return assumptions;
	}

	void encode_new_moment()
	{
		encode_cnot_gates(num_moments_ - 1);
		assert((offset_ * num_moments_) == cnf_builder_.num_variables());

		encode_transition(num_moments_);
		if (use_symmetry_break_) {
			symmetry_break_matrix(num_moments_);
			if (num_moments_ >= 2) {
				symmetry_break_transition(num_moments_ - 1);
			}
		}
		encode_parity_terms(num_moments_);
		++num_moments_;
	}
	
	template<class Network>
	void decode(Network& network, std::vector<io_id> const& qubits, std::vector<lbool_type> const& model)
	{
		std::vector<uint32_t> qubits_states;
		for (uint32_t i = 0u; i < num_qubits(); ++i) {
			qubits_states.emplace_back(1u << i);
			angle rotation = parities_.extract_term(qubits_states[i]);
			if (rotation != 0.0) {
				network.add_gate(gate_base(gate_lib::rz, rotation), qubits.at(i));
			}
		}
		for (uint32_t moment = 0u; moment < (num_moments_ - 1); ++moment) {
			uint32_t control = 0u;
			uint32_t target = 0u;
			for (uint32_t row = 0u; row < num_qubits(); ++row) {
				if (model.at(control_var(moment, row)) == lbool_type::true_) {
					control = row;
					assert(model.at(target_var(moment, row)) == lbool_type::false_);
				} else if (model.at(target_var(moment, row)) == lbool_type::true_) {
					target = row;
					assert(model.at(control_var(moment, row)) == lbool_type::false_);
				}
			}
			network.add_gate(gate::cx, qubits.at(control), qubits.at(target));
			qubits_states[target] ^= qubits_states[control];
			angle rotation = parities_.extract_term(qubits_states[target]);
			if (rotation != 0.0) {
				network.add_gate(gate_base(gate_lib::rz, rotation), qubits.at(target));
			}
		}
	}

private:
	uint32_t num_qubits() const
	{
		return transform_.num_rows();
	}

	uint32_t num_terms() const
	{
		return num_terms_;
	}

private:
	void encode_parity_terms(uint32_t moment)
	{
		std::vector<lit_type> matrix_lits;
		std::vector<lit_type> term_lits;

		// Create parity terms variables and term->row
		cnf_builder_.add_variables(num_terms() + (num_terms() * num_qubits()));
		uint32_t term_id = 0u;
		for (auto const& [term, _] : parities_) {
			(void) _;
			for (uint32_t row = 0u; row < num_qubits(); ++row) {
				for (uint32_t column = 0u; column < num_qubits(); ++column) {
					const auto polarity = (term >> column) & 1 ? bill::positive_polarity : bill::negative_polarity;
					matrix_lits.emplace_back(matrix_var(moment, row, column), polarity);
				}
				encode_and(matrix_lits, parity_term_row_var(moment, term_id, row));
				term_lits.emplace_back(parity_term_row_var(moment, term_id, row), bill::positive_polarity);
				matrix_lits.clear();
			}
			if (moment > 0) {
				std::vector<lit_type> clause;
				term_lits.emplace_back(parity_term_var(moment - 1, term_id), bill::positive_polarity);
				clause.emplace_back(parity_term_var(moment - 1, term_id), bill::negative_polarity);
				clause.emplace_back(parity_term_var(moment, term_id), bill::positive_polarity);
				cnf_builder_.add_clause(clause);
			}
			encode_or(term_lits, parity_term_var(moment, term_id));
			++term_id;
			term_lits.clear();
		}
	}

	void encode_cnot_gates(uint32_t moment)
	{
		std::vector<var_type> control;
		std::vector<var_type> target;

		// Create control and target variables
		cnf_builder_.add_variables(2 * num_qubits());

		for (uint32_t row = 0u; row < num_qubits(); ++row) {
			control.emplace_back(control_var(moment, row));
			target.emplace_back(target_var(moment, row));
		}
		// fmt::print("Control vars: {}\n", fmt::join(control.begin(), control.end(), ","));
		// fmt::print("Target vars: {}\n", fmt::join(target.begin(), target.end(), ","));
		// fmt::print("Number of vars: {}\n", cnf_builder_.num_variables());
		bill::at_least_one(control, cnf_builder_);
		bill::at_most_one_pairwise(control, cnf_builder_);
		bill::at_least_one(target, cnf_builder_);
		bill::at_most_one_pairwise(target, cnf_builder_);

		std::vector<lit_type> clause;
		for (uint32_t row = 0u; row < num_qubits(); ++row) {
			clause.emplace_back(control_var(moment, row), bill::negative_polarity);
			clause.emplace_back(target_var(moment, row), bill::negative_polarity);
			cnf_builder_.add_clause(clause);
			clause.clear();
		}
	}

	void encode_transition(uint32_t moment)
	{
		// Create matrix variables for new moment
		cnf_builder_.add_variables(transform_.num_rows() * transform_.num_columns());

		std::vector<lit_type> clause;
		for (uint32_t row = 0u; row < num_qubits(); ++row) {
			for (uint32_t column = 0u; column < num_qubits(); ++column) {
				clause.emplace_back(target_var(moment - 1, row), bill::positive_polarity);
				clause.emplace_back(matrix_var(moment - 1, row, column), bill::negative_polarity);
				clause.emplace_back(matrix_var(moment, row, column), bill::positive_polarity);
				cnf_builder_.add_clause(clause);

				clause.at(1) = lit_type(matrix_var(moment - 1, row, column), bill::positive_polarity);
				clause.at(2) = lit_type(matrix_var(moment, row, column), bill::negative_polarity);
				cnf_builder_.add_clause(clause);

				encode_more_dependencies(moment, row, column);
				clause.clear();
			}
		}
	}

	void encode_more_dependencies(uint32_t moment, uint32_t row, uint32_t column)
	{
		for (uint32_t other_row = 0u; other_row < num_qubits(); ++other_row) {
			if (other_row == row) {
				continue;
			}
			std::vector<lit_type> clause;
			clause.emplace_back(target_var(moment - 1, row), bill::negative_polarity);
			clause.emplace_back(control_var(moment - 1, other_row), bill::negative_polarity);

			clause.emplace_back(matrix_var(moment - 1, row, column), bill::negative_polarity);
			clause.emplace_back(matrix_var(moment - 1, other_row, column), bill::negative_polarity);
			clause.emplace_back(matrix_var(moment, row, column), bill::negative_polarity);
			cnf_builder_.add_clause(clause);

			clause.at(3).complement();
			clause.at(4).complement();
			cnf_builder_.add_clause(clause);

			clause.at(2).complement();
			clause.at(3).complement();
			cnf_builder_.add_clause(clause);

			clause.at(3).complement();
			clause.at(4).complement();
			cnf_builder_.add_clause(clause);
		}
	}

	void symmetry_break_matrix(uint32_t moment)
	{
		std::vector<lit_type> clause;
		/* there cannot be a row with all zeroes */
		for (uint32_t row = 0u; row < num_qubits(); ++row) {
			for (uint32_t column = 0u; column < num_qubits(); ++column) {
				clause.emplace_back(matrix_var(moment, row, column), bill::positive_polarity);
			}
			cnf_builder_.add_clause(clause);
		}
		clause.clear();

		/* there cannot be a column with all zeroes */
		for (uint32_t column = 0u; column < num_qubits(); ++column) {
			for (uint32_t row = 0u; row < num_qubits(); ++row) {
				clause.emplace_back(matrix_var(moment, row, column), bill::positive_polarity);
			}
			cnf_builder_.add_clause(clause);
		}
	}

	void symmetry_break_transition(uint32_t moment)
	{
		std::vector<lit_type> clause;

		/* same control, first target must be smaller than others */
		for (uint32_t control0 = 0u; control0 < num_qubits(); ++control0) {
			for (uint32_t target0 = 1u; target0 < num_qubits(); ++target0) {
				if (control0 == target0) {
					continue;
				}
				for (uint32_t target1 = 0u; target1 < target0; ++target1) {
					if (control0 == target1) {
						continue;
					}
					clause.emplace_back(control_var(moment - 1, control0), bill::negative_polarity);
					clause.emplace_back(control_var(moment, control0), bill::negative_polarity);
					clause.emplace_back(target_var(moment - 1, target0), bill::negative_polarity);
					clause.emplace_back(target_var(moment, target1), bill::negative_polarity);
					cnf_builder_.add_clause(clause);
					clause.clear();
				}
			}
		}
	}

// Maybe add these methods in bill
private:
	void encode_and(std::vector<lit_type> const& lits, var_type output)
	{
		std::vector<lit_type> clause_0;
		std::vector<lit_type> clause_1(2, lit_type(output, bill::negative_polarity));
		for (lit_type lit : lits) {
			clause_1.at(0) = lit;
			cnf_builder_.add_clause(clause_1);
			clause_0.push_back(~lit);
		}
		clause_0.emplace_back(output, bill::positive_polarity);
		cnf_builder_.add_clause(clause_0);
	}

	void encode_or(std::vector<lit_type> const& lits, var_type output)
	{
		std::vector<lit_type> clause_0;
		std::vector<lit_type> clause_1(2, lit_type(output, bill::positive_polarity));
		for (lit_type lit : lits) {
			clause_1.at(0) = ~lit;
			cnf_builder_.add_clause(clause_1);
			clause_0.emplace_back(lit);
		}
		clause_0.emplace_back(output, bill::negative_polarity);
		cnf_builder_.add_clause(clause_0);
	}

private:
	var_type matrix_var(uint32_t moment, uint32_t row, uint32_t column) const
	{
		var_type var = (moment * offset_) + (row * num_qubits()) + column;
		assert(var < var_type(cnf_builder_.num_variables()));
		return var;
	}

	var_type parity_term_var(uint32_t moment, uint32_t id) const
	{
		var_type var = (moment * offset_) + (num_qubits() * num_qubits()) + id;
		assert(var < var_type(cnf_builder_.num_variables()));
		return var;
	}

	var_type parity_term_row_var(uint32_t moment, uint32_t id, uint32_t row) const
	{
		var_type var =  (moment * offset_) + (num_qubits() * num_qubits()) + num_terms() + (id * num_qubits()) + row;
		assert(var < var_type(cnf_builder_.num_variables()));
		return var;
	}

	var_type control_var(uint32_t moment, uint32_t row) const
	{
		var_type var =  moment * offset_ + (num_qubits() * num_qubits()) + num_terms() + (num_terms() * num_qubits()) + row;
		assert(var < var_type(cnf_builder_.num_variables()));
		return var;
	}

	var_type target_var(uint32_t moment, uint32_t row) const
	{
		var_type var =  moment * offset_ + (num_qubits() * num_qubits()) + num_terms() + (num_terms() * num_qubits()) + num_qubits() + row;
		assert(var < var_type(cnf_builder_.num_variables()));
		return var;
	}

private:
	Matrix const& transform_;
	parity_terms<uint32_t> parities_;

	// Parameters
	bool use_symmetry_break_;

	// Encoded problem
	uint32_t num_terms_;
	uint32_t num_moments_;
	uint32_t offset_;
	Cnf& cnf_builder_;
};

} // namespace detail

/*! \brief .
 */
template<class Network, class Matrix>
void cnot_rz(Network& network, std::vector<io_id> const& qubits,
             Matrix const& matrix, parity_terms<uint32_t> const& parities,
             cnot_rz_params params = {})
{
	assert(qubits.size() == matrix.num_rows());
	assert(matrix.num_rows() <= 32);

	bill::solver solver;
	detail::cnot_rz_encoder encoder(matrix, parities, solver, params);
	encoder.encode();
	do {
		std::vector<bill::lit_type> assumptions = encoder.encode_assumptions();
		solver.solve(assumptions);
		bill::result result = solver.get_result();
		if (result) {
			encoder.decode(network, qubits, result.model());
			return;
		}
		encoder.encode_new_moment();
	} while (1);
}

/*! \brief
 */
template<class Network, class Matrix>
Network cnot_rz(Matrix const& matrix, parity_terms<uint32_t> const& parities, cnot_rz_params params = {})
{
	assert(matrix.num_rows() <= 32);
	assert(matrix.is_square());
	Network network;
	for (auto i = 0u; i < matrix.num_rows(); ++i) {
		network.add_qubit();
	}
	cnot_rz(network, network.wiring_map(), matrix, parities, params);
	return network;
}

} // namespace tweedledum
