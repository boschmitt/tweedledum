/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../../gates/gate.hpp"
#include "../../../networks/wire_id.hpp"

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
	using xag_node = typename LogicNtk::node;
	using xag_signal = typename LogicNtk::signal;

	using q_node = typename QuantumNtk::node_type;
	using q_op = typename QuantumNtk::op_type;

	struct gate_info {
		wire_id control;
		wire_id target;

		gate_info(wire_id c, wire_id t)
		    : control(c)
		    , target(t)
		{}
	};

public:
	xag_synth(QuantumNtk& quantum_ntk, LogicNtk const& xag_ntk,
	          xag_synth_params const& params)
	    : quantum_ntk_(quantum_ntk)
	    , xag_ntk_(xag_ntk)
	    , params_(params)
	    , node_to_qubit_(xag_ntk, wire::invalid)
	    , node_ltfi_(xag_ntk)
	    , must_uncompute_(xag_ntk.size(), 0)
	{}

	void synthesize()
	{
		create_inputs();
		analyze_xag();

		// Compute AND gates
		xag_ntk_.foreach_node([&](xag_node const& node, uint32_t index) {
			if (!xag_ntk_.is_and(node)) {
				return;
			} 
			compute_and(node, index);
		});

		quantum_ntk_.default_value(0u);
		create_outputs();

		// Before uncomputing the AND gates, which will add operations to network. I must
		// make sure there is enough capacity in the network.  As I will be iterating each
		// operation in a reverse order, using ``foreach_rop``,  I need to guarantee that
		// adding new operations will not invalidate the iterators by triggering a
		// reallocation of the vector which hold all nodes in the network.
		if (quantum_ntk_.capacity() < (2 * quantum_ntk_.size())) {
			quantum_ntk_.reserve(2 * quantum_ntk_.size());
		}
		// Uncompute AND gates
		quantum_ntk_.foreach_rop([&](q_op const& op, q_node const& node) {
			if (must_uncompute_.at(quantum_ntk_.value(node))) {
				quantum_ntk_.emplace_op(op);
			}
		});
		quantum_ntk_.clear_values();
	}

private:
	template<class Iterator, class ElementType = typename Iterator::value_type, class Fn>
	void iterate_union(Iterator begin0, Iterator end0, Iterator begin1, Iterator end1, Fn&& fn)
	{
		while (begin0 != end0) {
			if (begin1 == end1) {
				while (begin0 != end0) {
					fn(*begin0++);
				}
				return;
			}
			if (*begin1 < *begin0) {
				fn(*begin1++);
			} else {
				fn(*begin0);
				if (!(*begin0 < *begin1)) {
					++begin1;
				}
				++begin0;
			}
		}
		while (begin1 != end1) {
			fn(*begin1++);
		}
	}

	// Here I will mark how many times a AND gate appears in the LTFI of another AND gate
	void analyze_xag()
	{
		uint32_t num_gates = 0u;
		xag_ntk_.clear_values();
		xag_ntk_.foreach_gate([&](xag_node const& node) {
			compute_ltfi(node);
			if (!xag_ntk_.is_and(node)) {
				return;
			}
			std::array<xag_signal, 2> fanins;
			xag_ntk_.foreach_fanin(node, [&](xag_signal const signal, uint32_t i) {
				fanins.at(i) = signal;
			});
			std::vector<xag_signal> const& ltfi_in0 = node_ltfi_[fanins.at(0)];
			std::vector<xag_signal> const& ltfi_in1 = node_ltfi_[fanins.at(1)];
			assert(!ltfi_in0.empty());
			assert(!ltfi_in1.empty());
			iterate_union(ltfi_in0.cbegin(), ltfi_in0.cend(), ltfi_in1.cbegin(),
			              ltfi_in1.cend(), [&](xag_signal const& signal) {
			                      xag_ntk_.incr_value(xag_ntk_.get_node(signal));
			                      ++num_gates;
			                });
		});
		quantum_ntk_.reserve(4 * num_gates);
	}

	void analyze_outputs()
	{
		// I will need to make sure the node of the classical network are not marked.
		xag_ntk_.clear_values();
		xag_ntk_.foreach_po([&](xag_signal const signal, auto index) {
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

private:
	wire_id request_ancilla()
	{
		if (free_ancillae_.empty()) {
			wire_id qubit = quantum_ntk_.create_qubit(wire_modes::ancilla);
			qubit_usage_.emplace_back(0);
			return qubit;
		} else {
			wire_id qubit = free_ancillae_.top();
			free_ancillae_.pop();
			return qubit;
		}
	}

	// Here I create one qubit for each primary input and mark them accordingly.  I also create
	// a vector `qubit_usage_` that I will use to keep track of how many times a qubit is used
	// as a target.
	void create_inputs()
	{
		xag_ntk_.foreach_pi([&](xag_node const& node, auto index) {
			node_to_qubit_[node] = quantum_ntk_.create_qubit(fmt::format("i_{}", index),
			                                                 wire_modes::in);
			node_ltfi_[node].emplace_back(xag_ntk_.make_signal(node));
			qubit_usage_.emplace_back(0u);
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

		xag_ntk_.foreach_po([&](xag_signal const signal, uint32_t index) {
			xag_node const& node = xag_ntk_.get_node(signal);
			wire_id output_qubit = node_to_qubit_[node];
			wire_modes mode = wire_modes::out;

			// Deal with constants. Here I assume all qubits are initialized with |0>
			if (xag_ntk_.is_constant(node)) {
				output_qubit = quantum_ntk_.create_qubit();
			} else if (xag_ntk_.value(node) != index) {
				output_qubit = quantum_ntk_.create_qubit();
				create_xor(node_to_qubit_[node], output_qubit);
			} else if (xag_ntk_.is_and(node)) {
				must_uncompute_.at(xag_ntk_.node_to_index(node)) = 0;
			} else if (xag_ntk_.is_xor(node)) {
				std::vector<wire_id> controls;
				bool found_output = false;
				for (auto in : node_ltfi_[node]) {
					const wire_id qubit = node_to_qubit_[in];
					auto in_node = xag_ntk_.get_node(in);
					if (!found_output && xag_ntk_.is_and(in_node) && xag_ntk_.value(in_node) == index) {
						output_qubit = qubit;
						found_output = true;
						must_uncompute_.at(xag_ntk_.node_to_index(in_node)) = 0;
					} else {
						controls.emplace_back(qubit);
					}
				}
				if (output_qubit == wire::invalid) {
					output_qubit = quantum_ntk_.create_qubit();
				}
				node_to_qubit_[node] = output_qubit;
				create_xor(controls, output_qubit);
			}
			if (xag_ntk_.is_complemented(signal)) {
				quantum_ntk_.create_op(gate_lib::x, output_qubit);
			}
			assert(quantum_ntk_.wire_mode(output_qubit) != wire_modes::in);
			quantum_ntk_.wire_mode(output_qubit, mode);
			// I need to set a internal qubit name, so I can equivalence check the
			// result.  This hack make sure the outputs are not permuted:
			quantum_ntk_.wire_label(output_qubit, fmt::format("__o_{}", index));
		});
	}

	void create_and(std::vector<wire_id> const& controls, wire_id target)
	{
		quantum_ntk_.create_op(gate_lib::ncx, controls, std::vector<wire_id>({target}));
	}

	void create_xor(wire_id control, wire_id target)
	{
		quantum_ntk_.create_op(gate_lib::cx, control.wire(), target);
		if (control.is_complemented()) {
			quantum_ntk_.create_op(gate_lib::x, target);
		}
	}

	void create_xor(std::vector<wire_id> const& controls, wire_id target)
	{
		bool invert = false;
		for (auto& control : controls) {
			quantum_ntk_.create_op(gate_lib::cx, control.wire(), target);
			invert ^= control.is_complemented();
		} 
		if (invert) {
			quantum_ntk_.create_op(gate_lib::x, target);
		}
	}

private:
	// lfti = linear transitive fanin
	void compute_ltfi(xag_node const& node)
	{
		if (xag_ntk_.is_and(node)) {
			node_ltfi_[node].emplace_back(xag_ntk_.make_signal(node));
			return;
		}
		std::array<xag_signal, 2> fanins;
		xag_ntk_.foreach_fanin(node, [&](xag_signal const& signal, uint32_t i) {
			fanins.at(i) = signal;
		});
		auto const& ltfi_in0 = node_ltfi_[fanins.at(0)];
		auto const& ltfi_in1 = node_ltfi_[fanins.at(1)];
		std::set_symmetric_difference(ltfi_in0.begin(), ltfi_in0.end(), ltfi_in1.begin(),
		                              ltfi_in1.end(), std::back_inserter(node_ltfi_[node]));
		assert(node_ltfi_[node].size()
		       && "If the size is empty, then the XAG was not properly optmized!");
	}

	// Return the control qubits
	std::vector<wire_id> control_qubits(xag_node const& node)
	{
		std::vector<wire_id> controls;
		xag_ntk_.foreach_fanin(node, [&](xag_signal const& signal) {
			const wire_id qubit = node_to_qubit_[signal];
			assert(qubit != wire::invalid);
			if (xag_ntk_.is_complemented(signal)) {
				controls.emplace_back(!qubit);
			} else {
				controls.emplace_back(qubit);
			}
		});
		return controls;
	}

	wire_id choose_target(std::vector<xag_signal> const& ltfi)
	{
		assert(ltfi.size());
		wire_id chosen_qubit = node_to_qubit_[ltfi.at(0)];
		std::for_each(ltfi.begin() + 1, ltfi.end(), [&](xag_signal const& signal) {
			wire_id qubit = node_to_qubit_[signal];
			if (qubit_usage_.at(chosen_qubit) < qubit_usage_.at(qubit)) {
				chosen_qubit = qubit;
			}
		});
		assert(chosen_qubit != wire::invalid);
		qubit_usage_.at(chosen_qubit) += 1;
		return chosen_qubit;
	}

	void compute_xor_ios(std::vector<xag_signal> const& ltfi, wire_id target,
	                     std::vector<gate_info>& gates)
	{
		std::for_each(ltfi.begin(), ltfi.end(), [&](xag_signal const& signal) {
			wire_id control = node_to_qubit_[signal]; 
			assert( control != wire::invalid);
			if (control == target) {
				return;
			}
			gates.emplace_back(control, target);
		});
	}

	void compute_sets(std::vector<xag_signal> const& ltfi_0,
	                  std::vector<xag_signal> const& ltfi_1,
	                  std::vector<xag_signal>& in0,
	                  std::vector<xag_signal>& in1,
	                  std::vector<xag_signal>& intersection_in01)
	{
		in0.reserve(ltfi_0.size());
		in1.reserve(ltfi_1.size());
		intersection_in01.reserve(ltfi_1.size());

		auto in0_begin = ltfi_0.cbegin();
		auto in0_end = ltfi_0.cend();
		auto in1_begin = ltfi_1.cbegin();
		auto in1_end = ltfi_1.cend();
		while (in0_begin != in0_end && in1_begin != in1_end) {
			if (*in0_begin == *in1_begin) {
				intersection_in01.emplace_back(*in0_begin);
				++in0_begin;
				++in1_begin;
			} else if (*in0_begin < *in1_begin) {
				in0.emplace_back(*in0_begin);
				++in0_begin;
			} else {
				in1.emplace_back(*in1_begin);
				++in1_begin;
			}
		}
		std::copy(in0_begin, in0_end, std::back_inserter(in0));
		std::copy(in1_begin, in1_end, std::back_inserter(in1));
	}

	// This function computes the AND gate inputs __in-place__
	std::vector<gate_info> compute_and_inputs(xag_node const& node)
	{
		std::array<xag_signal, 2> fanins;
		std::vector<gate_info> gates;
		bool both_xor = true;
		xag_ntk_.foreach_fanin(node, [&](xag_signal const signal, uint32_t index) {
			xag_node const& fanin = xag_ntk_.get_node(signal);
			both_xor &= xag_ntk_.is_xor(fanin);
			fanins.at(index) = signal;
		});

		std::vector<xag_signal>* ltfi_0 = &(node_ltfi_[fanins.at(0)]);
		std::vector<xag_signal>* ltfi_1 = &(node_ltfi_[fanins.at(1)]);
		assert(!ltfi_0->empty());
		assert(!ltfi_1->empty());
		if ((ltfi_0->size() == 1) && (ltfi_1->size() == 1)) {
			node_to_qubit_[fanins.at(0)] = choose_target(*ltfi_0);
			node_to_qubit_[fanins.at(1)] = choose_target(*ltfi_1);
			return gates;
		}

		if (ltfi_0->size() < ltfi_1->size()) {
			std::swap(ltfi_0, ltfi_1);
			std::swap(fanins.at(0), fanins.at(1));
		}

		std::vector<xag_signal> in0; // Set difference LFTI0 - LFTI1
		std::vector<xag_signal> in1; // Set difference LFTI1 - LFTI0
		std::vector<xag_signal> intersection_in01;
		compute_sets(*ltfi_0, *ltfi_1, in0, in1, intersection_in01);

		wire_id target_0 = choose_target(in0);

		if (both_xor == false) {
			assert(target_0 != wire::invalid);
			compute_xor_ios(*ltfi_0, target_0, gates);
			node_to_qubit_[fanins.at(0)] = target_0;
			return gates;
		}

		compute_xor_ios(in0, target_0, gates);

		wire_id target_1 = wire::invalid;
		if (!intersection_in01.empty()) {
			target_1 = choose_target(intersection_in01);
			compute_xor_ios(intersection_in01, target_1, gates);
			gates.emplace_back(target_1, target_0);
		} else {
			target_1 = choose_target(*ltfi_1);
		}

		compute_xor_ios(in1, target_1, gates);

		node_to_qubit_[fanins.at(0)] = target_0;
		node_to_qubit_[fanins.at(1)] = target_1;
		return gates;
	}

	// Compute AND node out-of-place
	void compute_and(xag_node const& node, uint32_t index)
	{
		quantum_ntk_.default_value(index);
		// Compute inputs
		std::vector<gate_info> gates = compute_and_inputs(node);
		std::for_each(gates.begin(), gates.end(), [&](gate_info const& gate) {
			create_xor(gate.control, gate.target);
		});

		std::vector<wire_id> controls = control_qubits(node);
		wire_id target = request_ancilla();
		node_to_qubit_[node] = target;
		create_and(controls, target);

		// Uncompute inputs
		std::for_each(gates.rbegin(), gates.rend(), [&](gate_info const& gate) {
			create_xor(gate.control, gate.target);
		});
		must_uncompute_.at(index) = 1;
	}

private:
	QuantumNtk& quantum_ntk_;
	LogicNtk xag_ntk_;
	xag_synth_params params_;

	// Internal
	mockturtle::node_map<wire_id, LogicNtk> node_to_qubit_;
	mockturtle::node_map<std::vector<xag_signal>, LogicNtk> node_ltfi_;
	std::vector<uint32_t> qubit_usage_;
	std::stack<wire_id> free_ancillae_;
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
