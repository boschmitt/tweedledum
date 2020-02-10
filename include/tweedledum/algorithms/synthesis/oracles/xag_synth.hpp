/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../../gates/gate_base.hpp"
#include "../../../networks/io_id.hpp"
#include "../stg.hpp"

#include <fmt/format.h>
#include <mockturtle/networks/xag.hpp>
#include <mockturtle/traits.hpp>
#include <mockturtle/utils/node_map.hpp>
#include <stack>
#include <vector>

namespace tweedledum {

struct xag_synth_params {
	/*! \brief Be verbose. */
	bool verbose = false;
};

namespace detail {

template<class QuantumNtk>
class xag_synth {
	using LogicNtk = mockturtle::xag_network;
	using mt_node_type = typename LogicNtk::node;
	using mt_signal_type = typename LogicNtk::signal;

public:
	xag_synth(QuantumNtk& quantum_ntk, LogicNtk const& xag_ntk,
	          xag_synth_params const& params)
	    : quantum_ntk_(quantum_ntk)
	    , xag_ntk_(xag_ntk)
	    , params_(params)
	    , node_to_qubit_(xag_ntk, io_invalid)
	    , node_ltfi_(xag_ntk)
	    , must_uncompute_(xag_ntk.size(), 0)
	{}

	void synthesize()
	{
		create_inputs();

		// Compute AND gates
		xag_ntk_.foreach_node([&](auto node, auto index) {
			compute_ltfi(node);
			if (xag_ntk_.is_and(node)) {
				quantum_ntk_.set_default_value(index);
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
				must_uncompute_.at(index) = 1;
			} 
		});

		quantum_ntk_.set_default_value(0u);
		create_outputs();

		// Before uncomputing the AND gates, an operation which will add gates to network,
		// I must make sure there is enough capacity in the network.  As I will be iterating
		// each gate in a reverse order, using ``foreach_rgate``,  I need to guarantee that
		// adding new gates will not invalidate the iterators by triggering a reallocation
		// of the vector which hold all nodes in the network.
		if (quantum_ntk_.capacity() < (2 * quantum_ntk_.size())) {
			quantum_ntk_.reserve(2 * quantum_ntk_.size());
		}
		// Uncompute AND gates
		quantum_ntk_.foreach_rgate([&](auto const& node) {
			if (must_uncompute_.at(quantum_ntk_.value(node))) {
				quantum_ntk_.emplace_gate(node.gate);
			}
		});
		quantum_ntk_.clear_values();
	}

private:
	// Here I create one qubit for each primary input and mark them accordingly.  I also create
	// a vector `qubit_usage_` that I will use to keep track of how many times a qubit is used
	// as a target.
	void create_inputs()
	{
		xag_ntk_.foreach_pi([&](mt_node_type const& node, auto index) { 
			node_to_qubit_[node] = quantum_ntk_.add_qubit(fmt::format("i_{}", index));
			quantum_ntk_.mark_as_input(node_to_qubit_[node]);
			qubit_usage_.emplace_back(0u);
		});
	}

	void analyze_outputs()
	{
		// I will need to make sure the node of the classical network are not marked.
		xag_ntk_.clear_values();
		xag_ntk_.foreach_po([&](auto const signal, auto index) {
			auto node = xag_ntk_.get_node(signal);
			if (xag_ntk_.is_and(node)) {
				xag_ntk_.set_value(node, index);
			} 
			// If this output points to a XOR gate, then I search for a AND gate in its
			// LTFI set.  This would mean that this XOR gate is controlled by the AND
			// gate.  Then what I can do is to invert control and target of the XOR.
			// For example:
			//
			// This allows me to save gates.
			else if (xag_ntk_.is_xor(node)) {
				xag_ntk_.set_value(node, index);
				for (auto in : node_ltfi_[node]) {
					auto in_node = xag_ntk_.get_node(in);
					if (xag_ntk_.is_and(in_node)) {
						xag_ntk_.set_value(in_node, index);
					}
				}
			}
		});
	}

	// Creating the outputs could be as trivial as creating the inputs.  At this point we have 
	// computed all AND nodes of the network and the results for the nodes corresponding to the
	// primary outputs are in the ancillae that we added.  Hence we could simply create new
	// qubits and use XOR operations to 'copy' the results from the ancillae, and then
	// uncompute all AND nodes.
	//
	// This trivial way, however, is quite wastefull in both number of qubits and number of
	// gates.  A smarter approach requires a more complex implementation.
	void create_outputs()
	{
		// First I will need to do one pass to analyze the primary outputs.
		analyze_outputs();

		xag_ntk_.foreach_po([&](auto const signal, auto index) {
			auto node = xag_ntk_.get_node(signal);
			io_id output_qubit = node_to_qubit_[node];

			// Deal with constants. Here I assume all qubits are initialized with |0>
			if (xag_ntk_.is_constant(node)) {
				output_qubit = quantum_ntk_.add_qubit();
			} else if (xag_ntk_.value(node) != index) {
				output_qubit = quantum_ntk_.add_qubit();
				quantum_ntk_.add_gate(gate::cx, node_to_qubit_[node], output_qubit);
			} else if (xag_ntk_.is_and(node)) {
				quantum_ntk_.unmark_as_ancilla(node_to_qubit_[node]);
				must_uncompute_.at(xag_ntk_.node_to_index(node)) = 0;
			} else if (xag_ntk_.is_xor(node)) {
				std::vector<io_id> controls;
				bool found_output = false;
				for (auto in : node_ltfi_[node]) {
					const io_id qubit = node_to_qubit_[in];
					auto in_node = xag_ntk_.get_node(in);
					if (!found_output && xag_ntk_.is_and(in_node) && xag_ntk_.value(in_node) == index) {
						output_qubit = qubit;
						found_output = true;
						must_uncompute_.at(xag_ntk_.node_to_index(in_node)) = 0;
					} else {
						controls.emplace_back(qubit);
					}
				}
				if (output_qubit == io_invalid) {
					output_qubit = quantum_ntk_.add_qubit();
				}
				node_to_qubit_[node] = output_qubit;
				compute_xor(controls, output_qubit);
			}
			if (xag_ntk_.is_complemented(signal)) {
				quantum_ntk_.add_gate(gate::pauli_x, output_qubit);
			}
			quantum_ntk_.unmark_as_ancilla(output_qubit);
			quantum_ntk_.mark_as_output(output_qubit);
			quantum_ntk_.io_set_label(output_qubit, fmt::format("o_{}", index));
		});
	}

private:
	// lfti = linear transitive fanin
	void compute_ltfi(mt_node_type const& node)
	{
		std::vector<mt_signal_type> ltfi;
		if (xag_ntk_.is_and(node) || xag_ntk_.is_pi(node)) {
			ltfi.emplace_back(xag_ntk_.make_signal(node));
		} else {
			std::array<mt_signal_type, 2> fanins; 
			xag_ntk_.foreach_fanin(node, [&](auto const& signal, auto i) {
				fanins.at(i) = signal;
			});
			const auto& ltfi_in0 = node_ltfi_[fanins[0]];
			const auto& ltfi_in1 = node_ltfi_[fanins[1]];
			std::set_symmetric_difference(ltfi_in0.begin(), ltfi_in0.end(),
			                              ltfi_in1.begin(), ltfi_in1.end(),
						      std::back_inserter(ltfi));
			std::sort(ltfi.begin(), ltfi.end());
		}
		assert(ltfi.size() && "If the size is empty, then the XAG was not properly optmized!");
		node_ltfi_[node] = ltfi;
	}

	void add_constant(bool constant)
	{
		const mt_signal_type signal = xag_ntk_.get_constant(constant);
		const mt_node_type node = xag_ntk_.get_node(signal);
		if (xag_ntk_.fanout_size(node) == 0) {
			return;
		}
		node_to_qubit_[node] = quantum_ntk_.add_qubit();
	}

	io_id request_ancilla()
	{
		if (free_ancillae_.empty()) {
			io_id qubit = quantum_ntk_.add_qubit(/* is_ancilla = */ true);
			qubit_usage_.emplace_back(0);
			return qubit;
		} else {
			io_id qubit = free_ancillae_.top();
			free_ancillae_.pop();
			return qubit;
		}
	}

	// Return the control qubits
	std::vector<io_id> control_qubits(mt_node_type const& node)
	{
		std::vector<io_id> controls;
		xag_ntk_.foreach_fanin(node, [&](auto const& signal) {
			const io_id qubit = node_to_qubit_[xag_ntk_.get_node(signal)];
			if (xag_ntk_.is_complemented(signal)) {
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
		xag_ntk_.foreach_fanin(node, [&](auto const& signal) {
			auto fanin = xag_ntk_.get_node(signal);
			if (!xag_ntk_.is_xor(fanin)) {
				both_xor = false;
			}
			node_l0_l1.emplace_back(signal);
		});

		auto* l0 = &(node_ltfi_[node_l0_l1.at(0)]);
		auto* l1 = &(node_ltfi_[node_l0_l1.at(1)]);
		assert(l0->size());
		assert(l1->size());
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
			auto r_qubit = node_to_qubit_[xag_ntk_.get_node(l1->at(0))];
			for (auto r = 1u; r < l1->size(); ++r) {
				auto t_qubit = node_to_qubit_[xag_ntk_.get_node(l1->at(r))];
				if (qubit_usage_[r_qubit] < qubit_usage_[t_qubit]) {
					r_qubit = t_qubit;
				}
			}
			qubit_usage_[r_qubit] += 1;
			auto target_l1 = r_qubit;
			std::for_each(l1->begin(), l1->end(), 
			[&](auto& t) {
				assert(node_to_qubit_[xag_ntk_.get_node(t)] != io_invalid);
				if (node_to_qubit_[xag_ntk_.get_node(t)] == target_l1) {
					return;
				}
				gates.emplace_back(node_to_qubit_[xag_ntk_.get_node(t)], target_l1);
			});
			node_to_qubit_[xag_ntk_.get_node(node_l0_l1[1])] = target_l1;

			// Prepare L0
			r_qubit = node_to_qubit_[xag_ntk_.get_node(subset_l0.at(0))];
			for (auto r = 1u; r < subset_l0.size(); ++r) {
				auto t_qubit = node_to_qubit_[xag_ntk_.get_node(subset_l0.at(r))];
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
					assert(node_to_qubit_[xag_ntk_.get_node(t)] != io_invalid);
					if (node_to_qubit_[xag_ntk_.get_node(t)] == target_l0) {
						return;
					}
					gates.emplace_back(node_to_qubit_[xag_ntk_.get_node(t)], target_l0);
				});
				gates.emplace_back(target_l1, target_l0);
				node_to_qubit_[xag_ntk_.get_node(node_l0_l1[0])] = target_l0;
			} else {
				std::for_each(l0->begin(), l0->end(), 
				[&](auto& t) {
					assert(node_to_qubit_[xag_ntk_.get_node(t)] != io_invalid);
					if (node_to_qubit_[xag_ntk_.get_node(t)] == target_l0) {
						return;
					};
					gates.emplace_back(node_to_qubit_[xag_ntk_.get_node(t)], target_l0);
				});
				node_to_qubit_[xag_ntk_.get_node(node_l0_l1[0])] = target_l0;
				std::reverse(gates.begin(), gates.end());
			}
		} else {
			auto fanin1 = xag_ntk_.get_node(node_l0_l1.at(1));
			if (xag_ntk_.is_xor(fanin1)) {
				std::swap(l0, l1);
				std::swap(node_l0_l1[0], node_l0_l1[1]);
			}
			std::vector<mt_signal_type> subset_l0;
			std::set_difference(l0->begin(), l0->end(),
			                    l1->begin(), l1->end(),
					    std::back_inserter(subset_l0));

			auto r_qubit = node_to_qubit_[xag_ntk_.get_node(subset_l0.at(0))];
			for (auto r = 1u; r < subset_l0.size(); ++r) {
				auto t_qubit = node_to_qubit_[xag_ntk_.get_node(subset_l0.at(r))];
				if (qubit_usage_[r_qubit] < qubit_usage_[t_qubit]) {
					r_qubit = t_qubit;
				}
			}
			qubit_usage_[r_qubit] += 1;
		
			auto target = r_qubit;
			std::for_each(l0->begin(), l0->end(), 
			[&](auto& control) {
				assert(node_to_qubit_[xag_ntk_.get_node(control)] != io_invalid);
				if (node_to_qubit_[xag_ntk_.get_node(control)] == target) {
					return;
				};
				gates.emplace_back(node_to_qubit_[xag_ntk_.get_node(control)], target);
			});
			node_to_qubit_[xag_ntk_.get_node(node_l0_l1[0])] = target;
		}
		return gates;
	}

	void compute_and(std::vector<io_id> const& controls, io_id target)
	{
		quantum_ntk_.add_gate(gate::mcx, controls, std::vector<io_id>({target}));
	}

	void compute_xor(io_id control, io_id target)
	{
		quantum_ntk_.add_gate(gate::cx, control.id(), target);
		if (control.is_complemented()) {
			quantum_ntk_.add_gate(gate::pauli_x, target);
		}
	}

	void compute_xor(std::vector<io_id> const& controls, io_id target)
	{
		bool invert = false;
		for (auto& control : controls) {
			quantum_ntk_.add_gate(gate::cx, control.id(), target);
			invert ^= control.is_complemented();
		} 
		if (invert) {
			quantum_ntk_.add_gate(gate::pauli_x, target);
		}
	}

private:
	QuantumNtk& quantum_ntk_;
	LogicNtk const& xag_ntk_;
	xag_synth_params params_;

	// Internal
	mockturtle::node_map<io_id, LogicNtk> node_to_qubit_;
	mockturtle::node_map<std::vector<mt_signal_type>, LogicNtk> node_ltfi_;
	std::vector<uint32_t> qubit_usage_;
	std::stack<io_id> free_ancillae_;
	std::vector<uint8_t> must_uncompute_;
};

} // namespace detail

/*! \brief Oracle synthesis from a XAG graph (from mockturtle) */
template<class QuantumNtk>
void xag_synth(QuantumNtk& quantum_ntk, mockturtle::xag_network const& xag_ntk,
               xag_synth_params const& params = {})
{
	detail::xag_synth synthesizer(quantum_ntk, xag_ntk, params);
	synthesizer.synthesize();
}

/*! \brief Oracle synthesis from a XAG graph (from mockturtle) */
template<class QuantumNtk>
QuantumNtk xag_synth(mockturtle::xag_network const& xag_ntk, xag_synth_params const& params = {})
{
	QuantumNtk quantum_ntk;
	xag_synth(quantum_ntk, xag_ntk, params);
	return quantum_ntk;
}

} // namespace tweedledum
