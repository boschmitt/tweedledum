/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../../IR/Circuit.h"
#include "../../../IR/Qubit.h"
#include "../../../Target/Device.h"
#include "../../../Target/Placement.h"

#include <bill/sat/cardinality.hpp>
#include <bill/sat/solver.hpp>
#include <optional>

namespace tweedledum {

template<typename Solver>
class SatPlacer {
    using LBool = bill::lbool_type;
    using Lit = bill::lit_type;
    using Var = bill::var_type;

public:
    SatPlacer(Device const& device, Circuit const& original, Solver& solver)
        : device_(device)
        , original_(original)
        , solver_(solver)
        , pairs_(original.num_qubits() * (original.num_qubits() + 1) / 2, 0u)
    {}

    std::optional<Placement> run()
    {
        solver_.add_variables(num_v() * num_phy());
        qubits_constraints();
        original_.foreach_instruction([&](Instruction const& inst) {
            if (inst.num_qubits() != 2u) {
                return;
            }
            Qubit const control = inst.control();
            Qubit const target = inst.target();
            if (pairs_[triangle_to_vector_idx(control, target)] == 0) {
                gate_constraints(control, target);
            }
            ++pairs_[triangle_to_vector_idx(control, target)];
        });
        solver_.solve();
        bill::result result = solver_.get_result();
        if (result.is_satisfiable()) {
            return decode(result.model());
        }
        return std::nullopt;
    }

private:
    Placement decode(std::vector<LBool> const& model)
    {
        Placement placement(num_phy(), num_v());
        for (uint32_t v_qid = 0; v_qid < num_v(); ++v_qid) {
            for (uint32_t phy_qid = 0; phy_qid < num_phy(); ++phy_qid) {
                Var var = v_to_phy_var(v_qid, phy_qid);
                if (model.at(var) == LBool::true_) {
                    Qubit const v = Qubit(v_qid);
                    Qubit const phy = Qubit(phy_qid);
                    placement.map_v_phy(v, phy);
                    break;
                }
            }
        }
        return placement;
    }

    uint32_t num_phy() const
    {
        return device_.num_qubits();
    }

    uint32_t num_v() const
    {
        return original_.num_qubits();
    }

    void qubits_constraints()
    {
        // Condition 2:
        std::vector<Var> variables;
        for (uint32_t v = 0u; v < num_v(); ++v) {
            for (uint32_t p = 0u; p < num_phy(); ++p) {
                variables.emplace_back(v_to_phy_var(v, p));
            }
            at_least_one(variables, solver_);
            at_most_one_pairwise(variables, solver_);
            variables.clear();
        }

        // Condition 3:
        for (uint32_t p = 0u; p < num_phy(); ++p) {
            for (uint32_t v = 0u; v < num_v(); ++v) {
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
    void gate_constraints(uint32_t c_v, uint32_t t_v)
    {
        std::vector<Lit> clause;
        for (uint32_t t_phy = 0; t_phy < num_phy(); ++t_phy) {
            uint32_t t_v_phy_var = v_to_phy_var(t_v, t_phy);
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
    }

    Var v_to_phy_var(uint32_t v_id, uint32_t phy_id)
    {
        return v_id * num_phy() + phy_id;
    }

    uint32_t triangle_to_vector_idx(uint32_t i, uint32_t j)
    {
        if (i > j) {
            std::swap(i, j);
        }
        return i * num_v() - (i - 1) * i / 2 + j - i;
    }

    Device const& device_;
    Circuit const& original_;
    Solver& solver_;
    std::vector<uint32_t> pairs_;
};

/*! \brief Yet to be written.
 */
std::optional<Placement> sat_place(Device const& device,
    Circuit const& original);

} // namespace tweedledum
