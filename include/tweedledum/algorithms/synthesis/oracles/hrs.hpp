/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../../gates/gate_base.hpp"
#include "../../../networks/io_id.hpp"
#include "../stg.hpp"

#include <fmt/format.h>
#include <mockturtle/traits.hpp>
#include <mockturtle/utils/node_map.hpp>
#include <mockturtle/networks/xag.hpp>
#include <stack>
#include <vector>

namespace tweedledum {

struct hrs_params {
	/*! \brief Be verbose. */
	bool verbose = false;
};

struct hrs_info {
	std::vector<io_id> inputs;
	std::vector<io_id> outputs;
};

namespace detail {

template<class QuantumNetwork>
class hrs_xag {
	using LogicNetwork = mockturtle::xag_network;
	using mt_node_type = typename LogicNetwork::node;
	using mt_signal_type = typename LogicNetwork::signal;

public:
	hrs_xag(QuantumNetwork& q_network, LogicNetwork const& c_network, hrs_info* info,
	        hrs_params const& params)
	    : q_network_(q_network)
	    , c_network_(c_network)
	    , params_(params)
	    , info_(info)
	    , node_to_qubit_(c_network, io_invalid)
	    , node_ltfi_(c_network)
	{}

	void synthesize()
	{
		c_network_.foreach_pi([&](auto const& node, auto index) { 
			node_to_qubit_[node] = q_network_.add_qubit(fmt::format("i_{}", index));
			qubit_usage_.emplace_back(0);
			is_ancilla_.emplace_back(0);
			if (info_) {
				info_->inputs.push_back(node_to_qubit_[node]);
			}
		});

		std::vector<tweedledum::io_id> ancillae_to_release;
		std::vector<mt_node_type> gate_to_uncompute;
		c_network_.foreach_node([&](auto node) {
			compute_ltfi(node);
			if (c_network_.is_and(node)) {
				// Compute inputs
				auto gates = compute_inputs(node);
				std::for_each(gates.begin(), gates.end(), [&](auto& gate) {
					auto& [control, target] = gate;
					compute_xor(control, target);
				});

				// Compute node (Always Out-of-place)
				const auto controls = control_qubits(node);
				const auto target = request_ancilla();
				node_to_qubit_[node] = target;
				compute_and(controls, target);

				// Uncompute inputs
				std::for_each(gates.rbegin(), gates.rend(), [&](auto& gate) {
					auto& [control, target] = gate;
					compute_xor(control, target);
				});
				gate_to_uncompute.push_back(node);
				ancillae_to_release.push_back(target);
			} 
		});

		c_network_.clear_values();
		c_network_.foreach_po([&](auto const signal, auto index) {
			auto node = c_network_.get_node(signal);
			if (c_network_.is_and(node)) {
				c_network_.set_value(node, index);
				is_ancilla_[node_to_qubit_[node]] = 0;
			} else if (c_network_.is_xor(node)) {
				c_network_.set_value(node, index);
				for (auto in : node_ltfi_[node]) {
					auto in_node = c_network_.get_node(in);
					if (c_network_.is_and(in_node)) {
						c_network_.set_value(in_node, index);
					}
				}
			}
		});
		c_network_.foreach_po([&](auto const signal, auto index) {
			auto node = c_network_.get_node(signal);
			io_id output_qubit = node_to_qubit_[node];
			if (c_network_.is_constant(node)) {
				output_qubit = request_ancilla();
				if (c_network_.is_complemented(signal)) {
					q_network_.add_gate(gate::pauli_x, output_qubit);
				}
			} else if (c_network_.value(node) != index) {
				output_qubit = request_ancilla();
				q_network_.add_gate(gate::cx, node_to_qubit_[node], output_qubit);
				if (c_network_.is_complemented(signal)) {
					q_network_.add_gate(gate::pauli_x, output_qubit);
				}
			} else {
				if (c_network_.is_xor(node)) {
					/* XOR that drives an output */
					std::vector<io_id> controls;
					bool found_output = false;
					for (auto in : node_ltfi_[node]) {
						const io_id qubit = node_to_qubit_[in];
						auto in_node = c_network_.get_node(in);
						if (!found_output && c_network_.is_and(in_node) && c_network_.value(in_node) == index) {
							output_qubit = qubit;
							found_output = true;
						} else {
							controls.emplace_back(qubit);
						}
					}
					if (output_qubit == io_invalid) {
						output_qubit = request_ancilla();
					}
					is_ancilla_[output_qubit] = 0;
					node_to_qubit_[node] = output_qubit;
					compute_xor(controls, output_qubit);
				}
				if (c_network_.is_complemented(signal)) {
					q_network_.add_gate(gate::pauli_x, output_qubit);
				}
			}
			if (info_) {
				info_->outputs.push_back(output_qubit);
			}
		});

		std::reverse(gate_to_uncompute.begin(), gate_to_uncompute.end());
		std::reverse(ancillae_to_release.begin(), ancillae_to_release.end());
		for (auto i = 0u; i < gate_to_uncompute.size(); ++i) {
			auto n = gate_to_uncompute[i];
			if (is_ancilla_.at(ancillae_to_release[i]) == 0) {
				continue;
			}
			auto gs = compute_inputs(n);
			std::for_each(gs.begin(), gs.end(), [&](auto& gate) {
				auto& [control, target] = gate;
				compute_xor(control, target);
			});
			const auto controls = control_qubits(n);
			compute_and(controls, ancillae_to_release[i]);
			std::for_each(gs.rbegin(), gs.rend(), [&](auto& gate) {
				auto& [control, target] = gate;
				compute_xor(control, target);
			});
			release_ancilla(ancillae_to_release[i]);
			node_to_qubit_[n] = io_invalid;
		}
		ancillae_to_release.clear();
		gate_to_uncompute.clear();
	}

private:
	// lfti = linear transitive fanin
	void compute_ltfi(mt_node_type const& node)
	{
		std::vector<mt_signal_type> ltfi;
		if (c_network_.is_and(node) || c_network_.is_pi(node)) {
			ltfi.emplace_back(c_network_.make_signal(node));
		} else {
			std::array<mt_signal_type, 2> fanins; 
			c_network_.foreach_fanin(node, [&](auto const& signal, auto i) {
				fanins.at(i) = signal;
			});
			const auto& ltfi_in0 = node_ltfi_[fanins[0]];
			const auto& ltfi_in1 = node_ltfi_[fanins[1]];
			std::set_symmetric_difference(ltfi_in0.begin(), ltfi_in0.end(),
			                              ltfi_in1.begin(), ltfi_in1.end(),
						      std::back_inserter(ltfi));
			std::sort(ltfi.begin(), ltfi.end());
		}
		node_ltfi_[node] = ltfi;
	}

	void add_constant(bool constant)
	{
		const mt_signal_type signal = c_network_.get_constant(constant);
		const mt_node_type node = c_network_.get_node(signal);
		if (c_network_.fanout_size(node) == 0) {
			return;
		}
		node_to_qubit_[node] = q_network_.add_qubit();
	}

	io_id request_ancilla()
	{
		if (free_ancillae_.empty()) {
			io_id qubit = q_network_.add_qubit();
			qubit_usage_.emplace_back(0);
			is_ancilla_.emplace_back(1);
			return qubit;
		} else {
			io_id qubit = free_ancillae_.top();
			free_ancillae_.pop();
			return qubit;
		}
	}

	void release_ancilla(io_id qubit)
	{
		free_ancillae_.push(qubit);
	}

	// Return the control qubits
	std::vector<io_id> control_qubits(mt_node_type const& node)
	{
		std::vector<io_id> controls;
		c_network_.foreach_fanin(node, [&](auto const& signal) {
			const io_id qubit = node_to_qubit_[c_network_.get_node(signal)];
			if (c_network_.is_complemented(signal)) {
				controls.emplace_back(!qubit);
			} else {
				controls.emplace_back(qubit);
			}
		});
		return controls;
	}

	std::vector<std::pair<io_id, io_id>> compute_inputs(mt_node_type const& node)
	{
		std::vector<mt_signal_type> node_l0_l1;
		std::vector<std::pair<io_id, io_id>> gates;
		bool both_xor = true;
		c_network_.foreach_fanin(node, [&](auto const& signal) {
			auto fanin = c_network_.get_node(signal);
			if (!c_network_.is_xor(fanin)) {
				both_xor = false;
			}
			node_l0_l1.emplace_back(signal);
		});

		auto* l0 = &(node_ltfi_[node_l0_l1.at(0)]);
		auto* l1 = &(node_ltfi_[node_l0_l1.at(1)]);
		if (both_xor) {
			bool l0_includes_l1 = false;
			if (std::includes(l0->begin(), l0->end(), l1->begin(), l1->end())) {
				l0_includes_l1 = true;
			} else if (std::includes(l1->begin(), l1->end(), l0->begin(), l0->end())) {
				std::swap(l0, l1);
				std::swap(node_l0_l1[0], node_l0_l1[1]);
				l0_includes_l1 = true;
			}
			std::vector<mt_signal_type> subset_l0;
			std::set_difference(l0->begin(), l0->end(),
			                    l1->begin(), l1->end(),
					    std::back_inserter(subset_l0));

			// Prepare L1
			auto r_qubit = node_to_qubit_[c_network_.get_node(l1->at(0))];
			for (auto r = 1u; r < l1->size(); ++r) {
				auto t_qubit = node_to_qubit_[c_network_.get_node(l1->at(r))];
				if (qubit_usage_[r_qubit] < qubit_usage_[t_qubit]) {
					r_qubit = t_qubit;
				}
			}
			qubit_usage_[r_qubit] += 1;
			auto target_l1 = r_qubit;
			std::for_each(l1->begin(), l1->end(), 
			[&](auto& t) {
				assert(node_to_qubit_[c_network_.get_node(t)] != io_invalid);
				if (node_to_qubit_[c_network_.get_node(t)] == target_l1) {
					return;
				}
				gates.emplace_back(node_to_qubit_[c_network_.get_node(t)], target_l1);
			});
			node_to_qubit_[c_network_.get_node(node_l0_l1[1])] = target_l1;

			// Prepare L0
			r_qubit = node_to_qubit_[c_network_.get_node(subset_l0.at(0))];
			for (auto r = 1u; r < subset_l0.size(); ++r) {
				auto t_qubit = node_to_qubit_[c_network_.get_node(subset_l0.at(r))];
				if (qubit_usage_[r_qubit] < qubit_usage_[t_qubit]) {
					r_qubit = t_qubit;
				}
			}
			qubit_usage_[r_qubit] += 1;
			auto target_l0 = r_qubit;
			if (l0_includes_l1) {
				// fmt::print("Includes!\n");
				std::for_each(subset_l0.begin(), subset_l0.end(), 
				[&](auto& t) {
					assert(node_to_qubit_[c_network_.get_node(t)] != io_invalid);
					if (node_to_qubit_[c_network_.get_node(t)] == target_l0) {
						return;
					}
					gates.emplace_back(node_to_qubit_[c_network_.get_node(t)], target_l0);
				});
				gates.emplace_back(target_l1, target_l0);
				node_to_qubit_[c_network_.get_node(node_l0_l1[0])] = target_l0;
			} else {
				std::for_each(l0->begin(), l0->end(), 
				[&](auto& t) {
					assert(node_to_qubit_[c_network_.get_node(t)] != io_invalid);
					if (node_to_qubit_[c_network_.get_node(t)] == target_l0) {
						return;
					};
					gates.emplace_back(node_to_qubit_[c_network_.get_node(t)], target_l0);
				});
				node_to_qubit_[c_network_.get_node(node_l0_l1[0])] = target_l0;
				std::reverse(gates.begin(), gates.end());
			}
		} else {
			auto fanin1 = c_network_.get_node(node_l0_l1.at(1));
			if (c_network_.is_xor(fanin1)) {
				std::swap(l0, l1);
				std::swap(node_l0_l1[0], node_l0_l1[1]);
			}
			std::vector<mt_signal_type> subset_l0;
			std::set_difference(l0->begin(), l0->end(),
			                    l1->begin(), l1->end(),
					    std::back_inserter(subset_l0));

			auto r_qubit = node_to_qubit_[c_network_.get_node(subset_l0.at(0))];
			for (auto r = 1u; r < subset_l0.size(); ++r) {
				auto t_qubit = node_to_qubit_[c_network_.get_node(subset_l0.at(r))];
				if (qubit_usage_[r_qubit] < qubit_usage_[t_qubit]) {
					r_qubit = t_qubit;
				}
			}
			qubit_usage_[r_qubit] += 1;
		
			auto target = r_qubit;
			std::for_each(l0->begin(), l0->end(), 
			[&](auto& control) {
				assert(node_to_qubit_[c_network_.get_node(control)] != io_invalid);
				if (node_to_qubit_[c_network_.get_node(control)] == target) {
					return;
				};
				gates.emplace_back(node_to_qubit_[c_network_.get_node(control)], target);
			});
			node_to_qubit_[c_network_.get_node(node_l0_l1[0])] = target;
		}
		return gates;
	}

	void compute_and(std::vector<io_id> const& controls, io_id target)
	{
		q_network_.add_gate(gate::mcx, controls, std::vector<io_id>({target}));
	}

	void compute_xor(io_id control, io_id target)
	{
		q_network_.add_gate(gate::cx, control.id(), target);
		if (control.is_complemented()) {
			q_network_.add_gate(gate::pauli_x, target);
		}
	}

	void compute_xor(std::vector<io_id> const& controls, io_id target)
	{
		bool invert = false;
		for (auto& control : controls) {
			q_network_.add_gate(gate::cx, control.id(), target);
			invert ^= control.is_complemented();
		} 
		if (invert) {
			q_network_.add_gate(gate::pauli_x, target);
		}
	}

private:
	QuantumNetwork& q_network_;
	LogicNetwork const& c_network_;
	hrs_params params_;
	hrs_info* info_;

	// Internal
	mockturtle::node_map<io_id, LogicNetwork> node_to_qubit_;
	mockturtle::node_map<std::vector<mt_signal_type>, LogicNetwork> node_ltfi_;
	std::vector<uint32_t> qubit_usage_;
	std::vector<uint8_t> is_ancilla_;
	std::stack<io_id> free_ancillae_;
};

} // namespace detail

/*! \brief Hierarchical reversible logic synthesis based on an irreversible XAG logic network.
 */
template<class QuantumNetwork>
void hrs(QuantumNetwork& q_network, mockturtle::xag_network const& c_network,
         hrs_info* info = nullptr, hrs_params const& params = {})
{
	detail::hrs_xag synthesizer(q_network, c_network, info, params);
	synthesizer.synthesize();
}

/*! \brief Hierarchical reversible logic synthesis based on an irreversible XAG logic network.
 */
template<class QuantumNetwork>
QuantumNetwork hrs(mockturtle::xag_network const& c_network, hrs_info* info = nullptr,
                   hrs_params const& params = {})
{
	QuantumNetwork q_network;
	hrs(q_network, c_network, info, params);
	return q_network;
}

} // namespace tweedledum
