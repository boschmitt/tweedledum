/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../IR/Circuit.h"
#include "../Operators/Standard/Swap.h"
#include "../Target/Device.h"

#include <bill/sat/cardinality.hpp>
#include <bill/sat/solver.hpp>
#include <nlohmann/json.hpp>
#include <vector>

namespace tweedledum {

namespace {

template<typename Cnf>
class SatSwap {
    using map_type = std::vector<uint32_t>;
    using Swap = std::pair<uint32_t, uint32_t>;

    using LBool = bill::lbool_type;
    using Lit = bill::lit_type;
    using Var = bill::var_type;

public:
    SatSwap(Device const& graph, std::vector<uint32_t> const& init_cfg,
      std::vector<uint32_t> const& final_cfg, Cnf& cnf_builder)
        : device_(graph)
        , init_cfg_(init_cfg)
        , init_t2v_(init_cfg.size(), 0)
        , final_cfg_(final_cfg)
        , is_activation_() // TODO: better naming!
        , num_moments_(0)
        , offset_(num_vertices() * num_vertices() + num_edges())
        , cnf_builder_(cnf_builder)
        , vertice_edges_map_(num_vertices())
    {
        for (uint32_t i = 0; i < init_cfg_.size(); ++i) {
            init_t2v_[init_cfg[i]] = i;
        }
        for (uint32_t i = 0; i < num_edges(); ++i) {
            auto& [u, v] = device_.edge(i);
            vertice_edges_map_[u].emplace_back(i);
            vertice_edges_map_[v].emplace_back(i);
        }
        opt_num_swaps_ = true;
        pre_process();
    }

    void encode()
    {
        initial_moment();
        // Assume initial configuration
        for (uint32_t i = 0; i < init_cfg_.size(); ++i) {
            Lit lit(
              token_vertice_var(0, init_cfg_[i], i), bill::positive_polarity);
            cnf_builder_.add_clause(lit);
        }
        for (uint32_t i = 0; i < min_num_moments_; ++i) {
            add_moment();
        }
    }

    std::vector<Lit> encode_assumptions()
    {
        std::vector<Lit> assumptions;
        // Assume intial configuration
        // for (uint32_t i = 0; i < init_cfg_.size(); ++i) {
        // 	sat::Var var = token_vertice_var(0, init_cfg_[i], i);
        // 	assumptions.emplace_back(var, sat::positive_polarity);
        // }
        // Assume final configuration
        for (uint32_t i = 0; i < final_cfg_.size(); ++i) {
            Var var = token_vertice_var(num_moments_ - 1, final_cfg_[i], i);
            assumptions.emplace_back(var, bill::positive_polarity);
        }
        return assumptions;
    }

    void encode_new_moment()
    {
        add_moment();
        if (opt_num_swaps_) {
            add_moment();
        }
    }

    std::vector<Swap> decode(std::vector<LBool> const& model)
    {
        std::vector<Swap> swaps;
        for (uint32_t moment = 0; moment < num_moments_ - 1; ++moment) {
            for (uint32_t edge = 0; edge < num_edges(); ++edge) {
                Var var = swap_var(moment, edge);
                if (model.at(var) == LBool::true_) {
                    swaps.push_back(device_.edge(edge));
                }
            }
        }
        return swaps;
    }

private:
    uint32_t compute_inv(std::vector<uint32_t> const& permutation)
    {
        uint32_t inv = 0;
        for (uint32_t i = 0; i < permutation.size() - 1; ++i) {
            for (uint32_t j = i + 1; j < permutation.size(); ++j) {
                if (permutation.at(i) > permutation.at(j)) {
                    inv = inv + 1;
                }
            }
        }
        return inv;
    }

    void pre_process()
    {
        uint32_t max_distance = 0;
        uint32_t sum_distance = 0;
        for (uint32_t k = 0; k < init_cfg_.size(); ++k) {
            if (init_cfg_[k] != final_cfg_[k]) {
                auto it =
                  std::find(final_cfg_.begin(), final_cfg_.end(), init_cfg_[k]);
                uint32_t idx = std::distance(final_cfg_.begin(), it);
                uint32_t dist = device_.distance(k, idx);
                sum_distance += dist;
                max_distance = std::max(max_distance, dist);
            }
        }
        // When optimizing for swap number, each moment hold only one SWAP
        if (opt_num_swaps_) {
            // sgn = false (odd), true (even)
            bool sgn_init = compute_inv(init_cfg_) & 1;
            bool sgn_final = compute_inv(final_cfg_) & 1;
            min_num_moments_ = std::ceil(sum_distance / 2.0);
            // If the minimum number of moments is odd, but the sgn of
            // the permutations is the same, we add one, as we know that the
            // solution must have an even number of swaps.
            //
            // If the minimum number of moments is even, but the sgn of the
            // permutations are different, then we also need to add one.
            if (min_num_moments_ & 1) {
                if (sgn_init == sgn_final) {
                    ++min_num_moments_;
                }
            } else {
                if (sgn_init != sgn_final) {
                    ++min_num_moments_;
                }
            }
        } else {
            min_num_moments_ = max_distance;
        }
    }

    uint32_t num_edges() const
    {
        return device_.num_edges();
    }

    uint32_t num_vertices() const
    {
        return device_.num_qubits();
    }

    Var token_vertice_var(uint32_t moment, uint32_t token, uint32_t vertice)
    {
        return moment * offset_ + token * num_vertices() + vertice;
    }

    Var swap_var(uint32_t moment, uint32_t edge)
    {
        return moment * offset_ + num_vertices() * num_vertices() + edge;
    }

    void create_token_vertice_variables()
    {
        // Create token <-> vertice variables
        // Make sure that each token is assign to only one vertice
        std::vector<Var> variables;
        for (uint32_t token = 0; token < num_vertices(); ++token) {
            for (uint32_t vertice = 0; vertice < num_vertices(); ++vertice) {
                if (device_.distance(vertice, init_t2v_[token])
                    <= (num_moments_ + 1)) {
                    variables.emplace_back(cnf_builder_.add_variable());
                    is_activation_.emplace_back(0);
                    continue;
                }
                cnf_builder_.add_clause(
                  Lit(cnf_builder_.add_variable(), bill::negative_polarity));
                is_activation_.emplace_back(1);
            }
            bill::at_least_one(variables, cnf_builder_);
            bill::at_most_one_pairwise(variables, cnf_builder_);
            variables.clear();
        }

        // Make sure that each vertice is assign at only one token
        for (uint32_t vertice = 0; vertice < num_vertices(); ++vertice) {
            for (uint32_t token = 0; token < num_vertices(); ++token) {
                if (device_.distance(vertice, init_t2v_[token])
                    <= (num_moments_ + 1)) {
                    variables.emplace_back(
                      token_vertice_var(num_moments_, token, vertice));
                }
            }
            bill::at_least_one(variables, cnf_builder_);
            bill::at_most_one_pairwise(variables, cnf_builder_);
            variables.clear();
        }
    }

    void initial_moment()
    {
        create_token_vertice_variables();
        ++num_moments_;
    }

    void add_moment()
    {
        // Create swap variables
        std::vector<Var> variables;
        for (uint32_t i = 0; i < num_edges(); ++i) {
            variables.emplace_back(cnf_builder_.add_variable());
            is_activation_.emplace_back(0);
        }
        if (opt_num_swaps_) {
            at_most_one_pairwise(variables, cnf_builder_);
            if (num_moments_ > 1) {
                symmetry_break(num_moments_ - 2, num_moments_ - 1);
            }
        }
        variables.clear();

        // Create the token <-> vertice variables for the new moment
        create_token_vertice_variables();

        // Create the swaps contraints
        // *) Condition 1:
        assert(is_activation_.size() == cnf_builder_.num_variables());
        std::vector<Lit> clause;
        for (uint32_t vertice = 0; vertice < num_vertices(); ++vertice) {
            for (uint32_t token = 0; token < num_vertices(); ++token) {
                Var prev_var =
                  token_vertice_var(num_moments_ - 1, token, vertice);
                Var current_var =
                  token_vertice_var(num_moments_, token, vertice);
                if (is_activation_[current_var]) {
                    continue;
                }
                for (uint32_t edge : vertice_edges_map_[vertice]) {
                    Var edge_var = swap_var(num_moments_ - 1, edge);
                    clause.clear();
                    clause.emplace_back(current_var, bill::negative_polarity);
                    clause.emplace_back(prev_var, bill::negative_polarity);
                    clause.emplace_back(edge_var, bill::negative_polarity);
                    cnf_builder_.add_clause(clause);
                }
            }
        }
        // *) Condition 2:
        for (uint32_t vertice = 0; vertice < device_.num_qubits(); ++vertice) {
            for (uint32_t token = 0; token < device_.num_qubits(); ++token) {
                Var prev_var =
                  token_vertice_var(num_moments_ - 1, token, vertice);
                Var current_var =
                  token_vertice_var(num_moments_, token, vertice);
                std::vector<Var> edge_vars;
                std::vector<Lit> edge_lits;
                std::vector<Lit> token_lits;
                for (uint32_t edge : vertice_edges_map_[vertice]) {
                    edge_vars.emplace_back(swap_var(num_moments_ - 1, edge));
                    edge_lits.emplace_back(
                      edge_vars.back(), bill::positive_polarity);
                    auto [u, v] = device_.edge(edge);
                    if (u != vertice) {
                        token_lits.emplace_back(
                          token_vertice_var(num_moments_ - 1, token, u),
                          bill::positive_polarity);
                    } else {
                        token_lits.emplace_back(
                          token_vertice_var(num_moments_ - 1, token, v),
                          bill::positive_polarity);
                    }
                }
                if (!opt_num_swaps_) {
                    at_most_one_pairwise(edge_vars, cnf_builder_);
                }
                if (is_activation_[current_var]) {
                    continue;
                }
                clause = edge_lits;
                clause.emplace_back(current_var, bill::negative_polarity);
                clause.emplace_back(prev_var, bill::positive_polarity);
                assert(edge_lits.size() == token_lits.size());
                for (uint32_t i = 0; i < (1u << edge_lits.size()); ++i) {
                    for (uint32_t j = i, k = 0; j; j >>= 1, ++k) {
                        if (j & 1) {
                            clause[k] = token_lits[k];
                        } else {
                            clause[k] = edge_lits[k];
                        }
                    }
                    cnf_builder_.add_clause(clause);
                }
            }
        }
        ++num_moments_;
    }

    // I don't think this is complete
    void symmetry_break(uint32_t prev_moment, uint32_t current_moment)
    {
        std::vector<Lit> clause;
        for (uint32_t i = 0; i < device_.num_edges(); ++i) {
            auto& [u_i, v_i] = device_.edge(i);
            for (uint32_t j = i + 1; j < device_.num_edges(); ++j) {
                auto& [u_j, v_j] = device_.edge(j);
                if (u_i == u_j || u_i == v_j || v_i == u_j || v_i == v_j) {
                    continue;
                }
                clause.emplace_back(
                  swap_var(prev_moment, j), bill::negative_polarity);
                clause.emplace_back(
                  swap_var(current_moment, j), bill::negative_polarity);
                cnf_builder_.add_clause(clause);
                clause.clear();
            }
        }
    }

    Device const& device_;
    std::vector<uint32_t> init_cfg_; // vertice -> token
    std::vector<uint32_t> init_t2v_; // vertice <- token
    std::vector<uint32_t> final_cfg_; // vertice -> token
    uint32_t min_num_moments_;

    // Encoded problem
    std::vector<uint8_t> is_activation_;
    uint32_t num_moments_;
    uint32_t offset_;
    bool opt_num_swaps_;
    Cnf& cnf_builder_;

    // Auxiliary
    // Maps which edeges as connected to a particular node.
    std::vector<std::vector<uint32_t>> vertice_edges_map_;
};

} // namespace

Circuit sat_swap_synth(Device const& device,
  std::vector<uint32_t> const& init_cfg, std::vector<uint32_t> const& final_cfg,
  nlohmann::json const& config = {})
{
    using Swap = std::pair<uint32_t, uint32_t>;
    std::vector<Swap> swaps;
    bill::solver solver;
    SatSwap encoder(device, init_cfg, final_cfg, solver);
    encoder.encode();
    do {
        std::vector<bill::lit_type> assumptions = encoder.encode_assumptions();
        solver.solve(assumptions);
        bill::result result = solver.get_result();
        if (result) {
            swaps = encoder.decode(result.model());
            break;
        }
        encoder.encode_new_moment();
    } while (1);

    Circuit circuit;
    for (uint32_t i = 0u; i < device.num_qubits(); ++i) {
        circuit.create_qubit();
    }
    for (auto [x, y] : swaps) {
        circuit.apply_operator(Op::Swap(), {Qubit(x), Qubit(y)});
    }
    return circuit;
}

} // namespace tweedledum