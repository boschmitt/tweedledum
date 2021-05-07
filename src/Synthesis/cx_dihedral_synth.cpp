/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Synthesis/cx_dihedral_synth.h"
#include "tweedledum/Operators/Standard.h"

#include <bill/sat/cardinality.hpp>
#include <bill/sat/solver.hpp>
#include <vector>

namespace tweedledum {

namespace {

// TODO:
//  - Investigate lower bound!, this woudl allow me to encode a bunch o moments
//  in first go
//  -
template<typename Solver>
class CXDihedralEncoder {
    using LBool = bill::lbool_type;
    using Lit = bill::lit_type;
    using Var = bill::var_type;

public:
    CXDihedralEncoder(BMatrix const& linear_trans,
      LinPhasePoly const& phase_parities, Solver& solver)
        : transform_(linear_trans)
        , phase_parities_(phase_parities)
        , use_symmetry_break_(true)
        , num_terms_(phase_parities.size())
        , num_moments_(0)
        , offset_((num_qubits() * num_qubits()) + num_terms()
                  + (num_terms() * num_qubits()) + (2 * num_qubits()))
        , solver_(solver)
    {}

    void encode()
    {
        // Create matrix variables
        solver_.add_variables(transform_.rows() * transform_.cols());
        // Encode initial states
        for (uint32_t row = 0u; row < transform_.rows(); ++row) {
            for (uint32_t column = 0u; column < transform_.cols(); ++column) {
                const auto polarity = (row == column) ? bill::positive_polarity
                                                      : bill::negative_polarity;
                Lit lit(matrix_var(0, row, column), polarity);
                solver_.add_clause(lit);
            }
        }
        if (use_symmetry_break_) {
            symmetry_break_matrix(num_moments_);
        }
        encode_parity_terms(num_moments_);
        ++num_moments_;
    }

    std::vector<Lit> encode_assumptions() const
    {
        std::vector<Lit> assumptions;
        for (uint32_t row = 0u; row < num_qubits(); ++row) {
            for (uint32_t column = 0u; column < num_qubits(); ++column) {
                const auto polarity = transform_(row, column)
                                      ? bill::positive_polarity
                                      : bill::negative_polarity;
                assumptions.emplace_back(
                  matrix_var(num_moments_ - 1, row, column), polarity);
            }
        }
        for (uint32_t term_id = 0u; term_id < num_terms(); ++term_id) {
            assumptions.emplace_back(parity_term_var(num_moments_ - 1, term_id),
              bill::positive_polarity);
        }
        return assumptions;
    }

    void encode_new_moment()
    {
        encode_cnot_gates(num_moments_ - 1);
        assert((offset_ * num_moments_) == solver_.num_variables());

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

    void decode(Circuit& circuit, std::vector<Qubit> const& qubits,
      std::vector<LBool> const& model)
    {
        std::vector<uint32_t> qubits_states;
        for (uint32_t i = 0u; i < num_qubits(); ++i) {
            qubits_states.emplace_back(1u << i);
            double rotation = phase_parities_.extract_phase(qubits_states[i]);
            if (rotation != 0.0) {
                circuit.apply_operator(Op::P(rotation), {qubits.at(i)});
            }
        }
        for (uint32_t moment = 0u; moment < (num_moments_ - 1); ++moment) {
            uint32_t control = 0u;
            uint32_t target = 0u;
            for (uint32_t row = 0u; row < num_qubits(); ++row) {
                if (model.at(control_var(moment, row)) == LBool::true_) {
                    control = row;
                    assert(model.at(target_var(moment, row)) == LBool::false_);
                } else if (model.at(target_var(moment, row)) == LBool::true_) {
                    target = row;
                    assert(model.at(control_var(moment, row)) == LBool::false_);
                }
            }
            circuit.apply_operator(
              Op::X(), {qubits.at(control), qubits.at(target)});
            qubits_states[target] ^= qubits_states[control];
            double rotation =
              phase_parities_.extract_phase(qubits_states[target]);
            if (rotation != 0.0) {
                circuit.apply_operator(Op::P(rotation), {qubits.at(target)});
            }
        }
    }

private:
    uint32_t num_qubits() const
    {
        return transform_.rows();
    }

    uint32_t num_terms() const
    {
        return num_terms_;
    }

    void encode_parity_terms(uint32_t moment)
    {
        std::vector<Lit> matrix_lits;
        std::vector<Lit> term_lits;

        // Create parity terms variables and term->row
        solver_.add_variables(num_terms() + (num_terms() * num_qubits()));
        uint32_t term_id = 0u;
        for (auto const& [esop, _] : phase_parities_) {
            uint32_t term = 0;
            // TODO: fix this hack!!
            for (uint32_t lit : esop) {
                term |= (1 << ((lit >> 1) - 1));
            }
            (void) _;
            for (uint32_t row = 0u; row < num_qubits(); ++row) {
                for (uint32_t column = 0u; column < num_qubits(); ++column) {
                    const auto polarity = (term >> column) & 1
                                          ? bill::positive_polarity
                                          : bill::negative_polarity;
                    matrix_lits.emplace_back(
                      matrix_var(moment, row, column), polarity);
                }
                encode_and(
                  matrix_lits, parity_term_row_var(moment, term_id, row));
                term_lits.emplace_back(
                  parity_term_row_var(moment, term_id, row),
                  bill::positive_polarity);
                matrix_lits.clear();
            }
            if (moment > 0) {
                std::vector<Lit> clause;
                term_lits.emplace_back(parity_term_var(moment - 1, term_id),
                  bill::positive_polarity);
                clause.emplace_back(parity_term_var(moment - 1, term_id),
                  bill::negative_polarity);
                clause.emplace_back(
                  parity_term_var(moment, term_id), bill::positive_polarity);
                solver_.add_clause(clause);
            }
            encode_or(term_lits, parity_term_var(moment, term_id));
            ++term_id;
            term_lits.clear();
        }
    }

    void encode_cnot_gates(uint32_t moment)
    {
        std::vector<Var> control;
        std::vector<Var> target;

        // Create control and target variables
        solver_.add_variables(2 * num_qubits());

        for (uint32_t row = 0u; row < num_qubits(); ++row) {
            control.emplace_back(control_var(moment, row));
            target.emplace_back(target_var(moment, row));
        }
        // fmt::print("Control vars: {}\n", fmt::join(control.begin(),
        // control.end(), ",")); fmt::print("Target vars: {}\n",
        // fmt::join(target.begin(), target.end(), ",")); fmt::print("Number of
        // vars: {}\n", solver_.num_variables());
        bill::at_least_one(control, solver_);
        bill::at_most_one_pairwise(control, solver_);
        bill::at_least_one(target, solver_);
        bill::at_most_one_pairwise(target, solver_);

        std::vector<Lit> clause;
        for (uint32_t row = 0u; row < num_qubits(); ++row) {
            clause.emplace_back(
              control_var(moment, row), bill::negative_polarity);
            clause.emplace_back(
              target_var(moment, row), bill::negative_polarity);
            solver_.add_clause(clause);
            clause.clear();
        }
    }

    void encode_transition(uint32_t moment)
    {
        // Create matrix variables for new moment
        solver_.add_variables(transform_.rows() * transform_.cols());

        std::vector<Lit> clause;
        for (uint32_t row = 0u; row < num_qubits(); ++row) {
            for (uint32_t column = 0u; column < num_qubits(); ++column) {
                clause.emplace_back(
                  target_var(moment - 1, row), bill::positive_polarity);
                clause.emplace_back(
                  matrix_var(moment - 1, row, column), bill::negative_polarity);
                clause.emplace_back(
                  matrix_var(moment, row, column), bill::positive_polarity);
                solver_.add_clause(clause);

                clause.at(1) = Lit(
                  matrix_var(moment - 1, row, column), bill::positive_polarity);
                clause.at(2) =
                  Lit(matrix_var(moment, row, column), bill::negative_polarity);
                solver_.add_clause(clause);

                encode_more_dependencies(moment, row, column);
                clause.clear();
            }
        }
    }

    void encode_more_dependencies(
      uint32_t moment, uint32_t row, uint32_t column)
    {
        for (uint32_t other_row = 0u; other_row < num_qubits(); ++other_row) {
            if (other_row == row) {
                continue;
            }
            std::vector<Lit> clause;
            clause.emplace_back(
              target_var(moment - 1, row), bill::negative_polarity);
            clause.emplace_back(
              control_var(moment - 1, other_row), bill::negative_polarity);

            clause.emplace_back(
              matrix_var(moment - 1, row, column), bill::negative_polarity);
            clause.emplace_back(matrix_var(moment - 1, other_row, column),
              bill::negative_polarity);
            clause.emplace_back(
              matrix_var(moment, row, column), bill::negative_polarity);
            solver_.add_clause(clause);

            clause.at(3).complement();
            clause.at(4).complement();
            solver_.add_clause(clause);

            clause.at(2).complement();
            clause.at(3).complement();
            solver_.add_clause(clause);

            clause.at(3).complement();
            clause.at(4).complement();
            solver_.add_clause(clause);
        }
    }

    void symmetry_break_matrix(uint32_t moment)
    {
        std::vector<Lit> clause;
        /* there cannot be a row with all zeroes */
        for (uint32_t row = 0u; row < num_qubits(); ++row) {
            for (uint32_t column = 0u; column < num_qubits(); ++column) {
                clause.emplace_back(
                  matrix_var(moment, row, column), bill::positive_polarity);
            }
            solver_.add_clause(clause);
        }
        clause.clear();

        /* there cannot be a column with all zeroes */
        for (uint32_t column = 0u; column < num_qubits(); ++column) {
            for (uint32_t row = 0u; row < num_qubits(); ++row) {
                clause.emplace_back(
                  matrix_var(moment, row, column), bill::positive_polarity);
            }
            solver_.add_clause(clause);
        }
    }

    void symmetry_break_transition(uint32_t moment)
    {
        std::vector<Lit> clause;

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
                    clause.emplace_back(control_var(moment - 1, control0),
                      bill::negative_polarity);
                    clause.emplace_back(
                      control_var(moment, control0), bill::negative_polarity);
                    clause.emplace_back(
                      target_var(moment - 1, target0), bill::negative_polarity);
                    clause.emplace_back(
                      target_var(moment, target1), bill::negative_polarity);
                    solver_.add_clause(clause);
                    clause.clear();
                }
            }
        }
    }

    // Maybe add these methods in bill
    void encode_and(std::vector<Lit> const& lits, Var output)
    {
        std::vector<Lit> clause_0;
        std::vector<Lit> clause_1(2, Lit(output, bill::negative_polarity));
        for (Lit lit : lits) {
            clause_1.at(0) = lit;
            solver_.add_clause(clause_1);
            clause_0.push_back(~lit);
        }
        clause_0.emplace_back(output, bill::positive_polarity);
        solver_.add_clause(clause_0);
    }

    void encode_or(std::vector<Lit> const& lits, Var output)
    {
        std::vector<Lit> clause_0;
        std::vector<Lit> clause_1(2, Lit(output, bill::positive_polarity));
        for (Lit lit : lits) {
            clause_1.at(0) = ~lit;
            solver_.add_clause(clause_1);
            clause_0.emplace_back(lit);
        }
        clause_0.emplace_back(output, bill::negative_polarity);
        solver_.add_clause(clause_0);
    }

    Var matrix_var(uint32_t moment, uint32_t row, uint32_t column) const
    {
        Var var = (moment * offset_) + (row * num_qubits()) + column;
        assert(var < Var(solver_.num_variables()));
        return var;
    }

    Var parity_term_var(uint32_t moment, uint32_t id) const
    {
        Var var = (moment * offset_) + (num_qubits() * num_qubits()) + id;
        assert(var < Var(solver_.num_variables()));
        return var;
    }

    Var parity_term_row_var(uint32_t moment, uint32_t id, uint32_t row) const
    {
        Var var = (moment * offset_) + (num_qubits() * num_qubits())
                + num_terms() + (id * num_qubits()) + row;
        assert(var < Var(solver_.num_variables()));
        return var;
    }

    Var control_var(uint32_t moment, uint32_t row) const
    {
        Var var = moment * offset_ + (num_qubits() * num_qubits()) + num_terms()
                + (num_terms() * num_qubits()) + row;
        assert(var < Var(solver_.num_variables()));
        return var;
    }

    Var target_var(uint32_t moment, uint32_t row) const
    {
        Var var = moment * offset_ + (num_qubits() * num_qubits()) + num_terms()
                + (num_terms() * num_qubits()) + num_qubits() + row;
        assert(var < Var(solver_.num_variables()));
        return var;
    }

    BMatrix const& transform_;
    LinPhasePoly phase_parities_;

    // Parameters
    bool use_symmetry_break_;

    // Encoded problem
    uint32_t num_terms_;
    uint32_t num_moments_;
    uint32_t offset_;
    Solver& solver_;
};

} // namespace

void cx_dihedral_synth(Circuit& circuit, std::vector<Qubit> const& qubits,
  std::vector<Cbit> const& cbits, BMatrix const& linear_trans,
  LinPhasePoly phase_parities, nlohmann::json const& config)
{
    assert(qubits.size() == linear_trans.rows());
    assert(linear_trans.rows() <= 32);

    bill::solver solver;
    CXDihedralEncoder encoder(linear_trans, phase_parities, solver);
    encoder.encode();
    do {
        std::vector<bill::lit_type> assumptions = encoder.encode_assumptions();
        solver.solve(assumptions);
        bill::result result = solver.get_result();
        if (result) {
            encoder.decode(circuit, qubits, result.model());
            return;
        }
        encoder.encode_new_moment();
    } while (1);
}

Circuit cx_dihedral_synth(BMatrix const& linear_trans,
  LinPhasePoly const& phase_parities, nlohmann::json const& config)
{
    Circuit circuit;
    std::vector<Qubit> qubits;
    qubits.reserve(linear_trans.rows());
    for (uint32_t i = 0u; i < linear_trans.rows(); ++i) {
        qubits.emplace_back(circuit.create_qubit());
    }
    cx_dihedral_synth(
      circuit, qubits, {}, linear_trans, phase_parities, config);
    return circuit;
}

} // namespace tweedledum
